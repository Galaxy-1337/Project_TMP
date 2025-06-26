QT += testlib network sql
CONFIG += console c++11
CONFIG -= app_bundle gui
TARGET = MineSweeperServerTest
TEMPLATE = app

# Тестовые файлы
SOURCES += tst_minesweeper.cpp \
           ../Server/minesweeper_server.cpp \
           ../Server/game_model.cpp \
           ../Server/db_manager.cpp

HEADERS += tst_minesweeper.h \
           ../Server/minesweeper_server.h \
           ../Server/game_model.h \
           ../Server/db_manager.h

INCLUDEPATH += ../Server

# Каталоги сборки
DESTDIR = $$PWD/../../build/MineSweeperTest
MOC_DIR = $$DESTDIR/moc
OBJECTS_DIR = $$DESTDIR/obj
