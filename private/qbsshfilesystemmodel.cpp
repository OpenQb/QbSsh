#include "qbsshfilesystemmodel.h"

#include "qbsshlogging_p.h"


namespace QbSsh {

QbSshFileInfo::QbSshFileInfo()
{
    this->m_name = QLatin1String();
    this->m_permissionsValid = false;
    this->m_sizeValid = false;
}

QbSshFileInfo QbSshFileInfo::form(const QString &pwd, const QSsh::SftpFileInfo &fi)
{
    QbSshFileInfo nfi;
    nfi.m_name = fi.name;
    nfi.m_path = pwd;
    nfi.m_permissions = fi.permissions;
    nfi.m_permissionsValid = fi.permissionsValid;
    nfi.m_size = fi.size;
    nfi.m_sizeValid = fi.sizeValid;
    nfi.m_type = fi.type;
    return nfi;
}


QbSshFileSystemModel::QbSshFileSystemModel(QObject *parent):
    QAbstractListModel(parent)
{
    this->m_pwd = QLatin1String("/");
}

void QbSshFileSystemModel::setRootDirectory(const QString &path)
{
    this->m_pwd = path;
}


bool QbSshFileSystemModel::isConnected()
{
    return this->m_isConnected;
}

void QbSshFileSystemModel::addIcon(const QString &ext, const QString &path)
{
    this->m_iconMap.insert(ext,path);
}

void QbSshFileSystemModel::setIconMap(const QMap<QString, QString> &iconMap)
{
    this->m_iconMap = iconMap;
}

void QbSshFileSystemModel::clearIconMap()
{
    this->m_iconMap.clear();
}

bool QbSshFileSystemModel::isFolder(const QModelIndex &index)
{
    if(index.isValid() && index.row()<this->m_fileList.size()){
        QSsh::SftpFileInfo fi = this->m_fileList.at(index.row());
        return fi.type == QSsh::SftpFileType::FileTypeDirectory;
    }

    return false;
}

bool QbSshFileSystemModel::isFile(const QModelIndex &index)
{
    if(index.isValid() && index.row()<this->m_fileList.size()){
        QSsh::SftpFileInfo fi = this->m_fileList.at(index.row());
        return fi.type == QSsh::SftpFileType::FileTypeRegular;
    }
    return false;
}

QString QbSshFileSystemModel::getName(const QModelIndex &index)
{
    QString name;
    if(index.isValid() && index.row()<this->m_fileList.size()){
        QSsh::SftpFileInfo fi = this->m_fileList.at(index.row());
        name = fi.name;
    }

    return name;
}

QString QbSshFileSystemModel::getSFTPFullPath(const QModelIndex &index)
{
    QString url;
    if(index.isValid() && index.row()<this->m_fileList.size()){
        QSsh::SftpFileInfo fi = this->m_fileList.at(index.row());
        if(fi.type == QSsh::SftpFileType::FileTypeRegular){
            //qDebug() << this->m_pwd;
            url.append("sftp://");
            url.append(this->m_userName);
            url.append(":");
            url.append(this->m_password);
            url.append("@");
            url.append(this->m_host);
            url.append(this->m_pwd+QLatin1String("/") + fi.name);
        }
    }

    return url;
}

QbSshFileInfo QbSshFileSystemModel::getFile(const QModelIndex &index)
{
    QbSshFileInfo fi;
     if(index.isValid() && index.row()<this->m_fileList.size()){
        QSsh::SftpFileInfo sfi = this->m_fileList.at(index.row());
        fi = QbSshFileInfo::form(this->m_pwd,sfi);
     }
     return fi;
}

QVariant QbSshFileSystemModel::data(const QModelIndex &index, int role) const
{
    if(index.row() >= this->m_fileList.size() ){
        return QVariant();
    }


    QSsh::SftpFileInfo fi = this->m_fileList.at(index.row());

    if( index.column() == 0){
        if(role == Qt::DisplayRole){
            return fi.name;
        }
        if(role == Qt::DecorationRole){
            QFileIconProvider icon;
            switch (fi.type) {
            case QSsh::FileTypeRegular:
            case QSsh::FileTypeOther:
                return icon.icon(QFileIconProvider::File);
            case QSsh::FileTypeDirectory:
                return icon.icon(QFileIconProvider::Folder);
            case QSsh::FileTypeUnknown:
                return icon.icon(QFileIconProvider::File);
            }
        }
    }

    return QVariant();
}

QString QbSshFileSystemModel::getPWD()
{
    return this->m_pwd;
}

int QbSshFileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

Qt::ItemFlags QbSshFileSystemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant QbSshFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

int QbSshFileSystemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return this->m_fileList.size();
}

void QbSshFileSystemModel::navigateToPath(const QString &path)
{
    this->m_pwd = path;
    this->listDir();
}

void QbSshFileSystemModel::navigateToBack()
{
    if(this->m_pwd != QLatin1String("/")){
        QStringList dl = this->m_pwd.split(QLatin1String("/"),QString::SkipEmptyParts);
        if(dl.size()>0){
            dl.removeLast();
            QString npath = dl.join(QLatin1String("/"));
            npath.prepend("/");
            this->navigateToPath(npath);
        }
    }
}

