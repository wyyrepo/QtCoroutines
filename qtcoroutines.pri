PUBLIC_HEADERS += \
	$$PWD/qtcoroutine.h \
	$$PWD/qtawaitables.h

HEADERS += $$PUBLIC_HEADERS \
	$$PWD/coroutine/coroutine.h \
	$$PWD/qtcoroutine_p.h

SOURCES += \
	$$PWD/qtcoroutine.cpp \
	$$PWD/qtawaitables.cpp

win32: SOURCES += $$PWD/qtcoroutine_win.cpp
else: SOURCES += $$PWD/qtcoroutine_unix.cpp

INCLUDEPATH += $$PWD
