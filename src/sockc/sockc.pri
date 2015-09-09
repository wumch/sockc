
INCLUDEPATH += $$_PRO_FILE_PWD_/../stage/ccpp
DEPENDPATH += $$_PRO_FILE_PWD_/../stage/ccpp

LIBS += -lboost_system -lboost_filesystem -lboost_program_options -lboost_thread \
    -lssl -lcrypto++ -lcrypto

SOURCES += Config.cpp Buffer.cpp
HEADERS += Buffer.hpp  Bus.hpp  Channel.hpp  Config.hpp  Crypto.hpp  Pool.hpp  Portal.hpp  predef.hpp  Traits.hpp
