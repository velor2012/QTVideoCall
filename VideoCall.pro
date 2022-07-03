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
    ffmpeg/include/libavcodec/ac3_parser.h \
    ffmpeg/include/libavcodec/adts_parser.h \
    ffmpeg/include/libavcodec/avcodec.h \
    ffmpeg/include/libavcodec/avdct.h \
    ffmpeg/include/libavcodec/avfft.h \
    ffmpeg/include/libavcodec/bsf.h \
    ffmpeg/include/libavcodec/codec.h \
    ffmpeg/include/libavcodec/codec_desc.h \
    ffmpeg/include/libavcodec/codec_id.h \
    ffmpeg/include/libavcodec/codec_par.h \
    ffmpeg/include/libavcodec/d3d11va.h \
    ffmpeg/include/libavcodec/dirac.h \
    ffmpeg/include/libavcodec/dv_profile.h \
    ffmpeg/include/libavcodec/dxva2.h \
    ffmpeg/include/libavcodec/jni.h \
    ffmpeg/include/libavcodec/mediacodec.h \
    ffmpeg/include/libavcodec/packet.h \
    ffmpeg/include/libavcodec/qsv.h \
    ffmpeg/include/libavcodec/vaapi.h \
    ffmpeg/include/libavcodec/vdpau.h \
    ffmpeg/include/libavcodec/version.h \
    ffmpeg/include/libavcodec/videotoolbox.h \
    ffmpeg/include/libavcodec/vorbis_parser.h \
    ffmpeg/include/libavcodec/xvmc.h \
    ffmpeg/include/libavdevice/avdevice.h \
    ffmpeg/include/libavdevice/version.h \
    ffmpeg/include/libavfilter/avfilter.h \
    ffmpeg/include/libavfilter/buffersink.h \
    ffmpeg/include/libavfilter/buffersrc.h \
    ffmpeg/include/libavfilter/version.h \
    ffmpeg/include/libavformat/avformat.h \
    ffmpeg/include/libavformat/avio.h \
    ffmpeg/include/libavformat/version.h \
    ffmpeg/include/libavresample/avresample.h \
    ffmpeg/include/libavresample/version.h \
    ffmpeg/include/libavutil/adler32.h \
    ffmpeg/include/libavutil/aes.h \
    ffmpeg/include/libavutil/aes_ctr.h \
    ffmpeg/include/libavutil/attributes.h \
    ffmpeg/include/libavutil/audio_fifo.h \
    ffmpeg/include/libavutil/avassert.h \
    ffmpeg/include/libavutil/avconfig.h \
    ffmpeg/include/libavutil/avstring.h \
    ffmpeg/include/libavutil/avutil.h \
    ffmpeg/include/libavutil/base64.h \
    ffmpeg/include/libavutil/blowfish.h \
    ffmpeg/include/libavutil/bprint.h \
    ffmpeg/include/libavutil/bswap.h \
    ffmpeg/include/libavutil/buffer.h \
    ffmpeg/include/libavutil/camellia.h \
    ffmpeg/include/libavutil/cast5.h \
    ffmpeg/include/libavutil/channel_layout.h \
    ffmpeg/include/libavutil/common.h \
    ffmpeg/include/libavutil/cpu.h \
    ffmpeg/include/libavutil/crc.h \
    ffmpeg/include/libavutil/des.h \
    ffmpeg/include/libavutil/dict.h \
    ffmpeg/include/libavutil/display.h \
    ffmpeg/include/libavutil/dovi_meta.h \
    ffmpeg/include/libavutil/downmix_info.h \
    ffmpeg/include/libavutil/encryption_info.h \
    ffmpeg/include/libavutil/error.h \
    ffmpeg/include/libavutil/eval.h \
    ffmpeg/include/libavutil/ffversion.h \
    ffmpeg/include/libavutil/fifo.h \
    ffmpeg/include/libavutil/file.h \
    ffmpeg/include/libavutil/frame.h \
    ffmpeg/include/libavutil/hash.h \
    ffmpeg/include/libavutil/hdr_dynamic_metadata.h \
    ffmpeg/include/libavutil/hmac.h \
    ffmpeg/include/libavutil/hwcontext.h \
    ffmpeg/include/libavutil/hwcontext_cuda.h \
    ffmpeg/include/libavutil/hwcontext_d3d11va.h \
    ffmpeg/include/libavutil/hwcontext_drm.h \
    ffmpeg/include/libavutil/hwcontext_dxva2.h \
    ffmpeg/include/libavutil/hwcontext_mediacodec.h \
    ffmpeg/include/libavutil/hwcontext_opencl.h \
    ffmpeg/include/libavutil/hwcontext_qsv.h \
    ffmpeg/include/libavutil/hwcontext_vaapi.h \
    ffmpeg/include/libavutil/hwcontext_vdpau.h \
    ffmpeg/include/libavutil/hwcontext_videotoolbox.h \
    ffmpeg/include/libavutil/hwcontext_vulkan.h \
    ffmpeg/include/libavutil/imgutils.h \
    ffmpeg/include/libavutil/intfloat.h \
    ffmpeg/include/libavutil/intreadwrite.h \
    ffmpeg/include/libavutil/lfg.h \
    ffmpeg/include/libavutil/log.h \
    ffmpeg/include/libavutil/lzo.h \
    ffmpeg/include/libavutil/macros.h \
    ffmpeg/include/libavutil/mastering_display_metadata.h \
    ffmpeg/include/libavutil/mathematics.h \
    ffmpeg/include/libavutil/md5.h \
    ffmpeg/include/libavutil/mem.h \
    ffmpeg/include/libavutil/motion_vector.h \
    ffmpeg/include/libavutil/murmur3.h \
    ffmpeg/include/libavutil/opt.h \
    ffmpeg/include/libavutil/parseutils.h \
    ffmpeg/include/libavutil/pixdesc.h \
    ffmpeg/include/libavutil/pixelutils.h \
    ffmpeg/include/libavutil/pixfmt.h \
    ffmpeg/include/libavutil/random_seed.h \
    ffmpeg/include/libavutil/rational.h \
    ffmpeg/include/libavutil/rc4.h \
    ffmpeg/include/libavutil/replaygain.h \
    ffmpeg/include/libavutil/ripemd.h \
    ffmpeg/include/libavutil/samplefmt.h \
    ffmpeg/include/libavutil/sha.h \
    ffmpeg/include/libavutil/sha512.h \
    ffmpeg/include/libavutil/spherical.h \
    ffmpeg/include/libavutil/stereo3d.h \
    ffmpeg/include/libavutil/tea.h \
    ffmpeg/include/libavutil/threadmessage.h \
    ffmpeg/include/libavutil/time.h \
    ffmpeg/include/libavutil/timecode.h \
    ffmpeg/include/libavutil/timestamp.h \
    ffmpeg/include/libavutil/tree.h \
    ffmpeg/include/libavutil/twofish.h \
    ffmpeg/include/libavutil/tx.h \
    ffmpeg/include/libavutil/version.h \
    ffmpeg/include/libavutil/video_enc_params.h \
    ffmpeg/include/libavutil/xtea.h \
    ffmpeg/include/libpostproc/postprocess.h \
    ffmpeg/include/libpostproc/version.h \
    ffmpeg/include/libswresample/swresample.h \
    ffmpeg/include/libswresample/version.h \
    ffmpeg/include/libswscale/swscale.h \
    ffmpeg/include/libswscale/version.h \
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

