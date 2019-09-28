#include "filemodel.h"

MFile::MFile(const QString &type, const QString &size)
    : m_type(type), m_size(size)
{
}

QString MFile::type() const
{
    return m_type;
}

QString MFile::size() const
{
    return m_size;
}

FileModel::FileModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void FileModel::addMFile(const MFile &MFile)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_MFiles << MFile;
    endInsertRows();
}

int FileModel::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_MFiles.count();
}

QVariant FileModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= m_MFiles.count())
        return QVariant();

    const MFile &MFile = m_MFiles[index.row()];
    if (role == TypeRole)
        return MFile.type();
    else if (role == SizeRole)
        return MFile.size();
    return QVariant();
}

//![0]
QHash<int, QByteArray> FileModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[SizeRole] = "size";
    return roles;
}
//![0]

