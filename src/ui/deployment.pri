android-no-sdk {
    target.path = /data/user/qt
    export(target.path)
    INSTALLS += target
} else:android {
#    INCLUDEPATH += /usr/include/c++/4.9 /usr/include/x86_64-linux-gnu/c++/4.9 /usr/include/x86_64-linux-gnu /usr/include/c++/4.9/backward /usr/lib/gcc/x86_64-linux-gnu/4.9/include /usr/local/include /usr/lib/gcc/x86_64-linux-gnu/4.9/include-fixed /usr/include
    x86 {
        target.path = /libs/x86
    } else: armeabi-v7a {
        target.path = /libs/armeabi-v7a
    } else {
        target.path = /libs/armeabi
    }
    export(target.path)
    INSTALLS += target
} else:unix {
    DEPLOYMENT += addFiles
    isEmpty(target.path) {
        qnx {
            target.path = /tmp/$${TARGET}/bin
        } else {
            target.path = /opt/$${TARGET}/bin
        }
        export(target.path)
    }
}

export(INSTALLS)
