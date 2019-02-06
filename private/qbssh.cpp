#include "qbssh_p.h"
#include "qbsshlogging_p.h"
#include "qbsshfs_p.h"

#include <QQmlApplicationEngine>

namespace QbSsh {

QbSsh::QbSsh(QObject *parent) : QObject(parent)
{
    qCDebug(Qb_Native_QbSsh())<<"QbSsh constructor";

    //QLoggingCategory::setFilterRules(QStringLiteral("qtc.ssh.debug=false"));
    this->m_isConnected = false;
    this->m_timeout = 30;//30 seconds
    this->m_port = 22;
    this->m_processId = 0;
}

QbSsh::~QbSsh()
{
    qCDebug(Qb_Native_QbSsh())<<"QbSsh desctrutor";
    this->shutdown();
    this->m_processMap.clear();
    this->m_processMapPtr.clear();
}

QSharedPointer<QSsh::SftpChannel> QbSsh::createSftpChannel()
{
    return this->m_sshconnection->createSftpChannel();
}

void QbSsh::registerQML()
{
    qmlRegisterType<QbSsh>("Qb.Ssh",1,0,"QbSsh");
    qmlRegisterType<QbSshFS>("Qb.Ssh",1,0,"QbSshFS");
}

bool QbSsh::isConnected()
{
    return this->m_isConnected;
}

void QbSsh::onConnected()
{
    qCDebug(Qb_Native_QbSsh)<<"Server Connected";

    this->m_isConnected = true;
    emit hostConnected();
}

void QbSsh::onConnectionError(QSsh::SshError error)
{
    QString errorString = this->m_sshconnection->errorString();
    Q_UNUSED(error)
    qCDebug(Qb_Native_QbSsh)<<"Error: "<< errorString;
    this->m_isConnected = false;
    emit hostConnectionError(errorString);
}

void QbSsh::onChannelStarted()
{
    qCDebug(Qb_Native_QbSsh)<< "Command Communication started";
    QSsh::SshRemoteProcess *rp = dynamic_cast<QSsh::SshRemoteProcess*>(this->sender());
    if(this->m_processMap.contains(rp)){
        //qCDebug(Qb_Native_QbSsh)<<"Remote process found";
        emit commandStarted(this->m_processMap.value(rp));
    }
}

void QbSsh::onOutputReadyRead()
{
    qCDebug(Qb_Native_QbSsh)<< "Command is executed and now ready for reading";
    QSsh::SshRemoteProcess *rp = dynamic_cast<QSsh::SshRemoteProcess*>(this->sender());

    if(this->m_processMap.contains(rp)){
        this->m_nProcessMap.removeByKey(rp);
        quint64 pid = this->m_processMap.take(rp);
        emit commandOutput(pid,rp->readAllStandardOutput());
        QSsh::SshRemoteProcess::Ptr rptr = this->m_processMapPtr.take(rp);
        rptr.clear();
    }
}

void QbSsh::onErrorReadyRead()
{
    qCDebug(Qb_Native_QbSsh)<< "Command is executed and got error";
    QSsh::SshRemoteProcess *rp = dynamic_cast<QSsh::SshRemoteProcess*>(this->sender());

    if(this->m_processMap.contains(rp)){
        this->m_nProcessMap.removeByKey(rp);
        quint64 pid = this->m_processMap.take(rp);
        emit commandError(pid,rp->readAllStandardError());
        QSsh::SshRemoteProcess::Ptr rptr = this->m_processMapPtr.take(rp);
        rptr.clear();
    }
}

void QbSsh::onDisconected()
{
    this->m_isConnected = false;
    emit hostDisconnected();
    this->m_nProcessMap.clear();
    this->m_processMap.clear();
    this->m_processMapPtr.clear();
}

void QbSsh::setHost(const QString &host)
{
    if(this->m_host != host){
        this->m_host = host;
    }
}

void QbSsh::setUsername(const QString &username)
{
    if(this->m_username != username){
        this->m_username = username;
    }
}

void QbSsh::setPassword(const QString &password)
{
    if(this->m_password != password){
        this->m_password = password;
    }
}

void QbSsh::setPort(int _port)
{
    if(this->m_port!=_port){
        this->m_port = _port;
    }
}

void QbSsh::setTimeout(int _timeout)
{
    if(this->m_timeout!=_timeout){
        this->m_timeout = _timeout;
    }
}

void QbSsh::setLoginDetails(const QString &host, const QString &username, const QString &password)
{
    this->setHost(host);
    this->setUsername(username);
    this->setPassword(password);
}

void QbSsh::reconnectToHost()
{
    this->shutdown();
    emit hostReconnecting();
    this->connectionCode();
}

void QbSsh::disconnectFromHost()
{
    this->shutdown();
}

void QbSsh::connectToHost()
{
    if(this->m_isConnected){
        emit hostConnectionError(QLatin1String("Already connected to a host."));
        return;
    }

    this->connectionCode();
}

quint64 QbSsh::runCommand(const QByteArray &cmd)
{
    //if(this->m_rp) delete this->m_rp.data();
    //    if(!this->m_rp.isNull()){
    //        this->m_rp.clear();
    //    }
    if(this->m_sshconnection.isNull()) return 0;
    if(this->m_processId>18446744073){
        this->m_processId = 0;
    }

    quint64 pid = ++this->m_processId;
    QSsh::SshRemoteProcess::Ptr rp = this->m_sshconnection->createRemoteProcess(cmd);

    this->m_processMap.insert(rp.data(),pid);
    this->m_processMapPtr.insert(rp.data(),rp);
    this->m_nProcessMap.append(rp.data(),pid);

    qCDebug(Qb_Native_QbSsh())<<"Executing command: "<<cmd;
    qCDebug(Qb_Native_QbSsh())<<"Command ID: "<<  pid;

    if(rp){
        this->connect(rp.data(),
                      &QSsh::SshRemoteProcess::started,
                      this,
                      &QbSsh::onChannelStarted);
        this->connect(rp.data(),
                      &QSsh::SshRemoteProcess::readyReadStandardOutput,
                      this,
                      &QbSsh::onOutputReadyRead
                      );
        this->connect(rp.data(),
                      &QSsh::SshRemoteProcess::readyReadStandardError,
                      this,
                      &QbSsh::onErrorReadyRead);

        rp->start();
        return pid;
    }

    return 0;

}

QString QbSsh::getHost()
{
    return this->m_host;
}

bool QbSsh::isRunning(quint64 pid)
{
    if(this->m_nProcessMap.isValueExists(pid)){
        return this->m_nProcessMap.getByValue(pid)->isRunning();
    }
    return false;
}

void QbSsh::kill(quint64 pid)
{
    if(this->m_nProcessMap.isValueExists(pid)){
        this->m_nProcessMap.getByValue(pid)->kill();
    }
}

void QbSsh::sendExitSignal(quint64 pid)
{
    if(this->m_nProcessMap.isValueExists(pid)){
        this->m_nProcessMap.getByValue(pid)->sendSignal(QSsh::SshRemoteProcess::QuitSignal);
    }
}

int QbSsh::exitCode(quint64 pid)
{
    if(this->m_nProcessMap.isValueExists(pid)){
        return this->m_nProcessMap.getByValue(pid)->exitCode();
    }
    return -100;
}

void QbSsh::shutdown()
{
    qCDebug(Qb_Native_QbSsh())<<"Shutting down SSH connection from HOST:"<<this->m_host;
    if (!this->m_sshconnection.isNull()) {
        qCDebug(Qb_Native_QbSsh())<<"Disconnecting SIGNALS";
        this->disconnect(this->m_sshconnection.data(), 0, this, 0);
        delete this->m_sshconnection.data();
        this->m_sshconnection.clear();
    }
    this->m_isConnected = false;
    emit hostDisconnected();
}

void QbSsh::connectionCode()
{

    if(this->m_host.isEmpty()){
        emit hostConnectionError(QLatin1String("Host can not be empty."));
        return;
    }

    if(this->m_username.isEmpty()){
        emit hostConnectionError(QLatin1String("Username can not be empty."));
        return;
    }

    if(this->m_password.isEmpty()){
        emit hostConnectionError(QLatin1String("Password can not be empty."));
    }


    QSsh::SshConnectionParameters params;
    params.host = this->m_host;
    params.userName = this->m_username;
    params.password = this->m_password;
    params.timeout = this->m_timeout;
    params.port = quint16(this->m_port);
    params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword;

    this->m_sshconnection = new QSsh::SshConnection(params);

    this->connect(this->m_sshconnection.data(),
                  &QSsh::SshConnection::connected,
                  this,
                  &QbSsh::onConnected);
    this->connect(this->m_sshconnection.data(),
                  &QSsh::SshConnection::disconnected,
                  this,
                  &QbSsh::onDisconected
                  );
    this->connect(this->m_sshconnection.data(),
                  &QSsh::SshConnection::error,
                  this,
                  &QbSsh::onConnectionError);

    qCDebug(Qb_Native_QbSsh())<<"Connecting to HOST: "<<this->m_host<<":"<<this->m_port;

    this->m_sshconnection->connectToHost();
}



}
