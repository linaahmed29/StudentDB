cmake_minimum_required(VERSION 3.16)
project(StudentDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(StudentDB main.cpp)
target_link_libraries(StudentDB Qt6::Widgets Qt6::Sql)