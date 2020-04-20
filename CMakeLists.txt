cmake_minimum_required(VERSION 2.6)
project(capstone_server)

add_executable(capstone_server main.cpp)
TARGET_LINK_LIBRARIES(capstone_server capstone pthread)
install(TARGETS capstone_server RUNTIME DESTINATION bin)
set(CMAKE_CXX_STANDARD 11) ## or 14 !!
set(CMAKE_CXX_EXTENSIONS OFF) ## on g++ this ensures: -std=c++11 and not -std=gnu++11
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -frecord-gcc-switches")