void QbSshFileSystemModel::refresh()
{
    this->listDir();
}

void QbSshFileSystemModel::connectToHost(const QString &host, const QString &username, const QString &password)
{
    bool needLogin = false;
    if(this->m_host!=host){
        this->m_host = host;
        needLogin = true;
    }
    if(this->m_userName!=username){
        this->m_userName = username;
        needLogin = true;
    }
    if(this->m_password!=password){
        this->m_password = password;
        needLogin = true;
    }

    if(!this->m_isConnected && needLogin){

    }
    else if(this->m_isConnected && needLogin){
        if(!this->m_sshconnection.isNull()){
            this->m_sshconnection.clear();
        }
    }
    else if(this->m_isConnected && !needLogin){
        return;
    }

    this->connectionCode();
}

void QbSshFileSystemModel::reconnectToHost()
{
    this->shutdown();
    emit status(3);//reconnecting
    this->connectionCode();
}

void QbSshFileSystemModel::listDir()
{
    if(!this->m_sftptr.isNull()){
        emit status(4);//data listing
        this->m_sftptr->listDirectory(this->m_pwd);
    }
}

void QbSshFileSystemModel::connectionCode()
{
    QSsh::SshConnectionParameters params;
    params.host = this->m_host;
    params.userName = this->m_userName;
    params.password = this->m_password;
    params.timeout = 30;
    params.port = 22;
    params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword;


    this->m_sshconnection = new QSsh::SshConnection(params, this);


    this->connect(this->m_sshconnection.data(),
                  &QSsh::SshConnection::error,
                  this,
                  &QbSshFileSystemModel::handleSshConnectionFailure);


//    if (d->sshConnection->state() == SshConnection::Connected) {
//        handleSshConnectionEstablished();
//        return;
//    }


    this->connect(this->m_sshconnection.data(),
                     &QSsh::SshConnection::connected,
                     this,
                     &QbSshFileSystemModel::handleSshConnectionEstablished);

//    if (d->sshConnection->state() == SshConnection::Unconnected)
    this->m_sshconnection->connectToHost();
}

void QbSshFileSystemModel::shutdown()
{
    emit status(2);//diconnecting
    if (this->m_sftptr) {
        this->disconnect(this->m_sftptr.data(), 0, this, 0);
        this->m_sftptr->closeChannel();
        this->m_sftptr.clear();
    }

    if (!this->m_sshconnection.isNull()) {
        this->disconnect(this->m_sshconnection.data(), 0, this, 0);
        this->m_sshconnection.clear();
    }

    this->m_isConnected = false;
    emit status(0);//disconnected
}

void QbSshFileSystemModel::handleSshConnectionFailure()
{
    emit connectionError(this->m_sshconnection->errorString());
    beginResetModel();
    this->shutdown();
    endResetModel();
}

void QbSshFileSystemModel::handleSshConnectionEstablished()
{
    this->m_isConnected = true;
    emit status(1);

    this->m_sftptr = this->m_sshconnection->createSftpChannel();
    this->connect(this->m_sftptr.data(), &QSsh::SftpChannel::initialized,
            this, &QbSshFileSystemModel::handleSftpChannelInitialized);
    this->connect(this->m_sftptr.data(), &QSsh::SftpChannel::channelError,
            this, &QbSshFileSystemModel::handleSftpChannelError);

    this->m_sftptr->initialize();
}

void QbSshFileSystemModel::handleSftpChannelInitialized()
{
    connect(this->m_sftptr.data(),
        &QSsh::SftpChannel::fileInfoAvailable,
        this, &QbSshFileSystemModel::handleFileInfo);
    connect(this->m_sftptr.data(), &QSsh::SftpChannel::finished,
        this, &QbSshFileSystemModel::handleSftpJobFinished);

    this->listDir();
}

void QbSshFileSystemModel::handleSftpChannelError(const QString &reason)
{
    emit connectionError(reason);
    beginResetModel();
    this->shutdown();
    endResetModel();
}

void QbSshFileSystemModel::handleSftpJobFinished(QSsh::SftpJobId jobId, const QString &errorMessage)
{
    Q_UNUSED(jobId)
    Q_UNUSED(errorMessage)
}

void QbSshFileSystemModel::handleFileInfo(QSsh::SftpJobId jobId, const QList<QSsh::SftpFileInfo> &fileInfoList)
{
    Q_UNUSED(jobId)

    beginResetModel();
    this->m_fileList.clear();
    endResetModel();

    foreach (const QSsh::SftpFileInfo &fi, fileInfoList) {
        if(fi.name != QLatin1String(".") && fi.name != QLatin1String(".."))
            this->m_fileList.append(fi);
    }

    beginInsertRows(QModelIndex(), 0, this->m_fileList.size()-1);
    endInsertRows();
    emit status(5);//data list done

    qCDebug(Qb_Native_QbSsh)<<"Data listing done";

}


}

