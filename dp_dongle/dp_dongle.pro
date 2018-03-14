TEMPLATE = lib
DEFINES += libGemaltoDongle
CONFIG += c++11

JDK=/usr/lib/jvm/java-8-openjdk-amd64
INCLUDEPATH +=$$JDK/include \
                $$JDK/include/linux

SOURCES += \
    com_beyondsoft_utils_GemaltoDongle.cpp

HEADERS += \
    hasp_vcode.h \
    hasp_api.h \
    com_beyondsoft_utils_GemaltoDongle.h

DISTFILES += \
    com.beyondsoft.utils.GemaltoDongle.TestGemaltoDongle.java

LIBS +=/root/workspace/sentinal/gemalto_dongle/dp_dongle/libhasp_linux_x86_64_29431.so
