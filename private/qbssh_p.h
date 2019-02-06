#ifndef QBSSH_H
#define QBSSH_H


#include <QUrl>
#include <QMap>
#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QLoggingCategory>

#include "qboogmap_p.h"

#include <sftpchannel.h>
#include <sshconnection.h>
#include <sshremoteprocess.h>


namespace QbSsh {


///////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief The QbSsh class
///
///////////////////////////////////////////////////////////////////////////////////////////////////
class QbSsh : public QObject
{
    Q_OBJECT
private:
    bool m_isConnected;

    QString m_host;
    QString m_username;
    QString m_password;
    int m_port;
    int m_timeout;
    quint64 m_processId;

public:
    explicit QbSsh(QObject *parent = Q_NULLPTR);

    ~QbSsh();

    QSharedPointer<QSsh::SftpChannel> createSftpChannel();

    static void registerQML();

signals:
    void hostConnected();
    void hostConnectionError(const QString &error);
    void hostDisconnected();
    void hostReconnecting();

    void commandOutput(quint64 processId,const QByteArray &data);
    void commandStarted(quint64 processId);
    void commandError(quint64 processId, const QByteArray &error);

private slots:
    void onConnected();
    void onConnectionError(QSsh::SshError error);
    void onChannelStarted();
    void onOutputReadyRead();
    void onErrorReadyRead();
    void onDisconected();

public slots:
    void setHost(const QString &host);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setPort(int _port);
    void setTimeout(int _timeout);

    void setLoginDetails(const QString &host, const QString &username, const QString &password);

    void connectToHost();
    void reconnectToHost();
    void disconnectFromHost();

    bool isConnected();

    quint64 runCommand(const QByteArray &cmd);
    QString getHost();

    bool isRunning(quint64 pid);
    void kill(quint64 pid);
    void sendExitSignal(quint64 pid);
    int exitCode(quint64 pid);

private:
    void shutdown();
    void connectionCode();

private:
    QPointer<QSsh::SshConnection> m_sshconnection;

    QMap<QSsh::SshRemoteProcess*,quint64> m_processMap;
    QMap<QSsh::SshRemoteProcess*,QSsh::SshRemoteProcess::Ptr> m_processMapPtr;

    QbCore::QbOOGMap<QSsh::SshRemoteProcess*,quint64> m_nProcessMap;
};


}

#endif // QBSSH_H
