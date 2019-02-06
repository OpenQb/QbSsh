TEMPLATE = lib
TARGET = QbSsh
QT += qml quick core
QT += widgets

CONFIG += plugin c++11

INCLUDEPATH += $$PWD


TARGET = $$qtLibraryTarget($$TARGET)
uri = Qb.Ssh



include(QbSsh.pri)

DISTFILES = qmldir QbSsh.qmltypes

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmltypes.commands = qmlplugindump -nonrelocatable Qb.Ssh 1.0 > $$PWD/QbSsh.qmltypes
QMAKE_EXTRA_TARGETS += qmltypes

qmldir.files = qmldir QbSsh.qmltypes
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}

HEADERS += \
    qbssh_plugin.h

SOURCES += \
    qbssh_plugin.cpp