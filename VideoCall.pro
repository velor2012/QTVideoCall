QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets multimedia-private core network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += SDL_MAIN_HANDLED
SOURCES += \
    Config.cpp \
    EnDeCoder/VideoEncoder.cpp \
    EnDeCoder/audiodecoder.cpp \
    EnDeCoder/audioencoder.cpp \
    EnDeCoder/videodecoder.cpp \
    Threads/audiodecodethread.cpp \
    Threads/audioencodethread.cpp \
    Threads/udphandler.cpp \
    Threads/videodecodethread.cpp \
    Threads/videoencodethread.cpp \
    cammicloader.cpp \
    main.cpp \
    mainwindow.cpp \
    mysessionmanager.cpp \
    player/audioplayer.cpp \
    player/videoplayerEnc.cpp \
    player/videoplayerdec.cpp \
    settingwindow.cpp

HEADERS += \
    Config.h \
    EnDeCoder/VideoEncoder.h \
    EnDeCoder/audiodecoder.h \
    EnDeCoder/audioencoder.h \
    EnDeCoder/videodecoder.h \
    MyBaseQThread.h \
    MyQueue.h \
    MyUDPPacket.h \
    MyUDPPacketBuilder.h \
    Threads/audiodecodethread.h \
    Threads/audioencodethread.h \
    Threads/udphandler.h \
    Threads/videodecodethread.h \
    Threads/videoencodethread.h \
    cammicloader.h \
    mainwindow.h \
    mysessionmanager.h \
    player/audioplayer.h \
    player/videoplayerEnc.h \
    player/videoplayerdec.h \
    settingwindow.h \
    tool.h

FORMS += \
    mainwindow.ui \
    settingwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/ffmpeg/ -lswscale -lavdevice -lavfilter -lavformat\
-lavresample -lavutil -lpostproc -lswresample -lswscale -lavcodec -lOleAut32

INCLUDEPATH += $$PWD/ffmpeg/include
DEPENDPATH += $$PWD/ffmpeg

win32: LIBS += -L$$PWD/lib/ -lSDL2

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
