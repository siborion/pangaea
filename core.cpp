#include "core.h"
#include <QSettings>
#include <QThread>
#include <QSerialPortInfo>
#include <QDir>
#include <QDesktopServices>
#include <QGuiApplication>
#include "qserialport.h"
//#include <QTextToSpeech>
#include <QtTest/QTest>

//#define logEn 1

Core::Core(QObject *parent) : QObject(parent), settings("AM Electronics", "CP-100")
{
    typeDev = UnknownDev;
    enResiv = false;
    bImpulsCopy = false;
    sendCount = 0;
    maxBank = 10;
    maxPreset = 10;
    timer = new QTimer();
    timer->setInterval(1000);

//    QTextToSpeech *speech;
//    speech = new QTextToSpeech;
//    speech->say("Проверка воспроизведения звука");

    getName    = new Parser("amtdev\rX\nEND\n", "1111111X11111");
    getEND     = new Parser("gsEND\n", "111111");
    getccEND   = new Parser("ccEND\n", "111111");
    getdccEND  = new Parser("dcc\rEND\n", "11111111");
    getlccEND  = new Parser("lcc\rEND\n", "11111111");
    getescEND  = new Parser("esc\rEND\n", "11111111");
    getNX      = new Parser("NX\n", "111");
    getRX      = new Parser("RX 01", "11111");
    getStatus  = new Parser("gs\r00000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n","111000000000000000000000000000000000000000000000000000000000000000000000000000000000000001");
    getChb     = new Parser("chb 0\r00\n", "111101001");
    getChp     = new Parser("chp 0\r00\n", "111101001");
    savePreset = new Parser("sp\rEND\n", "11111");
    getBank    = new Parser("gb\r0000\n", "11100001");
    getImpuls  = new Parser("cc\r0\n0\n", "111X1X1");
    getImpulsEmpty  = new Parser("cc\r\n\n", "11111");
    getMode    = new Parser("gm\r00\n", "111001");
    getRn      = new Parser("rn\r \n", "111X1");
    getRnEmpty = new Parser("rn\r\n", "1111");
    getCabSim = new Parser("ce 0\r", "11101");
    getPC     = new Parser("pc x\r", "111X1");
    getVer    = new Parser("amtver\r1.2.3\n","1111111000001");

    QByteArray sample, mask;
    sample = "rns\r";
    mask   = "1111";
    for(quint8 i=0; i<16; i++)
    {
        sample.append("_\n00\nEND\n");
        mask.append("X10011111");
    }
    getRns16     = new Parser(sample, mask);

    sample = "rns\r";
    mask   = "1111";
    for(quint8 i=0; i<100; i++)
    {
        sample.append("_\n00\nEND\n");
        mask.append("X10011111");
    }
    getRns100     = new Parser(sample, mask);
    getRns100->setObjectName("RX");

#ifdef logEn
    log = new QFile("txt.log");
    log->open(QIODevice::WriteOnly);
#endif

    presetEdit   = false;
    bImpulsPaste = false;
    typeDev = 1;
    presetChange = (-1);
    presetUpDown = 0;

    port = new QSerialPort(this);
    connect(port,  SIGNAL(readyRead()), SLOT(slReadyRead()));
    connect(port,  SIGNAL(error(QSerialPort::SerialPortError)), SLOT(slPortError()));
    connect(port,  SIGNAL(destroyed(QObject*)), SLOT(slPortError()));
    connect(port,  SIGNAL(errorOccurred(QSerialPort::SerialPortError)), SLOT(slPortError()));
    connect(timer, SIGNAL(timeout()), SLOT(slPortTimer()));

    timer->start();

//    QTextToSpeech *speech;
//    speech = new QTextToSpeech;
//    speech->say("Проверка воспроизведения звука");
}

void Core::slPortError()
{
    QObject *ob;
    ob = qobject_cast<QObject*>(port);
    if (ob)
    {
        if(port->isOpen())
        {
            port->close();
            emit sgPortOpen(false, "");
            emit sgReadText("close_port", "");
            emit sgReadValue ("type_dev", 0);
        }
    }
}

bool Core::openPort(QString portName)
{
    if(port->isOpen())
    {
        port->close();
        qDebug()<<__FUNCTION__<<__LINE__;
    }
    bool open;
    port->setPortName(portName);
    port->setBaudRate(9600);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::HardwareControl);
    open = port->open(QIODevice::ReadWrite);
    //    emit sgPortOpen (open, portName);
    emit sgReadText(open?"open_port":"close_port", portName.replace(".","").replace("\\",""));
    if(open)
    {
        res.clear();
        readAll();
    }
    return open;
}

