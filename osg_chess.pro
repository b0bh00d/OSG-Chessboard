TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        Callbacks.cpp \
        Chessboard.cpp \
        Game.cpp \
        Handlers.cpp \
        OSG_Chess.cpp \
        Visitors.cpp \

HEADERS += \
        Callbacks.h \
        Chessboard.h \
        Game.h \
        Handlers.h \
        OSG.h \
        Types.h \
        Visitors.h \

CONFIG(debug, debug|release) {
    win32 {
        INCLUDEPATH += Y:/Dev/OSG/debug/include
        LIBS += -losgd -losgDBd -losgViewerd -losgUtild -losgGAd -losgTextd
        LIBS += -LY:/Dev/OSG/debug/lib
    }
} else {
    win32 {
        INCLUDEPATH += Y:/Dev/OSG/release/include
        LIBS += -losg -losgDB -losgViewer -losgUtil -losgGA -losgText
        LIBS += -LY:/Dev/OSG/release/lib
    }
}

INTERMEDIATE_NAME = intermediate
MOC_DIR = $$INTERMEDIATE_NAME/moc
OBJECTS_DIR = $$INTERMEDIATE_NAME/obj
RCC_DIR = $$INTERMEDIATE_NAME/rcc
UI_DIR = $$INTERMEDIATE_NAME/ui
