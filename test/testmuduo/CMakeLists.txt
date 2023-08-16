#配置头文件搜索路径
#include_directories()

#配置库文件搜索路径
#link_directories()

#设置需要编译的源文件列表
set(SRC_LIST ./muduo_server.cpp)

#设置可执行文件，最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)#PROJECT_SOURCE_DIR是工程的根目录，这里就是testmuduo

#把.指定路径下的所有源文件名，放入变量SRC_LIST中（解决有很多文件名的问题）
#aux_source_directory(. SRC_LIST)

#生成可执行文件(生成可执行文件server，由SRC_LIST定义的源文件编译而来)
add_executable(server ${SRC_LIST})#后面变量里面，放的就是定义的源文件列表

#可执行文件需要链接的库(表示server这个目标文件，需要链接这三个库)
target_link_libraries(server muduo_net muduo_base pthread)