void Core::slReadyRead()
{
    enResiv = false;
    res = port->readAll();
//    qDebug()<<"slReadyRead"<<res;

#ifdef logEn
    log->write("\r\n<<");
    log->write(res);
    log->flush();
#endif

    if(getNX->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getNX";
    }

    if(getRX->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getRX";
    }

    if(getStatus->getParse(res, &rs))
    {
        if(rs.size()==1)
        {
            if(rs.at(0).size()==(43*2))
            {
                bool ok;
                quint8 count=0;
                quint8 nomByte=0;
                QString sss;
                if(!bEditable)
                {
                    if(bCurGs)
                        baCur = (rs.at(0));
                }
                bCurGs = false;

                if(bGSCopy)
                {
                    baGSCopy = rs.at(0);
                    bGSCopy = false;
                }

                emit sgSetEnabled(bEditable);
                emit sgReadValue ("editable",      bEditable);

                foreach (quint8 val, rs.at(0))
                {
                    if((nomByte&1)==0)
                    {
                        sss.clear();
                        sss.append(val);
                    }
                    else
                    {
                        sss.append(val);
                        massiv[count++] = sss.toInt(&ok, 16);
                    }
                    nomByte++;
                }
                //                emit sgReadGsBand   (massiv[eBand1],  massiv[eBand2],  massiv[eBand3],  massiv[eBand4],  massiv[eBand5]);
                //                emit sgReadEarlyVal(massiv[eEarVol]);
                //                emit sgReadEarlyType(massiv[eEarTyp]);
                //                emit sgReadVol      (massiv[eVol]);
                //                emit sgReadEqOn     (massiv[eEqOn]);
                qint8 *sigMassiv;
                sigMassiv = (qint8*)massiv;
                //                emit sgReadFreq     ((qint32)sigMassiv[eF1], (qint32)sigMassiv[eF2], (qint32)sigMassiv[eF3], (qint32)sigMassiv[eF4], (qint32)sigMassiv[eF5]);
                //                emit sgReadLPF      ((quint32)massiv[eLpfScale], massiv[eLpfOn]);
                //                emit sgReadHPF      ((quint32)massiv[eHpfScale], massiv[eHpfOn]);
                //                emit sgReadPresence (massiv[ePresence], massiv[ePresenceOn]);
                //                emit sgReadCabOn    (massiv[eCabOn]);
                //                emit sgReadEarRefOn (massiv[eEarRefOn]);

                emit sgReadValue ("level_MASTER",           massiv[eVol]);
                emit sgReadValue ("level_HPF",              massiv[eHpfScale]);
                emit sgReadValue ("level_PRESENCE",         massiv[ePresence]);
                emit sgReadValue ("level_LPF",              massiv[eLpfScale]);
                emit sgReadValue ("level_PREAMP_VOLUME",    massiv[ePreampVolume]);
                emit sgReadValue ("level_PREAMP_LOW",       qint8(massiv[ePreampLow]));
                emit sgReadValue ("level_PREAMP_MID",       qint8(massiv[ePreampMid]));
                emit sgReadValue ("level_PREAMP_HIGH",      qint8(massiv[ePreampHigh]));
                emit sgReadValue ("level_POWERAMP_SLAVE",   massiv[eAmpSlave]);
                emit sgReadValue ("level_POWERAMP_VOLUME",  massiv[eAmpVolume]);
                emit sgReadValue ("level_EARLY",            massiv[eEarVol]);

                emit sgReadValue ("level_PARAMETRIC_Q0",   sigMassiv[eQ1]);
                emit sgReadValue ("level_PARAMETRIC_Q1",   sigMassiv[eQ2]);
                emit sgReadValue ("level_PARAMETRIC_Q2",   sigMassiv[eQ3]);
                emit sgReadValue ("level_PARAMETRIC_Q3",   sigMassiv[eQ4]);
                emit sgReadValue ("level_PARAMETRIC_Q4",   sigMassiv[eQ5]);

                emit sgReadValue ("level_PARAMETRIC_FREQ0",   sigMassiv[eF1]);
                emit sgReadValue ("level_PARAMETRIC_FREQ1",   sigMassiv[eF2]);
                emit sgReadValue ("level_PARAMETRIC_FREQ2",   sigMassiv[eF3]);
                emit sgReadValue ("level_PARAMETRIC_FREQ3",   sigMassiv[eF4]);
                emit sgReadValue ("level_PARAMETRIC_FREQ4",   sigMassiv[eF5]);

                emit sgReadValue ("type_POWERAMP",          massiv[eAmpType]);
                emit sgReadValue ("on_HPF",                 massiv[eHpfOn]);
                emit sgReadValue ("on_PRESENCE",            massiv[ePresenceOn]);
                emit sgReadValue ("on_LPF",                 massiv[eLpfOn]);
                emit sgReadValue ("on_PREAMP",              massiv[ePreampOn]);
                emit sgReadValue ("on_POWERAMP",            massiv[eAmpOn]);

                qDebug()<<"READ GS";

                emit sgReadValue ("master_volume",         massiv[EnumC46::pres_lev]);

                emit sgReadValue ("early_on",              massiv[EnumC46::er_on]);
                emit sgReadValue ("early_volume",          massiv[EnumC46::e_vol]);
                emit sgReadValue ("early_type",            massiv[EnumC46::e_t]);

                emit sgReadValue ("presence_on",           massiv[EnumC46::pr_on]);
                emit sgReadValue ("presence_volume",       massiv[EnumC46::pr_vol]);

                emit sgReadValue ("lpf_on",                massiv[EnumC46::lop_on]);
                emit sgReadValue ("lpf_volume",            massiv[EnumC46::lop]);

                emit sgReadValue ("hpf_on",                massiv[EnumC46::hip_on]);
                emit sgReadValue ("hpf_volume",            massiv[EnumC46::hip]);

                emit sgReadValue ("cabinet_enable",        massiv[EnumC46::cab_on]);

                emit sgReadValue ("amp_on",                massiv[EnumC46::amp_on]);
                emit sgReadValue ("amp_slave",             massiv[EnumC46::amp_slave]);
                emit sgReadValue ("amp_volume",            massiv[EnumC46::a_vol]);
                emit sgReadValue ("amp_type",              massiv[EnumC46::a_t]);

                emit sgReadValue ("preamp_on",             massiv[EnumC46::preamp_on]);
                emit sgReadValue ("preamp_volume",         massiv[EnumC46::preamp_vol]);
                emit sgReadValue ("preamp_low",            (qint8)massiv[EnumC46::preamp_lo]);
                emit sgReadValue ("preamp_mid",            (qint8)massiv[EnumC46::preamp_mi]);
                emit sgReadValue ("preamp_high",           (qint8)massiv[EnumC46::preamp_hi]);

                emit sgReadValue ("compreccor_on",         massiv[EnumC46::compr_on]);
                emit sgReadValue ("compreccor_sustein",    massiv[EnumC46::sustein]);
                emit sgReadValue ("compreccor_volume",     massiv[EnumC46::compr_vol]);

                emit sgReadValue ("gate_on",               massiv[EnumC46::gate_on]);
                emit sgReadValue ("gate_threshold",        massiv[EnumC46::gate_th]);
                emit sgReadValue ("gate_decay",            massiv[EnumC46::gate_att]);

                emit sgReadValue ("eq_on",                 massiv[EnumC46::eq_on]);
                emit sgReadValue ("eq_freq1",              (qint8)massiv[EnumC46::fr1]);
                emit sgReadValue ("eq_freq2",              (qint8)massiv[EnumC46::fr2]);
                emit sgReadValue ("eq_freq3",              (qint8)massiv[EnumC46::fr3]);
                emit sgReadValue ("eq_freq4",              (qint8)massiv[EnumC46::fr4]);
                emit sgReadValue ("eq_freq5",              (qint8)massiv[EnumC46::fr5]);
                emit sgReadValue ("eq_q1",                 (qint8)massiv[EnumC46::q1]);
                emit sgReadValue ("eq_q2",                 (qint8)massiv[EnumC46::q2]);
                emit sgReadValue ("eq_q3",                 (qint8)massiv[EnumC46::q3]);
                emit sgReadValue ("eq_q4",                 (qint8)massiv[EnumC46::q4]);
                emit sgReadValue ("eq_q5",                 (qint8)massiv[EnumC46::q5]);
                emit sgReadValue ("eq_volume1",            massiv[EnumC46::eq1]);
                emit sgReadValue ("eq_volume2",            massiv[EnumC46::eq2]);
                emit sgReadValue ("eq_volume3",            massiv[EnumC46::eq3]);
                emit sgReadValue ("eq_volume4",            massiv[EnumC46::eq4]);
                emit sgReadValue ("eq_volume5",            massiv[EnumC46::eq5]);

                emit sgReadValue ("eq_pre",                massiv[EnumC46::eq_po]);

                enResiv = true;
            }
        }
        qDebug()<<"getStatus";
    }

    if(getChb->getParse(res, &rs))
    {
        if(rs.size()==2)
        {
            if(rs.at(1).size()==2)
            {
                emit sgReadBank(rs.at(1).at(0), rs.at(1).at(1));
                emit sgReadValue ("read_bank",  (rs.at(1).at(0)-0x30)*10+(rs.at(1).at(1)-0x30));
                presetEdit = false;
                sgSetEdit(presetEdit);
                emit sgReadValue ("preset_edit",       presetEdit);
                enResiv = true;
            }
        }
        qDebug()<<"getChb";
    }

    if(getChp->getParse(res, &rs))
    {
        if(rs.size()==2)
        {
            if(rs.at(1).size()==2)
            {
                emit sgReadPreset(rs.at(1).at(0), rs.at(1).at(1));
                emit sgReadValue ("read_preset",  (rs.at(1).at(0)-0x30)*10+(rs.at(1).at(1)-0x30));
                presetEdit = false;
                sgSetEdit(presetEdit);
                emit sgReadValue ("preset_edit",       presetEdit);
                enResiv = true;
            }
        }
        qDebug()<<"getChp";
    }

    if(getBank->getParse(res, &rs))
    {
        if(rs.size()==1)
        {
            if(rs.at(0).size()==4)
            {
                quint8 bank, preset;
                bank   = rs.at(0).left(2).toUInt()+0x30;
                preset = rs.at(0).right(2).toUInt()+0x30;
                emit sgReadBank  (1, bank);
                emit sgReadPreset(1, preset);
                emit sgReadValue ("bank",              bank-48);
                emit sgReadValue ("preset",            preset-48);
                enResiv = true;
                presetEdit = false;
            }
        }
        qDebug()<<"getBank";
    }

    if(getRnEmpty->getParse(res, &rs))
    {
        //        emit sgSetImpulsName("empty");
        emit sgReadText ("impulse_name", "empty");
        getImpuls->clearAll();
        enResiv = true;
        qDebug()<<"getRnEmpty";
    }
    else
    {
        if(getRn->getParse(res, &rs))
        {
            QString str;
            str = rs.at(0);
            //            emit sgSetImpulsName(str);
            emit sgReadText ("impulse_name", str);
            enResiv = true;
            qDebug()<<"getRn";
        }
    }

    if(getMode->getParse(res, &rs))
    {
        if(rs.size()==1)
        {
            quint8 mode;
            mode = rs.at(0).toUInt();
            emit sgReadMode(mode);
            enResiv = true;
            emit sgReadValue ("mode",            mode);
        }
        qDebug()<<"getMode";
    }

    if(getName->getParse(res, &rs))
    {
        if(rs.size()==1)
        {
            typeDev = rs.at(0).toUInt();
            emit sgReadName(typeDev);
            emit sgReadValue ("type_dev", typeDev);
            if((typeDev==2)||(typeDev==3))
                maxPreset = maxBank = 4;
            if((typeDev==1)||(typeDev==4))
                maxPreset = maxBank = 10;
            enResiv = true;
            getName->clearAll();
            qDebug()<<"getName"<<typeDev;
        }
    }

    if(getVer->getParse(res, &rs))
    {
        if(rs.size()==1)
        {
            qDebug()<<"getVer";
        }
    }

    if(savePreset->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"savePreset";
    }

    if(getlccEND->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getLcc";
    }

    if(getEND->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getEND";
    }

    if(getCabSim->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getCabSim";
    }

    if(bImpulsCopy)
    {
        if(getImpuls->getParse(res, &rs))
        {
            qDebug()<<"parce getImpuls"<<rs.length();
            enResiv = true;
            baImpulsNameCopy.clear();
            baImpulsFileCopy.clear();
            if(rs.length()==2)
            {
                baImpulsNameCopy.append(rs.at(0));
                baImpulsFileCopy.append(rs.at(1));
                baImpulsNameCopy.replace(" ","_");
            }
            bImpulsCopy = false;
        }
    }

    if((typeDev==CP16)||(typeDev==CP16PA))
    {
        if(getRns16->getParse(res, &rs))
        {
            QString impulsNames;
            QString impulsEn;
            enResiv = true;
            QList<QByteArray>::const_iterator i = rs.constBegin();
            while (i != rs.constEnd())
            {
                QByteArray ba = *i;
                if(ba=="*")
                    ba="empty";
                impulsNames.append(ba);
                impulsNames.append(",");
                i++;

                ba = *i;
                impulsEn.append(ba);
                impulsEn.append(",");
                i++;
            }
            emit sgReadText ("impulse_names", impulsNames);
            emit sgReadText ("impulse_en", impulsEn);
            qDebug()<<"getRns16";
        }
    }

    if((typeDev==CP100)||(typeDev==CP100PA))
    {
        if(getRns100->getParse(res, &rs))
        {
            QString impulsNames;
            QString impulsEn;
            enResiv = true;
            QList<QByteArray>::const_iterator i = rs.constBegin();
            while (i != rs.constEnd())
            {
                QByteArray ba = *i;
                if(ba=="*")
                    ba="empty";
                impulsNames.append(ba);
                impulsNames.append(",");
                i++;

                ba = *i;
                impulsEn.append(ba);
                impulsEn.append(",");
                i++;
            }
            emit sgReadText ("impulse_names", impulsNames);
            emit sgReadText ("impulse_en", impulsEn);
            qDebug()<<"getRns100";
        }
    }

    if(getPC->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getPC";
    }

    if(getccEND->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getccEnd";
    }

    if(getdccEND->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getdccEnd";
    }

    if(getescEND->getParse(res, &rs))
    {
        enResiv = true;
        qDebug()<<"getescEnd";
    }


    if (enResiv)
    {
        if(command.length()>0)
            command.removeFirst();
        slTimer();
    }
}

