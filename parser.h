#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QRegExp>
#include <QString>

class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(QByteArray val, QByteArray mask);
    QList<QByteArray> ret;
    bool getParse(QByteArray, QList<QByteArray>*);
    void clearAll();

private:
    QByteArray val, mask;
//    QByteArray baBuffer;
    quint16 curPos;
    quint16 maskPos;
    quint16 curParam;
    quint8 prevMask; //предидущий символ маски


signals:

public slots:
};

#endif // PARSER_H
