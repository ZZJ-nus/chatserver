cmake_minimum_required(VERSION 3.0)#要求的版本
project(main)#工程名字是main

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)#从cmake使用中直接复制过来
#-g代表可进行调试，如果没有就是可执行文件，但不能调试

#指定搜索的子目录
#add_subdirectory(testmuduo)

#配置最终可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set(EXECUTABLE_OUTPUT_PATH /home/zzj/code/bin)

#配置头文件的搜索路径
# include_directories(${PROJECT_SOURCE_DIR}/include)
# include_directories(${PROJECT_SOURCE_DIR}/include/server)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)

#include_directories(/home/zzj/code/include)
#message(STATUS "lkmao:PROJECT_SOURCE_DIR -- ${PROJECT_SOURCE_DIR}")
#include/server/chatserver.hpp
#加载子目录
add_subdirectory(src) 