void Core::send(QByteArray val)
{
    command.append(val);
    commandCount = command.size();
}


void Core::setImpuls(QString filePath, QString fileName)
{
    command.clear();
    res.clear();
    port->readAll();

    qDebug("$$$$$ %s %d", __FUNCTION__, __LINE__);

    stWavHeader wavHead = getFormatWav(filePath);

    if((wavHead.sampleRate != 48000) || (wavHead.bitsPerSample != 24) || (wavHead.numChannels != 1))
    {
        emit sgNotSupport();
        emit sgSetWait(false);
        emit sgReadValue ("wait", false);
    }
    else
    {
        bImpulsPaste = false;
        lastImpulsFileDsp = fileName;
        lastImpulsPathDsp = filePath;
        emit sgSetImpulsName(fileName);
        emit sgReadText ("impulse_name", fileName);
        decodeWav(filePath, false);
        //        typeLastInsertImpuls = 1;
    }
}

void Core::PresetUpDownStage1(quint8 inChangePreset)
{
    if(presetEdit)
        emit sgPresetUpDownStage1(inChangePreset);
    else
    {
        presetUpDown = inChangePreset;
        slTimer();
    }
}

void Core::PresetChangeStage1(quint8 inChangePreset)
{
    if(presetEdit)
        emit sgPresetChangeStage1(inChangePreset);
    else
    {
        presetChange = inChangePreset;
        slTimer();
    }
}

