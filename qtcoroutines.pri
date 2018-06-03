PUBLIC_HEADERS += $$PWD/qtcoroutine.h

HEADERS += $$PUBLIC_HEADERS \
	$$PWD/coroutine/coroutine.h \
    $$PWD/qtawaitables.h

SOURCES += \
	$$PWD/qtcoroutine.cpp \
    $$PWD/qtawaitables.cpp

INCLUDEPATH += $$PWD
