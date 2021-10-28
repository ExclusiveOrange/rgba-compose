QT       += core gui widgets

CONFIG += c++latest

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ChannelUi.cc \
    GetImageSizeDialog.cc \
    getInputImageFilenameFilter.cc \
    getOutputImageFilenameFilter.cc \
    main.cc \
    RgbaComposer.cc

HEADERS += \
    ChannelUi.hh \
    Constants.hh \
    Defaults.hh \
    Destroyer.hh \
    Enums.hh \
    GetImageSizeDialog.hh \
    RgbaComposer.hh \
    Settings.hh \
    getInputImageFilenameFilter.hh \
    getOutputImageFilenameFilter.hh

FORMS += \
    GetImageSizeDialog.ui \
    RgbaComposer.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
