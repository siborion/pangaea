#include "resample.h"
#include "soxr.h"
#include <QDebug>

resample::resample(QObject *parent) : QObject(parent)
{
    qDebug()<<soxr_version();
    output = new short[30000];
//    delete output;
}

void resample::start(short *input)
{
    int numframes=30000;
    float speed=0.3;

    int outputFrames = numframes * speed;
//    int extraReadNeeded = numframes - outputFrames;

    soxr_error_t err;

    soxr_datatype_t itype = SOXR_INT16_I;
    soxr_datatype_t otype = SOXR_INT16_I;
    soxr_io_spec_t iospec = soxr_io_spec(itype, otype);

    size_t idone = 0;
    size_t odone = 0;

    // Do the resampling
    soxr_t sox = soxr_create(44100, 44100 * speed, 1, &err, &iospec, NULL, NULL);
    qDebug()<<"err1";
    if (!err)
    {
        err = soxr_process(sox, input, numframes * 1, &idone, output, outputFrames, &odone);

        qDebug()<<"err2"<<odone;
    }
    else
    {
        qDebug()<<"err3";
    }
    qDebug()<<"err4";

    soxr_delete(sox);
}

short* resample::getSample()
{
    return output;
}
