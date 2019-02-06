#include "qbssh_plugin.h"
#include <qqml.h>

#include <QbSsh>

void QbSshPlugin::registerTypes(const char *uri)
{
    // @uri Qb.Ssh
    Q_UNUSED(uri);
    QbSsh::QbSsh::registerQML();
}

