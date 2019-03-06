cmake_minimum_required(VERSION 2.6)
project(capstone_server)

add_executable(capstone_server main.cpp)
TARGET_LINK_LIBRARIES(capstone_server capstone pthread)
install(TARGETS capstone_server RUNTIME DESTINATION bin)
