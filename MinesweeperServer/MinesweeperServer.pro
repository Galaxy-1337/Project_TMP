QT = core network testlib

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Sources/core/authhandler.cpp \
        Sources/core/gamelogic.cpp \
        Sources/core/gameroom.cpp \
        Sources/core/gameroommanager.cpp \
        Sources/core/gameserver.cpp \
        Sources/main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Headers/core/authhandler.h \
    Headers/core/clientinfo.h \
    Headers/core/gamelogic.h \
    Headers/core/gameroom.h \
    Headers/core/gameroommanager.h \
    Headers/core/gameserver.h \
    Headers/network/protocol_defines.h
