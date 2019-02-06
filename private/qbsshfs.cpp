#include "qbsshfs_p.h"


namespace QbSsh {


QbSshFS::QbSshFS(QObject *parent) : QObject(parent)
{
    this->m_ssh = new QbSsh();
    this->m_pwd = QLatin1String("/");

    this->connect(this->m_ssh,&QbSsh::hostConnected,this,&QbSshFS::hostConnected);
    this->connect(this->m_ssh,&QbSsh::hostDisconnected,this,&QbSshFS::hostDisconnected);
    this->connect(this->m_ssh,&QbSsh::hostReconnecting,this,&QbSshFS::hostReconnecting);
    this->connect(this->m_ssh,&QbSsh::hostConnectionError,this,&QbSshFS::hostConnectionError);

    this->connect(this->m_ssh,&QbSsh::commandStarted,this,&QbSshFS::commandStarted);
    this->connect(this->m_ssh,&QbSsh::commandOutput,this,&QbSshFS::commandOutput);
    this->connect(this->m_ssh,&QbSsh::commandError,this,&QbSshFS::commandError);
}

void QbSshFS::setHost(const QString &host)
{
    this->m_ssh->setHost(host);
}

void QbSshFS::setUsername(const QString &username)
{
    this->m_ssh->setUsername(username);
}

void QbSshFS::setPassword(const QString &password)
{
    this->m_ssh->setPassword(password);
}

void QbSshFS::setPort(int _port)
{
    this->m_ssh->setPort(_port);
}

void QbSshFS::setTimeout(int _timeout)
{
    this->m_ssh->setTimeout(_timeout);
}

void QbSshFS::connectToHost()
{
    this->m_ssh->connectToHost();
}

void QbSshFS::reconnectToHost()
{
    this->m_ssh->reconnectToHost();
}

void QbSshFS::disconnectFromHost()
{
    if (this->m_sftptr) {
        this->disconnect(this->m_sftptr.data(), 0, this, 0);
        this->m_sftptr->closeChannel();
        this->m_sftptr.clear();
    }

    this->m_ssh->disconnectFromHost();
}

bool QbSshFS::isConnected()
{
    return this->m_ssh->isConnected();
}

QVariantList QbSshFS::getModel()
{
    return this->m_dataList;
}

void QbSshFS::cd(const QString &path)
{
    this->m_pwd = path;
    this->listDir();
}

QString QbSshFS::pwd()
{
    return this->m_pwd;
}

QString QbSshFS::getSFTPFullPath(const QVariantMap &data, const QString &username, const QString &password)
{
    QString url;
    //QVariantMap data = this->m_dataList.at(data).toMap();
    if(data["type"].toString() == QLatin1String("regular")){
        url.append("sftp://");
        url.append(username);
        url.append(":");
        url.append(password);
        url.append("@");
        url.append(this->m_ssh->getHost());
        url.append(this->m_pwd+QLatin1String("/") + data["name"].toString());
    }

    return url;
}

QString QbSshFS::backPath()
{
    if(this->m_pwd != QLatin1String("/")){
        QStringList dl = this->m_pwd.split(QLatin1String("/"),QString::SkipEmptyParts);
        if(dl.size()>0){
            dl.removeLast();
            QString npath = dl.join(QLatin1String("/"));
            npath.prepend("/");
            return npath;
        }
    }
    return QLatin1String("/");
}

quint64 QbSshFS::runCommand(const QByteArray &cmd)
{
    return this->m_ssh->runCommand(cmd);
}


QbSshFS::~QbSshFS()
{
    this->m_ssh->disconnectFromHost();
    delete this->m_ssh;
}

void QbSshFS::listDir()
{
    if(!this->m_sftptr.isNull()){
        this->m_dataList.clear();
        emit status(5);//data listing
        this->m_fileListJobId = this->m_sftptr->listDirectory(this->m_pwd);
    }
}

void QbSshFS::hostConnected()
{
    emit status(1);

    this->m_sftptr = this->m_ssh->createSftpChannel();
    this->connect(this->m_sftptr.data(), &QSsh::SftpChannel::initialized,
                  this, &QbSshFS::handleSftpChannelInitialized);
    this->connect(this->m_sftptr.data(), &QSsh::SftpChannel::channelError,
                  this, &QbSshFS::handleSftpChannelError);

    this->m_sftptr->initialize();
}

void QbSshFS::hostConnectionError(const QString &msg)
{
    emit status(0);
    emit error(msg);
}

void QbSshFS::hostDisconnected()
{
    emit status(0);
}

void QbSshFS::hostReconnecting()
{
    emit status(3);
}

void QbSshFS::handleSftpChannelInitialized()
{
    this->connect(this->m_sftptr.data(),
                  &QSsh::SftpChannel::fileInfoAvailable,
                  this, &QbSshFS::handleFileInfo);
    this->connect(this->m_sftptr.data(), &QSsh::SftpChannel::finished,
                  this, &QbSshFS::handleSftpJobFinished);

    emit status(4);
}

void QbSshFS::handleSftpChannelError(const QString &reason)
{
    emit error(reason);
    this->disconnectFromHost();
}

void QbSshFS::handleSftpJobFinished(QSsh::SftpJobId jobId, const QString &errorMessage)
{
    Q_UNUSED(jobId)
    Q_UNUSED(errorMessage)
    if(this->m_fileListJobId==jobId)
    {
        this->m_fileListJobId = -1;
        emit status(6);//data list done

        QStringList dl = this->m_pwd.split(QLatin1String("/"),QString::SkipEmptyParts);
        if(dl.size()>0){
            emit currentDirectory(dl.last());
        }
    }
}

void QbSshFS::handleFileInfo(QSsh::SftpJobId jobId, const QList<QSsh::SftpFileInfo> &fileInfoList)
{
    Q_UNUSED(jobId);

    //this->m_dataList.clear();
    foreach (const QSsh::SftpFileInfo &fi, fileInfoList) {
        if(fi.name != QLatin1String(".") && fi.name != QLatin1String("..")){
            QVariantMap data;
            data["name"] = fi.name;
            data["size"] = fi.size;
            data["sizeValid"] = fi.sizeValid;

            if(fi.type == QSsh::SftpFileType::FileTypeDirectory){
                data["type"] = "dir";
            }
            else if(fi.type == QSsh::SftpFileType::FileTypeRegular){
                data["type"] = "regular";
            }
            else if(fi.type == QSsh::SftpFileType::FileTypeOther){
                data["type"] = "other";
            }
            else{
                data["type"] = "unknown";
            }
            this->m_dataList.append(data);
        }
    }

}



}
