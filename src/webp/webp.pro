TARGET  = qwebp
TEMPLATE = lib
CONFIG += plugin
CONFIG -= debug_and_release debug
QMAKE_CXXFLAGS = -std=c++11

INCLUDEPATH +=
LIBS += -lwebp
#LIBS += C:\\Full\\Path\\to\\libwebp.a

target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target

DESTDIR = ..
BUILD_DIR = ../../build
MOC_DIR =     $$BUILD_DIR
RCC_DIR =     $$BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR
mytarget.commands += $${QMAKE_MKDIR} $$BUILD_DIR

HEADERS += $$files(*.h)
SOURCES += $$files(*.cpp)
