QT += core gui webkit webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MultiWars2
TEMPLATE = app
DEFINES -= UNICODE

SOURCES += main.cpp\
        MainWindow.cpp \
    GameProfile.cpp \
    GameProfileListModel.cpp \
    GameProfileDelegate.cpp \
    IdenticonBuilder.cpp \
    GameLauncher.cpp

HEADERS  += MainWindow.h \
    GameProfile.h \
    GameProfileListModel.h \
    GameProfileDelegate.h \
    IdenticonBuilder.h \
    GameLauncher.h \
    data_types.h

RESOURCES += \
    resources.qrc

RC_FILE = windows_icon.rc

OTHER_FILES += \
    License.txt
