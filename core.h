#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QtCore>
#include <QSerialPort>
#include <QFile>
#include <QStack>
#include "struct.h"
#include "parser.h"

struct stWavHeader
{
    unsigned char chunkId[4];
    quint32 chunkSize;
    unsigned char format[4];
    unsigned char subchunk1Id[4];
    quint32 subchunk1Size;
    quint16 audioFormat;
    quint16 numChannels;
    quint32 sampleRate;
    quint32 byteRate;
    quint16 blockAlign;
    quint16 bitsPerSample;
    unsigned char subchunk2Id[4];
    quint32 subchunk2Size;
};

class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(QObject *parent = nullptr);
    ~Core(void);
    Q_INVOKABLE void decodeWavOk(QString filePath, bool saveToFile);
    Q_INVOKABLE void send      (QByteArray);
    Q_INVOKABLE void sendRaw   (QByteArray);
    Q_INVOKABLE void sendEdit  (QByteArray);
    Q_INVOKABLE void setImpuls (QString filePath, QString fileName);
//    QByteArray getSaveImpuls(QString filePath, QString fileName);
    Q_INVOKABLE bool openPort  (QString);
    Q_INVOKABLE void findPort  ();
    Q_INVOKABLE void slTimer();
    void PresetUpDownStage1(quint8);
    void PresetChangeStage1(quint8);

    Q_INVOKABLE void slSaveUpDown(quint8);
    void slSaveChange(qint8);

    Q_INVOKABLE void slCompare();
    Q_INVOKABLE void slEsc();
    Q_INVOKABLE void slEscImpuls();

    void setPresetUpDown(quint8);
    void setPresetChange(quint8);
    void setBank(bool up_down); //true(1) - up
    void setPreset(bool up_down); //true(1) - up

    Q_INVOKABLE void setFolderPos(quint8);
    Q_INVOKABLE quint8 getFolderPos();

    Q_INVOKABLE void readAll();
    Q_INVOKABLE void slCopy();
    Q_INVOKABLE void slPaste();

    Q_INVOKABLE void slOpenUrl1();
    Q_INVOKABLE void slOpenUrl2();

    Q_INVOKABLE QStringList getDrivers();
    Q_INVOKABLE QString     getLastPath();
    Q_INVOKABLE void        setLastPath(QString);

    Q_INVOKABLE QString     getSkinsPath();
    Q_INVOKABLE void        setSkinsPath(QString);

    Q_INVOKABLE bool        getAlternativeSkins();
    Q_INVOKABLE void        setAlternativeSkins(bool);

    Q_INVOKABLE void        colorOpen();
    Q_INVOKABLE void        setColorFileName(QString);

    Q_INVOKABLE QString     getColorFileName();

    Q_INVOKABLE void slAnswerErrSave(QString);

    Q_INVOKABLE void slTabKey(bool revert);

    Q_INVOKABLE void slUpDown(bool);

    Q_INVOKABLE void slSpeechValue(QString);

    Q_INVOKABLE quint8      typeDev;

    Q_INVOKABLE void setValue (QString name, quint8 value);

    Parser *getName;
    Parser *getEND;
    Parser *getccEND;
    Parser *getdccEND;
    Parser *getlccEND;
    Parser *getescEND;
    Parser *getRX;
    Parser *getNX;
    Parser *getStatus;
    Parser *getChb;
    Parser *getChp;
    Parser *getBank;
    Parser *savePreset;
    Parser *getImpuls;
    Parser *getImpulsEmpty;
    Parser *getMode;
    Parser *getRn;
    Parser *getRns16;
    Parser *getRns100;
    Parser *getRnEmpty;
    Parser *getCabSim;
    Parser *getPC;
    Parser *getVer;

private:
    QByteArray baCommand;
    QByteArray baAnswer;
    QByteArray res;

    bool   enResiv;
    quint8 sendCount;
    quint8 maxBank;
    quint8 maxPreset;

    QStack<quint8> stackFolder;
    stWavHeader getFormatWav(QString filePath);
    QByteArray decode(QByteArray *data, quint8 lenSample, quint8 chanalCount);
    QByteArray decodeSample(QByteArray data, quint8 lenSample);
    QSettings settings;
    quint8 presetUpDown;
    qint8 presetChange;
    bool presetEdit;
    QByteArray intToBa(quint32);
    QByteArray baSend;
    QTimer     *timer;
