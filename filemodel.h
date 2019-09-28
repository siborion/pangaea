#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractListModel>
#include <QStringList>

//![0]
class MFile
{
public:
    MFile(const QString &type, const QString &size);
//![0]

    QString type() const;
    QString size() const;

private:
    QString m_type;
    QString m_size;
//![1]
};

class FileModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum MFileRoles {
        TypeRole = Qt::UserRole + 1,
        SizeRole
    };

    FileModel(QObject *parent = 0);
//![1]

    void addMFile(const MFile &MFile);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;
private:
    QList<MFile> m_MFiles;
//![2]
};
//![2]


#endif // FILEMODEL_H
