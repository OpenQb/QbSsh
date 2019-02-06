#ifndef QBSSHFS_H
#define QBSSHFS_H

#include <QFile>
#include <QStack>
#include <QObject>
#include <QVariantMap>
#include <QVariantList>


#include "qbssh_p.h"


namespace QbSsh {

class QbSshFS : public QObject
{
    Q_OBJECT
private:
    quint32 m_fileListJobId;

public:
    explicit QbSshFS(QObject *parent = Q_NULLPTR);

    ~QbSshFS();

private:
    void listDir();

private slots:
    void hostConnected();
    void hostConnectionError(const QString &msg);
    void hostDisconnected();
    void hostReconnecting();


    void handleSftpChannelInitialized();
    void handleSftpChannelError(const QString &reason);

    void handleSftpJobFinished(QSsh::SftpJobId jobId, const QString &errorMessage);
    void handleFileInfo(QSsh::SftpJobId jobId, const QList<QSsh::SftpFileInfo> &fileInfoList);

signals:
    void status(int code);//0: diconnected 1: connected 2: disconnecting 3: connecting //4: sftp ready //5: data listing //6: data listed
    void error(const QString &message);
    void pwd(const QString &path);

    void commandOutput(quint64 processId,const QByteArray &data);
    void commandStarted(quint64 processId);
    void commandError(quint64 processId, const QByteArray &error);


    void currentDirectory(const QString &name);


public slots:
    void setHost(const QString &host);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setPort(int _port);
    void setTimeout(int _timeout);

    void connectToHost();
    void reconnectToHost();
    void disconnectFromHost();

    bool isConnected();

    QVariantList getModel();

    void cd(const QString &path);

    QString pwd();


    QString getSFTPFullPath(const QVariantMap &data,const QString &username, const QString &password);

    QString backPath();

    quint64 runCommand(const QByteArray &cmd);

private:
    QbSsh *m_ssh;
    QSsh::SftpChannel::Ptr m_sftptr;
    QString m_pwd;
    QVariantList m_dataList;

    QStringList m_historyList;
};


}
#endif // QBSSHFS_H
