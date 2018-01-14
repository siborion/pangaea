#include "parser.h"
#include <QDebug>

//sample
//val  = "gs\r000000000000\r"
//mask = "1110000000000001"
// 1 - должно совпадать
// 0 - может не совпадать, помещается в возвращаемые параметры

Parser::Parser(QByteArray inVal, QByteArray inMask) : QObject()
{
    val  = inVal;
    mask = inMask;
    clearAll();
}

bool Parser::getParse(QByteArray inBa, QList<QByteArray> *inRet)
{
    quint8 afterX;
    bool kostyl;

    foreach (char chr, inBa)
    {
        kostyl = true;

//        if(this->objectName()=="RX")
//            qDebug() << QString("%1").arg(chr) << QString("%1").arg(val.at(curPos)) << curPos << "|" << mask.at(maskPos) << maskPos;

        if(mask.at(maskPos)=='X')
        {
            if(maskPos<val.size())
                afterX = val.at(maskPos+1);
        }

        if( (mask.at(maskPos)=='0') || ((mask.at(maskPos)=='X') && (afterX!=chr)) )
        {
            if(prevMask=='1')
                ret.append(QByteArray(""));
            ret.last().append(chr);
        }
        else
        {
            if(((mask.at(maskPos)=='X')&&(afterX==chr)) || (val.at(curPos)==chr))
            {

            }
            else
            {
                clearAll();
                kostyl = false;
                //                return false;
            }
        }


        if(kostyl)
        {
            prevMask = mask.at(maskPos);
            if((mask.at(maskPos)=='X'))
            {
                if (afterX == chr)
                {
                    maskPos++;
                    curPos++;
                    maskPos++;
                    curPos++;
                    prevMask = '1';
                }
            }
            else
            {
                maskPos++;
                curPos++;
            }

            if((maskPos>=mask.size()))
            {
                *inRet = ret;
                clearAll();
                return true;
            }
        }
    }
    return false;
}

void Parser::clearAll()
{
    curParam = curPos = maskPos = 0;
    prevMask = '1';
    ret.clear();
}

