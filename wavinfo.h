#ifndef WAVINFO_H
#define WAVINFO_H

#include <QObject>

struct StFmt
{
    quint8  name[4];
    quint32 size;
    quint16 compressionCode;
    quint16 numberChannels;
    quint32 sampleRate;
    quint32 bytePerSecond;
    quint16 blockAllign;
    quint16 bitPerSample;
    quint16 extraFormat;
};

struct StData
{
    quint32 *size;
    quint8  *data;
};

class WaveInfo : public QObject
{
    Q_OBJECT
public:
    explicit WaveInfo(QObject *parent = nullptr);
    bool parse(QByteArray *dataIn);
    StFmt getFmt();
    StData getData();

private:
    QByteArray allData;
    StFmt  *wavFmt;
    StData  wavData;

signals:

public slots:
};

#endif // WAVINFO_H