void Core::slSaveUpDown(quint8 inChangePreset)
{
    command.clear();
    res.clear();
    port->readAll();

    presetUpDown = inChangePreset;
    presetEdit = false;
    sgSetEdit(presetEdit);
    emit sgReadValue ("preset_edit",       presetEdit);

    if(bImpulsPaste)
    {
        if(!lastImpulsFileCopy.isEmpty())
        {
            qDebug()<<"save1"<<lastImpulsPathCopy;
            decodeWav(lastImpulsPathCopy, true);
            send("sp\r\n");
            send("rn\r\n");
        }
        else
        {

            if(baImpulsNameCopy.isEmpty())
                send("dcc\r\n");
            else
            {
                qDebug()<<"save2"<<baImpulsNameCopy;
                QByteArray baImp;
                baImp.append("cc ");
                baImp.append(baImpulsNameCopy);
                baImp.append(" 0\r");
                baImp.append(baImpulsFileCopy);
                baImp.append("\r");
                send(baImp);
            }
            send("sp\r\n");
            send("rn\r\n");
            slTimer();
            bImpulsPaste = false;
        }
    }
    else
    {

        if(!lastImpulsPathDsp.isEmpty())
        {
            emit sgSetWait(true);
            qDebug()<<"save3"<<lastImpulsPathDsp<<"file"<<lastImpulsFileDsp;
            decodeWav(lastImpulsPathDsp, true);
            emit sgReadValue ("wait", true);
        }
        else
        {
            send("sp\r\n");
            send("rn\r\n");
            send("gs\r\n");
            slTimer();
            qDebug()<<"save4";
        }
    }
    bEditable = true;
    emit sgSetEnabled(bEditable);
    emit sgReadValue ("editable",      bEditable);
}

void Core::slSaveChange(qint8 inChangePreset)
{
    command.clear();
    res.clear();
    port->readAll();

    presetChange = inChangePreset;
    presetEdit = false;
    sgSetEdit(presetEdit);
    emit sgReadValue ("preset_edit",       presetEdit);

    if(bImpulsPaste)
    {
        if(!lastImpulsFileCopy.isEmpty())
        {
            qDebug()<<"save1"<<lastImpulsPathCopy;
            decodeWav(lastImpulsPathCopy, true);
            send("sp\r\n");
            send("rn\r\n");
        }
        else
        {

            if(baImpulsNameCopy.isEmpty())
                send("dcc\r\n");
            else
            {
                qDebug()<<"save2"<<baImpulsNameCopy;
                QByteArray baImp;
                baImp.append("cc ");
                baImp.append(baImpulsNameCopy);
                baImp.append(" 0\r");
                baImp.append(baImpulsFileCopy);
                baImp.append("\r");
                send(baImp);
            }
            send("sp\r\n");
            send("rn\r\n");
            slTimer();
            bImpulsPaste = false;
        }
    }
    else
    {

        if(!lastImpulsPathDsp.isEmpty())
        {
            emit sgSetWait(true);
            qDebug()<<"save3"<<lastImpulsPathDsp<<"file"<<lastImpulsFileDsp;
            decodeWav(lastImpulsPathDsp, true);
            emit sgReadValue ("wait", true);
        }
        else
        {
            send("sp\r\n");
            send("rn\r\n");
            send("gs\r\n");
            slTimer();
            qDebug()<<"save4";
        }
    }
    bEditable = true;
    emit sgSetEnabled(bEditable);
    emit sgReadValue ("editable",      bEditable);
}


//QByteArray Core::getSaveImpuls(QString filePath, QString fileName)
//{
//    QByteArray baRet;
//    QByteArray baFile;
//    QByteArray baSample;
//    QFile   *file;
//    emit sgSetImpulsName(fileName);
//    file = new QFile(filePath);

//    qDebug()<<"filePath"<<filePath;

//    file->open(QIODevice::ReadOnly);
//    baSample = file->readAll();

//    qDebug()<<"baSample"<<baSample;

//    baRet.append(QString("cc %1 0\r").arg(fileName).toUtf8());
//    foreach (quint8 chr, baSample)
//    {
//        QString sTmp;
//        if(chr<=15)
//            sTmp = QString("0%1").arg(chr, 1, 16).toUtf8();
//        else
//            sTmp = QString("%1").arg (chr, 2, 16).toUtf8();
//        baFile.append(sTmp);
//    }

//    qDebug()<<"baFile"<<baFile;


//    baRet.append(baFile.left(32000));
//    baRet.append(QString("\r").toUtf8());
//    file->close();
//    return baRet;
//}

void Core::slTimer()
{
    if(command.size()>0)
    {
        qDebug()<<"sendCount"<<sendCount;
        float fVal;
        if(enResiv)
            sendCount = 0;
        else
        {
            sendCount++;
            if(sendCount>3)
            {
                sendCount = 0;
                baCommand = command.first();
                baAnswer = res;
                emit sgAnswerErrSave(command.first());
                command.clear();
                return;
            }
        }

        if( command.first().length() > 1000)
        {
            while (command.first().length())
            {
                sendRaw(command.first().left(1000));
                command.first().remove(0,1000);
            }
        }
        else
        {
            sendRaw(command.first());
        }

        if(command.first().indexOf("cc\r\n")==0)
        {
            timer->setInterval(5000);
        }
        else
        {
            if (command.first().indexOf("rns\r\n")==0)
            {
                timer->setInterval(10000);
            }
            else
            {
                if (command.first().indexOf("rn\r\n")==0)
                {
                    timer->setInterval(5000);
                }
                else
                {
                    if (command.first().indexOf("amtdev\r\n")==0)
                    {
                        timer->setInterval(2000);
                    }
                    else
                        timer->setInterval(1000);
                }
            }
        }

        fVal = commandCount;
        fVal -= command.size();
        fVal /= commandCount;
        emit sgSetProgress(fVal);
        emit sgSetWait(true);
        emit sgReadValue ("wait", true);
        if(command.size()==0)
            emit sgEndCommands();
    }
    else
    {
        commandCount = 0;
        if(presetUpDown)
        {
            setPresetUpDown(presetUpDown);
            presetUpDown = 0;
        }
        if(presetChange>=0)
        {
            setPresetChange(presetChange);
            presetChange = (-1);
        }
        emit sgSetWait(false);
        emit sgReadValue ("wait", false);
    }
}

void Core::slAskEn()
{
}

void Core::setBank(bool up_down)
{
    lastImpulsFileDsp.clear();
    lastImpulsPathDsp.clear();
    bEditable = true;
    QString str;
    command.clear();
    lastImpulsFileDsp.clear();
    lastImpulsPathDsp.clear();
    str = QString("chb %1\r\n").arg(up_down?0:1);
    send(str.toUtf8());
    send("gb\r\n");
    send("gs\r\n");
    send("rn\r\n");
    bImpulsPaste = false;
    slTimer();
}

void Core::setPreset(bool up_down)
{
    lastImpulsFileDsp.clear();
    lastImpulsPathDsp.clear();
    bEditable = true;
    QString str;
    command.clear();
    lastImpulsFileDsp.clear();
    lastImpulsPathDsp.clear();
    str = QString("chp %1\r\n").arg(up_down?0:1);
    send(str.toUtf8());
    send("gb\r\n");
    send("gs\r\n");
    send("rn\r\n");
    bImpulsPaste = false;
    slTimer();
}