//    QTimer     *timer_delme;
    QList<QByteArray> rs;
    QList<QByteArray> command;
    quint16 commandCount;
    bool bCurGs;
    bool bEditable;
    QByteArray baCur;
    QByteArray baGSCopy;
    bool       bGSCopy;

    bool       bImpulsCopy;
    bool       bImpulsPaste;

    QByteArray baImpulsNameCopy;  //Имя последний считанный из флеши файла
    QByteArray baImpulsFileCopy;  //Содержимое последнего файла

    QString    lastImpulsFileDsp; // Последний файл вставленный из браузера
    QString    lastImpulsPathDsp; // Путь к последний файл вставленный из браузера

    QString    lastImpulsFileCopy; // Последний файл вставленный из браузера и скопированный по Copy
    QString    lastImpulsPathCopy; // Путь к последний файл вставленный из браузера  и скопированный по Copy

    void decodeWav(QString filePath, bool saveToFile);
    quint32 decoderCount;
    QByteArray baDecode;
    bool saveToFile;

//    QFile *log;

    QSerialPort *port;
    GS gs;
    quint8  massiv[50];
    quint8 outputType;
    QString lastSendStr;

    QString curSpeechControl;

    enum TypeDev
    {
        UnknownDev=0,CP100,CP16,CP16PA,CP100PA
    };

    enum EnumB
    {
        eBand1=0,eBand2,eBand3,eBand4,eBand5,eEarVol,eEarTyp,eVol,eCabOn,eEqOn,eEarRefOn,
        eAmpOn,eAmpVolume,eAmpSlave,eAmpType,
        ePreampOn,ePreampVolume,ePreampLow,ePreampMid,ePreampHigh,e20,e21,e22,e23,e24,e25,
        eF1,eF2,eF3,eF4,eF5,eQ1,eQ2,eQ3,eQ4,eQ5,eLpfScale,eHpfScale,eHpfOn,eLpfOn,ePresenceOn,ePresence
    };

    enum EnumC46
    {
    eq1=0, eq2, eq3, eq4, eq5, e_vol, e_t, pres_lev, cab_on, eq_on, er_on, amp_on, a_vol,
    amp_slave,a_t, preamp_on, preamp_vol, preamp_lo, preamp_mi, preamp_hi, gate_on, gate_th, gate_att,
    compr_on, sustein, compr_vol,
    fr1, fr2, fr3, fr4, fr5, q1, q2, q3, q4, q5,
    lop, hip, hip_on, lop_on, pr_on, pr_vol, eq_po
    };


private slots:
    void slPortTimer();
    void slPortTimer_Delme();
    void slReadyRead();
    void slAskEn();
    void slDecodeStop();
    void slPortError();

signals:
    void sgReadGsBand(quint32 Band1, quint32 Band2, quint32 Band3, quint32 Band4, quint32 Band5);
    void sgReadVol   (quint32 Vol);
    void sgReadEqOn  (quint32 EqOn);
    void sgReadCabOn  (quint32 CabOn);
    void sgReadEarRefOn (quint32 EarRefOn);
    void sgReadFreq  (qint32 F1,  qint32 F2,  qint32 F3,  qint32 F4,  qint32 F5);
    void sgReadQ     (qint32 Q1,  qint32 Q2,  qint32 Q3,  qint32 Q4,  qint32 Q5);
    void sgReadLPF   (quint32 Vol, quint32 On);
    void sgReadHPF   (quint32 Vol, quint32 On);
    void sgReadPresence (quint32 Vol, quint32 On);
//    void sgPortError(QString str);
    void signal();

    void sgReadBank  (quint32 bImpuls, quint32 numBank);
    void sgReadPreset(quint32 bImpuls, quint32 numPreset);
    void sgSetImpulsName (QString val);
    void sgSetEnabled (bool val);
    void sgSetProgress (float val);
    void sgReadMode  (quint32 mode);
    void sgReadName  (quint32 name);
    void sgReadEarlyType(quint32 type);
    void sgReadEarlyVal(quint32 val);
    void sgSetEdit(bool setEdit);
    void sgSetWait(bool wait);
    void sgPresetUpDownStage1 (quint8 inChangePreset);
    void sgPresetChangeStage1 (quint8 inChangePreset);
    void sgEndCommands();
    void sgNotSupport();
    void sgRequestConvert(QString filePath);
    void sgPortOpen (bool val, QString portName);
    void sgColorOpen();
    void sgSetColorFileName(QString foneColor);
    void sgReadValue(QString nameParam, qint32 value);
    void sgReadText(QString nameParam, QString value);
    void sgAnswerErrSave(QString strCom);

    void sgUpDown(QString controlName, bool up);


};

#endif // CORE_H
