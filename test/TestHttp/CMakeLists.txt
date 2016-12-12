# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. LIST_SRCS)

# 设置头文件
include_directories(../../inc)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${BIN_OUTPUT_PATH})
# 设置链接的库
set(LINK_LIBS ${BIN_OUTPUT_PATH}/libBaseNetwork.so rt dl)
# 设置链接选项
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath=./ ")
# 生成gate
add_executable(TestHttp ${LIST_SRCS})

# 链接库
target_link_libraries(TestHttp ${LINK_LIBS})