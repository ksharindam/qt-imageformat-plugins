#pragma once
#include <QImageIOPlugin>

// In Qt5+ this class must be in a c++ header file.
// So it hasbeen moved here from jp2-plugin.cpp
class Jp2Plugin : public QImageIOPlugin
{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "jp2.json")
#endif
public:
    QStringList      keys() const;//for Qt4 only
    Capabilities     capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler* create(QIODevice *device, const QByteArray &format = QByteArray()) const;
private:
    QStringList supported = {"jp2", "j2k", "j2c", "jpf", "jpx", "jpm"};
};
