INCLUDEPATH += $$PWD
include(ext/QSsh/ssh/ssh.pri)


HEADERS += \
    $$PWD/QbSsh \
    $$PWD/QbSshFS \
    $$PWD/private/qbsshlogging_p.h \
    $$PWD/private/qbsshfilesystemmodel.h \
    $$PWD/private/qbsshfs_p.h \
    $$PWD/private/qbssh_p.h \
    $$PWD/private/qboogmap_p.h

SOURCES += \
    $$PWD/private/qbssh.cpp \
    $$PWD/private/qbsshlogging.cpp \
    $$PWD/private/qbsshfilesystemmodel.cpp \
    $$PWD/private/qbsshfs.cpp