void Core::readAll()
{
    bEditable = true;
    command.clear();
    res.clear();
    port->readAll();
    lastImpulsFileDsp.clear();
    lastImpulsPathDsp.clear();
    send("amtdev\r\n");
//    send("amtver\r\n");
    send("rns\r\n");
    send("rn\r\n");
    send("gb\r\n");
    send("gs\r\n");
    send("gm\r\n");
    //    slTimer();
}

void Core::sendRaw(QByteArray val)
{
    getNX->clearAll();
    getRX->clearAll();
    getRn->clearAll();
#ifdef logEn
    log->write(QString("\r\n>%1>").arg(command.size()).toUtf8());
    log->write(val);
    log->flush();
#endif
    port->readAll();
    res.clear();
    port->write(val);
//    qDebug()<<"send"<<val;
}

void Core::sendEdit(QByteArray val)
{
    presetEdit = true;
    sgSetEdit(presetEdit);
    emit sgReadValue ("preset_edit",       presetEdit);
    sendRaw(val);
}

void Core::findPort()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        if(
                ((info.vendorIdentifier()==0xff00)&&(info.productIdentifier()==0x0009)) ||
                ((info.vendorIdentifier()==0x0483)&&(info.productIdentifier()==0xA24B)) ||
                ((info.vendorIdentifier()==0x0483)&&(info.productIdentifier()==0x5740))
                )
        {
            openPort(info.systemLocation());
            qDebug()<<"vendor:"<<info.vendorIdentifier()<<" product: "<<info.productIdentifier()<<" location: "<<info.systemLocation();
            break;
        }
    }
}

void Core::slEsc()
{
    command.clear();
    res.clear();
    port->readAll();
    presetEdit = false;
    sgSetEdit(presetEdit);
    emit sgReadValue ("preset_edit",       presetEdit);
    bEditable = true;
    emit sgSetEnabled(bEditable);
    emit sgReadValue ("editable",      bEditable);
    send("esc\r\n");
    send("gs\r\n");
    send("rn\r\n");
    slTimer();
}

void Core::slCompare()
{
    command.clear();
    res.clear();
    port->readAll();

    if(bEditable)
    {
        bCurGs = true;
        send("gs\r\n");
        send("esc\r\n");
        send("rn\r\n");
    }
    else
    {
        if(bImpulsPaste)
            slPaste();
        else
        {
            QByteArray ba;
            ba.append("gs 1\r");
            ba.append(baCur);
            ba.append("\r");
            send(ba);
            if(!lastImpulsFileDsp.isEmpty())
            {
                decodeWav(lastImpulsPathDsp, false);
                emit sgSetImpulsName(lastImpulsFileDsp);
                sgReadText("impulse_name", lastImpulsFileDsp);
            }
        }
    }
    send("gs\r\n");

    bEditable^=1;
    slTimer();
}

void Core::slEscImpuls()
{
    command.clear();
    res.clear();
    port->readAll();
    lastImpulsFileDsp.clear();
    lastImpulsPathDsp.clear();
    send("lcc\r\n");
    send("rn\r\n");
    slTimer();
}


void Core::decodeWav(QString filePath, bool whereSave)
{
    qDebug()<<"decodeWav"<<filePath<<whereSave;

    if(!whereSave)
    {
        decodeWavOk(filePath, whereSave);
    }
    else
    {
        stWavHeader wavHead = getFormatWav(filePath);
        if((wavHead.bitsPerSample != 24) || (wavHead.numChannels == 2))
        {
            qDebug("$$$$ FormatError");
            send("sp\r\n");
            send("rn\r\n");
            send("gs\r\n");
            emit sgRequestConvert(filePath);
            slTimer();
        }
        else
        {
            qDebug("$$$$ FormatOk");
            decodeWavOk(filePath, whereSave);
        }
    }
}


void Core::decodeWavOk(QString filePath, bool whereSave)
{
    quint8 bitPerSample, numChannels;
    presetEdit = true;
    sgSetEdit(presetEdit);
    emit sgReadValue ("preset_edit",       presetEdit);
    saveToFile = whereSave;

    QFile *file = new QFile(filePath);
    file->open(QIODevice::ReadOnly);
    QByteArray baWav;

    baWav = file->read(44+32000);

    stWavHeader *wavHeader = reinterpret_cast<stWavHeader*>(baWav.data());
    bitPerSample = wavHeader->bitsPerSample;
    numChannels = wavHeader->numChannels;
    QString tmp=QString::fromLocal8Bit((char*)wavHeader->subchunk2Id, 4);
    baWav.remove(0, baWav.indexOf("data")+8);

    qDebug()<<"numChannels"<<numChannels;
    qDebug()<<"bitPerSample"<<bitPerSample;

    baDecode.clear();
    baDecode.append(decode(&baWav, bitPerSample/8, numChannels));
    slDecodeStop();
}

void Core::slDecodeStop()
{
    QByteArray baSend;

    qDebug()<<"__FUNCTION__"<<__FUNCTION__<<__LINE__;

    if(saveToFile)
    {
        QByteArray fileData, fileHeader;
        fileData = baDecode.left(32000);
        fileHeader.append(QString("RIFF").toUtf8());                        //chunkId
        fileHeader.append(QByteArray::fromRawData("\x00\x00\x00\x00", 4));  //chunkSize
        fileHeader.append(QString("WAVE").toUtf8());                        //format
        fileHeader.append(QString("fmt ").toUtf8());                        //subchunk1Id
        fileHeader.append(QByteArray::fromRawData("\x10\x00\x00\x00", 4));  //subchunk1Size
        fileHeader.append(QByteArray::fromRawData("\x01\x00", 2));          //audioFormat
        fileHeader.append(QByteArray::fromRawData("\x01\x00", 2));          //numChannels
        fileHeader.append(QByteArray::fromRawData("\x80\xBB\x00\x00", 4));  //sampleRate
        fileHeader.append(QByteArray::fromRawData("\x80\x32\x02\x00", 4));  //byteRate
        fileHeader.append(QByteArray::fromRawData("\x03\x00", 2));          //blockAlign
        fileHeader.append(QByteArray::fromRawData("\x18\x00", 2));          //bitsPerSample
        fileHeader.append(QString("data").toUtf8());                        //subchunk2Id
        fileHeader.append(intToBa(fileData.length()));                      //subchunk2Size

        fileHeader.append(fileData);
        fileHeader.replace(4, 4, intToBa(fileHeader.length()-8));

        QFile *wavFile = new QFile("wav.wav");
        wavFile->open(QIODevice::WriteOnly);
        wavFile->write(fileHeader);
        wavFile->close();

        QString fileNameAbs;
        if(!lastImpulsFileCopy.isEmpty() && bImpulsPaste)
            fileNameAbs = lastImpulsFileCopy.replace(QString(" "), QString("_"), Qt::CaseInsensitive);
        else
            fileNameAbs = lastImpulsFileDsp.replace(QString(" "), QString("_"), Qt::CaseInsensitive);
        baSend.append(QString("cc %1 0\r").arg(fileNameAbs).toUtf8());

        foreach (quint8 chr, fileHeader)
        {
            QString sTmp;
            if(chr<=15)
                sTmp = QString("0%1").arg(chr, 1, 16);
            else
                sTmp = QString("%1").arg (chr, 2, 16);
            baSend.append(sTmp.toUtf8());
        }
        baSend.append(QString("\r").toUtf8());
        send(baSend);
        send("sp\r\n");
        send("rn\r\n");
        send("gs\r\n");
        presetEdit = false;
        sgSetEdit(presetEdit);
        emit sgReadValue ("preset_edit",       presetEdit);
        lastImpulsFileDsp.clear();
        lastImpulsPathDsp.clear();
    }
    else
    {
        baSend.append(QString("cc s 1\r").toUtf8());
        for(quint16 i=0; i<(984*3); i++)
        {
            QString sTmp;
            quint8  chr;
            if(i>=baDecode.length())
                sTmp = QString("00");
            else
            {
                chr = baDecode.at(i);
                if(chr<=15)
                    sTmp = QString("0%1").arg(chr, 1, 16);
                else
                    sTmp = QString("%1").arg (chr, 2, 16);
            }
            baSend.append(sTmp.toUtf8());
        }
        baSend.append(QString("\r").toUtf8());

        send(baSend);
    }
    slTimer();
}


