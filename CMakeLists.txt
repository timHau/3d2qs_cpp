cmake_minimum_required(VERSION 3.17)
project(3d2qs_sunc_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

add_executable(3d2qs_sunc_cpp
        src/main.cpp
        include/OBJ_Loader.h
        include/boost/filesystem.hpp
        src/Object.h src/Object.cpp include/cpptoml.h src/Object.cpp src/Object.h src/SUNCTransformer.cpp src/SUNCTransformer.h src/Utils.h)