DISTFILES += \
    ffmpeg/OleAut32.Lib \
    ffmpeg/avcodec-58.dll \
    ffmpeg/avcodec.lib \
    ffmpeg/avdevice-58.dll \
    ffmpeg/avdevice.lib \
    ffmpeg/avfilter-7.dll \
    ffmpeg/avfilter.lib \
    ffmpeg/avformat-58.dll \
    ffmpeg/avformat.lib \
    ffmpeg/avresample-4.dll \
    ffmpeg/avresample.lib \
    ffmpeg/avutil-56.dll \
    ffmpeg/avutil.lib \
    ffmpeg/comsupp.lib \
    ffmpeg/ffmpeg.exe \
    ffmpeg/ffplay.exe \
    ffmpeg/ffprobe.exe \
    ffmpeg/postproc-55.dll \
    ffmpeg/postproc.lib \
    ffmpeg/swresample-3.dll \
    ffmpeg/swresample.lib \
    ffmpeg/swscale-5.dll \
    ffmpeg/swscale.lib \
    lib/SDL2.dll \
    lib/SDL2.lib \
    lib/SDL2main.lib \
    lib/SDL2test.lib \
    lib/x64/SDL2.dll \
    lib/x64/SDL2.lib \
    lib/x64/SDL2main.lib \
    lib/x64/SDL2test.lib \
    lib/x86/SDL2.dll \
    lib/x86/SDL2.lib \
    lib/x86/SDL2main.lib \
    lib/x86/SDL2test.lib