QByteArray Core::intToBa(quint32 val)
{
    QByteArray baRet;
    baRet.append((val&0xff));
    baRet.append((val&0xff00)>>8);
    baRet.append((val&0xff0000)>>16);
    baRet.append((val&0xff000000)>>24);
    return baRet;
}

void Core::setPresetChange(quint8 val)
{
    lastImpulsFileDsp.clear();
    lastImpulsPathDsp.clear();
    bEditable = true;
    presetEdit = false;
    //    emit sgSetEdit(presetEdit);
    emit sgReadValue ("preset_edit",   presetEdit);
    //    emit sgSetEnabled(bEditable);
    emit sgReadValue ("editable",      bEditable);
    command.clear();
    lastImpulsFileDsp.clear();
    lastImpulsPathDsp.clear();
    send(QString("pc %1\r\n").arg(val, 0, 16).toUtf8());
    send("gb\r\n");
    send("gs\r\n");
    send("rn\r\n");
    bImpulsPaste = false;
    slTimer();
}

void Core::setPresetUpDown(quint8 val)
{
    switch (val)
    {
    case 1:
        setBank(false);
        break;
    case 2:
        setBank(true);
        break;
    case 3:
        setPreset(false);
        break;
    case 4:
        setPreset(true);
        break;
    }
}

void Core::slCopy()
{
    bImpulsCopy = true;
    bGSCopy = true;
    lastImpulsFileCopy = lastImpulsFileDsp;
    lastImpulsPathCopy = lastImpulsPathDsp;
    send("gs\r\n");
    send("cc\r\n");
    slTimer();
}

void Core::slPaste()
{
    QByteArray ba, baImp;

    if(!lastImpulsFileCopy.isEmpty())
    {
        sgSetImpulsName(lastImpulsFileCopy);
        sgReadText("impulse_name",lastImpulsFileCopy);
        ba.append("gs 1\r");
        ba.append(baGSCopy);
        ba.append("\r");
        send(ba);
        send("gs\r\n");
        decodeWav(lastImpulsPathCopy, false);
        //        typeLastInsertImpuls = 2;
    }
    else
    {
        if(!baImpulsNameCopy.isEmpty())
        {
            sgSetImpulsName(baImpulsNameCopy);
            sgReadText("impulse_name", baImpulsNameCopy);
            ba.append("gs 1\r");
            ba.append(baGSCopy);
            ba.append("\r");
            send(ba);
            send("gs\r\n");
            ba.clear();

            ba.append(baImpulsFileCopy);
            ba.remove(0,44*2);
            ba = ba.left(984*3*2);

            baImp.append(QString("cc s 1\r").toUtf8());
            baImp.append(ba);
            baImp.append("\r");
            send(baImp);
            if(baImpulsNameCopy.isEmpty())
                send("ce 0\r\n");
        }
        else
        {
            sgSetImpulsName("empty");
            sgReadText("impulse_name", "empty");
            baGSCopy.replace(16,2,"00"); //if baImpulsNameCopy.isEmpty() - Cab Off !!!
            ba.append("gs 1\r");
            ba.append(baGSCopy);
            ba.append("\r");
            send(ba);
            send("gs\r\n");
            ba.clear();

            ba.append(QString("ffff7f").toUtf8());
            ba.append(983*3*2, 0x30);

            baImp.append(QString("cc s 1\r").toUtf8());
            baImp.append(ba);
            baImp.append("\r");
            send(baImp);
            if(baImpulsNameCopy.isEmpty())
                send("ce 0\r\n");
        }
        slTimer();
    }

    bImpulsPaste = true;
    presetEdit = true;
    sgSetEdit(presetEdit);
    emit sgReadValue ("preset_edit",       presetEdit);
}

QStringList Core::getDrivers()
{
    QStringList sl;
    QFileInfoList fil;
    QFileInfo fi;
    fil = QDir::drives();
    foreach (fi, fil)
    {
        sl.append(fi.absolutePath());
    }
    return (sl);
}


QString Core::getLastPath()
{
    QString ret;
    ret = settings.value("ImpulsPath","file:///C:/").toString();
    return ret;
}
void Core::setLastPath(QString path)
{
    settings.setValue("ImpulsPath", path);
    settings.sync();
}

QString Core::getSkinsPath()
{
    QString ret;
    ret = settings.value("SkinsPath","file:///C:/").toString();
    return ret;
}
void Core::setSkinsPath(QString path)
{
    settings.setValue("SkinsPath", path);
    settings.sync();
}

bool Core::getAlternativeSkins()
{
    bool ret;
    ret = settings.value("AlternativeSkins","false").toBool();
    return ret;
}
void Core::setAlternativeSkins(bool val)
{
    settings.setValue("AlternativeSkins", val);
    settings.sync();
}



QByteArray Core::decode(QByteArray *data, quint8 lenSample, quint8 chanalCount)
{
    QByteArray ret;
    quint16 lenWav;
    lenWav = data->length();
    for(quint16 i=0; i<lenWav; i+=(lenSample*chanalCount))
    {
        ret.append(decodeSample(data->mid(i,lenSample), lenSample));
    }

    return ret;
}

