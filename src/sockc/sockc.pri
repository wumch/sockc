
INCLUDEPATH += $$_PROFILE_PWD_/../stage/ccpp
DEPENDPATH += $$_PRO_FILE_PWD_/../stage/ccpp

LIBS += -lboost_system -lboost_filesystem -lboost_program_options.so \
    -lssl -lcrypto++ -lcrypto

SOURCES += main.cpp Config.cpp Buffer.cpp
HEADERS += Buffer.hpp  Bus.hpp  Channel.hpp  Config.hpp  Crypto.hpp  Pool.hpp  Portal.hpp  predef.hpp  Traits.hpp

