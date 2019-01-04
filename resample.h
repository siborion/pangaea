#ifndef RESAMPLE_H
#define RESAMPLE_H

#include <QObject>

class resample : public QObject
{
    Q_OBJECT
public:
    explicit resample(QObject *parent = nullptr);
    void start(short* input);
    short* getSample();
private:
    short* output;


signals:

public slots:
};

#endif // RESAMPLE_H
