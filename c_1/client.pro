QT += core network widgets
CONFIG += c++11

TARGET = MineSweeperClient
TEMPLATE = app

SOURCES += \
    main.cpp \
    game_controller.cpp \
    game_model.cpp \
    game_view.cpp

HEADERS += \
    game_controller.h \
    game_model.h \
    game_view.h

FORMS += \
    game_view.ui
