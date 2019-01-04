#include "wavinfo.h"
#include <QDebug>

WaveInfo::WaveInfo(QObject *parent) : QObject(parent)
{

}

bool WaveInfo::parse(QByteArray *dataIn)
{
    int fmtPos = -1;
    int dataPos = -1;
    bool ret = false;
    allData.clear();
    allData = (*dataIn);

    qDebug()<<"parce";

    fmtPos = allData.indexOf("fmt ");
    if(fmtPos>0)
    {

        qDebug()<<"parce_1";

        wavFmt = reinterpret_cast<StFmt*>(allData.data()+fmtPos);
        dataPos = allData.indexOf("data");
        if(dataPos>0)
        {
            qDebug()<<"parce_2"<<dataPos<<allData.length();
            wavData.size = reinterpret_cast<quint32*>(allData.data()+dataPos+4);
            qDebug()<<"parce_3"<< (*wavData.size);
            wavData.data = reinterpret_cast<quint8*>(allData.data()+dataPos+8);
            qDebug()<<"parce_4";
            ret = true;
        }
    }
    return ret;
}

StFmt WaveInfo::getFmt()
{
    return *wavFmt;
}

StData WaveInfo::getData()
{
    return wavData;
}