QByteArray Core::decodeSample(QByteArray data, quint8 lenSample)
{
    QByteArray ret;
    if(data.length()<lenSample)
    {
        quint8 i = 0;
        for(i=0; i<data.length(); i++)
        {
            ret.append(data.at(i));
        }

//        for(; i<lenSample; i++)
//        {
//            ret.append((char)0);
//        }

        return ret;
    }

    switch (lenSample)
    {
    case 1:
        ret.append((char)0);
        ret.append((char)0);
        ret.append(data.at(0));
        break;
    case 2:
        ret.append((char)0);
        ret.append(data.at(0));
        ret.append(data.at(1));
        break;
    case 3:
        ret.append(data.at(0));
        ret.append(data.at(1));
        ret.append(data.at(2));
        break;
    case 4:
        ret.append(data.at(1));
        ret.append(data.at(2));
        ret.append(data.at(3));
        break;
    }
    return ret;
}

stWavHeader Core::getFormatWav(QString filePath)
{
    QFile *file = new QFile(filePath);
    file->open(QIODevice::ReadOnly);
    QByteArray baWav;
    baWav = file->read(44+32000);
    stWavHeader *wavHeader = reinterpret_cast<stWavHeader*>(baWav.data());
    return *wavHeader;
}

void Core::slPortTimer_Delme()
{
    qDebug()<<"TimerDelMe";

}

void Core::slPortTimer()
{
    if(!port->isOpen())
    {
        qDebug()<<"!OPEN";
        findPort();
    }
    else
    {
        port->setBaudRate(9600); // ловим ошибку при отключении usb. Не убирать!
        slTimer();
    }
}


void Core::setFolderPos(quint8 curPosition)
{
    qDebug()<<curPosition;
    stackFolder.push(curPosition);
}

quint8 Core::getFolderPos()
{
    quint8 tmp;
    tmp = stackFolder.pop();
    qDebug()<<tmp;
    return tmp;
}

void Core::slOpenUrl1()
{
    QString file = "http://amtelectronics.com/new/amt-pangaea-cp-16-module/";
    QUrl url = QUrl::fromLocalFile(file);
    QDesktopServices::openUrl(url);
}

void Core::slOpenUrl2()
{
    QString file = "http://amtelectronics.com/new/shop/";
    QUrl url = QUrl::fromLocalFile(file);
    QDesktopServices::openUrl(url);
}

void Core::colorOpen()
{
    emit sgColorOpen();
}

void Core::setColorFileName(QString foneColor)
{
    qDebug()<<"setColorFileName"<<foneColor;
    emit sgSetColorFileName(foneColor);
    settings.setValue("ColorImpulsFone", foneColor);
    settings.sync();
}

QString Core::getColorFileName()
{
    return settings.value("ColorImpulsFone","#0000ff").toString();
}

