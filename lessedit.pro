#-------------------------------------------------
#
# Project created by QtCreator 2012-08-13T00:54:32
#
#-------------------------------------------------

QT       += core gui
QT       += webkit

LIBS += -ltidy

TARGET = lessedit
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    codewidget.cpp \
    htmlhighlighter.cpp \
    tidy.cpp \
    renderer.cpp \
    sundown/stack.c \
    sundown/markdown.c \
    sundown/html_smartypants.c \
    sundown/html.c \
    sundown/houdini_html_e.c \
    sundown/houdini_href_e.c \
    sundown/buffer.c \
    sundown/autolink.c \
    findreplace/findreplaceform.cpp \
    findreplace/findreplacedialog.cpp \
    findreplace/findform.cpp \
    findreplace/finddialog.cpp

HEADERS  += mainwindow.h \
    codewidget.h \
    htmlhighlighter.h \
    tidy.h \
    renderer.h \
    sundown/stack.h \
    sundown/markdown.h \
    sundown/html_blocks.h \
    sundown/html.h \
    sundown/houdini.h \
    sundown/buffer.h \
    sundown/autolink.h \
    findreplace/findreplace_global.h \
    findreplace/findreplaceform.h \
    findreplace/findreplacedialog.h \
    findreplace/findform.h \
    findreplace/finddialog.h

FORMS += \
    findreplace/findreplaceform.ui \
    findreplace/findreplacedialog.ui

RESOURCES += \
    resources.qrc
