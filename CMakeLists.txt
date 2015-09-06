cmake_minimum_required(VERSION 2.8)
project(sockc)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -DCS_DEBUG=2 -std=c++11 -pthread -O0 -g3 -Wall -ferror-limit=5")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -std=c++11 -pthread -O2 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic -pthread")

include_directories( "../stage/ccpp" )

set(SOURCE_FILES src/main.cpp)
add_executable(sockc ${SOURCE_FILES} 
    src/predef.hpp 
    src/main.cpp 
    src/Portal.hpp 
    src/Config.cpp 
    src/Config.hpp  
    src/Channel.hpp 
    src/Buffer.hpp 
    src/Crypto.hpp
)

link_directories(/usr/lib/x86_64-linux-gnu)
target_link_libraries(sockc 
    libboost_system.so libboost_filesystem.so libboost_program_options.so
    libssl.so libcrypto.so libcrypto++.so
)
