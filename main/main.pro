TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

JDK=/usr/lib/jvm/java-8-openjdk-amd64
INCLUDEPATH +=$$JDK/include \
                $$JDK/include/linux

SOURCES += main.cpp

LIB_PATH=/root/workspace/sentinal/gemalto_dongle/dp_dongle
LIBS +=$$LIB_PATH/libhasp_linux_x86_64_29431.so \
    $$LIB_PATH/libdp_dongle.so.1.0.0 \
$$LIB_PATH/libdp_dongle.so.1.0 \
$$LIB_PATH/libdp_dongle.so.1 \
$$LIB_PATH/libdp_dongle.so