void Core::setValue(QString name, quint8 value)
{
    QString sendStr;

    qDebug()<<"setValue"<<name<<value;

    if(name==("esc"))
        sendStr = QString("esc\r\n");

    if(name==("master_volume"))
        sendStr = QString("mv %1\r\n").arg(value, 0, 16);

    if(name==("early_on"))
        sendStr = QString("eo %1\r\n").arg(value, 0, 16);
    if(name==("early_volume"))
        sendStr = QString("ev %1\r\n").arg(value, 0, 16);
    if(name==("early_type"))
        sendStr = QString("et %1\r\n").arg(value, 0, 16);

    if(name==("presence_on"))
        sendStr = QString("po %1\r\n").arg(value, 0, 16);
    if(name==("presence_volume"))
        sendStr = QString("pv %1\r\n").arg(value, 0, 16);

    if(name==("lpf_on"))
        sendStr = QString("lo %1\r\n").arg(value, 0, 16);
    if(name==("lpf_volume"))
        sendStr = QString("lv %1\r\n").arg(value, 0, 16);

    if(name==("hpf_on"))
        sendStr = QString("ho %1\r\n").arg(value, 0, 16);
    if(name==("hpf_volume"))
        sendStr = QString("hv %1\r\n").arg(value, 0, 16);

    if(name==("cabinet_enable"))
        sendStr = QString("ce %1\r\n").arg(value, 0, 16);


    if(name==("amp_on"))
        sendStr = QString("ao %1\r\n").arg(value, 0, 16);
    if(name==("amp_slave"))
        sendStr = QString("as %1\r\n").arg(value, 0, 16);
    if(name==("amp_volume"))
        sendStr = QString("av %1\r\n").arg(value, 0, 16);
    if(name==("amp_type"))
        sendStr = QString("at %1\r\n").arg(value, 0, 16);

    if(name==("preamp_on"))
        sendStr = QString("pro %1\r\n").arg(value, 0, 16);
    if(name==("preamp_volume"))
        sendStr = QString("prv %1\r\n").arg(value, 0, 16);
    if(name==("preamp_low"))
        sendStr = QString("prl %1\r\n").arg(value, 0, 16);
    if(name==("preamp_mid"))
        sendStr = QString("prm %1\r\n").arg(value, 0, 16);
    if(name==("preamp_high"))
        sendStr = QString("prh %1\r\n").arg(value, 0, 16);

    if(name==("compreccor_on"))
        sendStr = QString("co %1\r\n").arg(value, 0, 16);
    if(name==("compreccor_sustein"))
        sendStr = QString("cs %1\r\n").arg(value, 0, 16);
    if(name==("compreccor_volume"))
        sendStr = QString("cv %1\r\n").arg(value, 0, 16);

    if(name==("gate_on"))
        sendStr = QString("go %1\r\n").arg(value, 0, 16);
    if(name==("gate_threshold"))
        sendStr = QString("gt %1\r\n").arg(value, 0, 16);
    if(name==("gate_decay"))
        sendStr = QString("gd %1\r\n").arg(value, 0, 16);

    if(name==("eq_on"))
        sendStr = QString("eqo %1\r\n").arg(value, 0, 16);
    if(name==("eq_freq1"))
        sendStr = QString("eqf 0 %1\r\n").arg(value, 0, 16);
    if(name==("eq_freq2"))
        sendStr = QString("eqf 1 %1\r\n").arg(value, 0, 16);
    if(name==("eq_freq3"))
        sendStr = QString("eqf 2 %1\r\n").arg(value, 0, 16);
    if(name==("eq_freq4"))
        sendStr = QString("eqf 3 %1\r\n").arg(value, 0, 16);
    if(name==("eq_freq5"))
        sendStr = QString("eqf 4 %1\r\n").arg(value, 0, 16);

    if(name==("eq_q1"))
        sendStr = QString("eqq 0 %1\r\n").arg(value, 0, 16);
    if(name==("eq_q2"))
        sendStr = QString("eqq 1 %1\r\n").arg(value, 0, 16);
    if(name==("eq_q3"))
        sendStr = QString("eqq 2 %1\r\n").arg(value, 0, 16);
    if(name==("eq_q4"))
        sendStr = QString("eqq 3 %1\r\n").arg(value, 0, 16);
    if(name==("eq_q5"))
        sendStr = QString("eqq 4 %1\r\n").arg(value, 0, 16);

    if(name==("eq_volume1"))
        sendStr = QString("eqv 0 %1\r\n").arg(value, 0, 16);
    if(name==("eq_volume2"))
        sendStr = QString("eqv 1 %1\r\n").arg(value, 0, 16);
    if(name==("eq_volume3"))
        sendStr = QString("eqv 2 %1\r\n").arg(value, 0, 16);
    if(name==("eq_volume4"))
        sendStr = QString("eqv 3 %1\r\n").arg(value, 0, 16);
    if(name==("eq_volume5"))
        sendStr = QString("eqv 4 %1\r\n").arg(value, 0, 16);

    if(name==("eq_pre"))
        sendStr = QString("eqp %1\r\n").arg(value, 0, 16);


    if(sendStr.length()>0)
        presetEdit = true;




    if(name==("mode"))
        sendStr = QString("gm %1\r\n").arg(value, 0, 16);

    if(name==("copy"))
        slCopy();

    if(name==("map_update"))
    {
        for(quint8 i=0; i<maxPreset; i++)
        {
            send("chp 1\r\n");
            send("gb\r\n");
            send("rn\r\n");
            send("gs\r\n");
            for(quint8 j=0; j<maxBank; j++)
            {
                send("chb 1\r\n");
                send("gb\r\n");
                send("rn\r\n");
                send("gs\r\n");
            }
        }
        slTimer();
    }

    if(name==("past"))
        slPaste();

    if(name==("save_up_down"))
        slSaveUpDown(value);
    if(name==("save_change"))
        slSaveChange(value);
    if(name==("comp"))
        slCompare();

    if(name==("preset_UpDown"))
        PresetUpDownStage1(value?3:4);
    if(name==("bank_UpDown"))
        PresetUpDownStage1(value?1:2);
    if(name==("preset_change"))
        PresetChangeStage1(value);


    if(name==("set_preset_updown"))
        setPresetUpDown(value);

    if(name==("set_preset_change"))
        setPresetChange(value);




    emit sgReadValue ("preset_edit", presetEdit);




    if(name==("level_MASTER"))
        sendStr = QString("mv %1\r\n").arg(value, 0, 16);
    if(name==("level_HPF"))
        sendStr = QString("hv %1\r\n").arg(value, 0, 16);
    if(name==("level_PRESENCE"))
        sendStr = QString("pv %1\r\n").arg(value, 0, 16);
    if(name==("level_LPF"))
        sendStr = QString("lv %1\r\n").arg(value, 0, 16);
    if(name==("level_PREAMP_VOLUME"))
        sendStr = QString("prv %1\r\n").arg(value, 0, 16);
    if(name==("level_PREAMP_LOW"))
        sendStr = QString("prl %1\r\n").arg(value, 0, 16);
    if(name==("level_PREAMP_MID"))
        sendStr = QString("prm %1\r\n").arg(value, 0, 16);
    if(name==("level_PREAMP_HIGH"))
        sendStr = QString("prh %1\r\n").arg(value, 0, 16);
    if(name==("level_POWERAMP_SLAVE"))
        sendStr = QString("as %1\r\n").arg(value, 0, 16);
    if(name==("level_POWERAMP_VOLUME"))
        sendStr = QString("av %1\r\n").arg(value, 0, 16);
    if(name==("level_EARLY"))
        sendStr = QString("ev %1\r\n").arg(value, 0, 16);

    if(name==("level_PARAMETRIC_Q0"))
        sendStr = QString("eqq 0 %1\r\n").arg(value, 0, 16);
    if(name==("level_PARAMETRIC_Q1"))
        sendStr = QString("eqq 1 %1\r\n").arg(value, 0, 16);
    if(name==("level_PARAMETRIC_Q2"))
        sendStr = QString("eqq 2 %1\r\n").arg(value, 0, 16);
    if(name==("level_PARAMETRIC_Q3"))
        sendStr = QString("eqq 3 %1\r\n").arg(value, 0, 16);
    if(name==("level_PARAMETRIC_Q4"))
        sendStr = QString("eqq 4 %1\r\n").arg(value, 0, 16);

    if(name==("level_PARAMETRIC_FREQ0"))
        sendStr = QString("eqf 0 %1\r\n").arg(value, 0, 16);
    if(name==("level_PARAMETRIC_FREQ1"))
        sendStr = QString("eqf 1 %1\r\n").arg(value, 0, 16);
    if(name==("level_PARAMETRIC_FREQ2"))
        sendStr = QString("eqf 2 %1\r\n").arg(value, 0, 16);
    if(name==("level_PARAMETRIC_FREQ3"))
        sendStr = QString("eqf 3 %1\r\n").arg(value, 0, 16);
    if(name==("level_PARAMETRIC_FREQ4"))
        sendStr = QString("eqf 4 %1\r\n").arg(value, 0, 16);


    if(name==("type_POWERAMP"))
        sendStr = QString("at %1\r\n").arg(value, 0, 16);

    if(name==("on_PARAMETRIC_EQ"))
        sendStr = QString("eqo %1\r\n").arg(value, 0, 16);
    if(name==("on_HPF"))
        sendStr = QString("ho %1\r\n").arg(value, 0, 16);
    if(name==("on_PRESENCE"))
        sendStr = QString("po %1\r\n").arg(value, 0, 16);
    if(name==("on_LPF"))
        sendStr = QString("lo %1\r\n").arg(value, 0, 16);
    if(name==("on_PREAMP"))
        sendStr = QString("pro %1\r\n").arg(value, 0, 16);
    //    if(name==("on_POWERAMP"))
    //        sendStr = QString("ao %1\r\n").arg(value, 0, 16);

    //    qDebug()<<"name"<<name<<value<<sendStr;


    if(sendStr.length()>0)
    {
        sgSetEdit(true);
        sendRaw(sendStr.toUtf8());
        lastSendStr = sendStr;
    }

}

void Core::slAnswerErrSave(QString fileName)
{
    QFile file;
#ifdef WIN32
    fileName.remove("file:///");
#else
    fileName.remove("file://");
#endif
    file.setFileName(fileName);
    qDebug()<<fileName;
    if (file.open(QIODevice::WriteOnly))
    {
        qDebug()<<fileName;
        file.write(">>>\r\n");
        file.write(baCommand);
        file.write("<<<\r\n");
        file.write(baAnswer);
    }
}

void Core::slTabKey(bool revert)
{
    static quint16 curSpeech=0;
    static const QStringList sl={"master_volume", "early_volume"};


    curSpeech++;
    if( curSpeech>=sl.length() )
        curSpeech = 0;

    curSpeechControl = sl.at(curSpeech);


//    QTextToSpeech *speech;
//    speech = new QTextToSpeech;
//    speech->say(curSpeechControl);


}

void Core::slUpDown(bool Up)
{
//    emit sgUpDown(curSpeechControl, Up);
}

void Core::slSpeechValue(QString value)
{
//    QTextToSpeech *speech;
//    speech = new QTextToSpeech;
//    speech->say(value);
}


Core::~Core()
{
    if(port->isOpen())
    {
        port->close();
        qDebug()<<"CLOSEPORT";
    }
}


