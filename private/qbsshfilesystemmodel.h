#ifndef QBSSHFILESYSTEMMODEL_H
#define QBSSHFILESYSTEMMODEL_H

#include <QHash>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QAbstractListModel>

#include <sftpchannel.h>
#include <sshconnection.h>
#include <sshremoteprocess.h>


namespace QbSsh {

class QbSshFileInfo;
class QbSshFileSystemModel;


class QbSshFileInfo{
    friend class QbSshFileSystemModel;

private:
    QString m_name;
    QString m_path;
    quint64 m_size;
    QSsh::SftpFileType m_type;
    QFile::Permissions m_permissions;
    bool m_sizeValid;
    bool m_permissionsValid;

public:
    explicit QbSshFileInfo();

protected:
    static QbSshFileInfo form(const QString &pwd,const QSsh::SftpFileInfo &fi);

};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief The QbSshFileSystemModel class
///
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class QbSshFileSystemModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QbSshFileSystemModel(QObject *parent = Q_NULLPTR);

    void setRootDirectory(const QString &path);

    bool isConnected();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief addIcon
    /// \param ext
    /// \param path
    ///
    /// Icon for different file types
    /// "dir" for directory
    /// "file" for any file type
    ///////////////////////////////////////////////////////////////////////////////////////////////
    void addIcon(const QString &ext, const QString &path);

    void setIconMap(const QMap<QString,QString> &iconMap);
    void clearIconMap();

    bool isFolder(const QModelIndex &index);
    bool isFile(const QModelIndex &index);

    QString getName(const QModelIndex &index);
    QString getSFTPFullPath(const QModelIndex &index);

    QbSshFileInfo getFile(const QModelIndex &index);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QString getPWD();

private:
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

public slots:
    void navigateToPath(const QString &path);
    void navigateToBack();
    void refresh();

    void connectToHost(const QString &host, const QString &username, const QString &password);
    void reconnectToHost();

private:
    void listDir();
    void connectionCode();
    void shutdown();

private slots:
    void handleSshConnectionFailure();
    void handleSshConnectionEstablished();
    void handleSftpChannelInitialized();
    void handleSftpChannelError(const QString &reason);

    void handleSftpJobFinished(QSsh::SftpJobId jobId, const QString &errorMessage);
    void handleFileInfo(QSsh::SftpJobId jobId, const QList<QSsh::SftpFileInfo> &fileInfoList);

signals:
    void status(int code);//0: diconnected 1: connected 2: disconnecting 3: connecting
                          //4: data listing //5: data listed
    void connectionError(const QString &error);
    void pwd(const QString &path);

private:
    bool m_isConnected;
    QString m_host;
    QString m_userName;
    QString m_password;

    QString m_pwd;

    QMap <QString, QString> m_iconMap;
    QList <QSsh::SftpFileInfo> m_fileList;

    QPointer<QSsh::SshConnection> m_sshconnection;
    QSsh::SftpChannel::Ptr m_sftptr;

};


}
#endif // QBSSHFILESYSTEMMODEL_H
