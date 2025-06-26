QT += core network sql
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = MineSweeperServer
TEMPLATE = app

SOURCES += \
    db_manager.cpp \
    main.cpp \
    minesweeper_server.cpp \
    game_model.cpp \

HEADERS += \
    db_manager.h \
    minesweeper_server.h \
    game_model.h \
    db_manager.h
