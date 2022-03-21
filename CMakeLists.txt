cmake_minimum_required(VERSION 3.0)		#指定cmake版本 
#设置项目名称
SET(PROJECT_NAME phpface)

project(${PROJECT_NAME})				#指定项目的名称，一般和项目的文件夹名称对应

set(CMAKE_CXX_STANDARD 11)				#设置c++标准
#寻找OpenCV库
FIND_PACKAGE(OpenCV REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
#打印调试信息
MESSAGE(STATUS "Project: ${PROJECT_NAME}")
MESSAGE(STATUS "OpenCV library status:")
MESSAGE(STATUS "    version: ${OpenCV_VERSION}")
MESSAGE(STATUS "    libraries: ${OpenCV_LIBS}")
MESSAGE(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
#获取代码，在项目中，将所有代码都放在src文件夹中
AUX_SOURCE_DIRECTORY(${PROJECT_NAME} DIR_SRCS)

#指定头文件目录
include_directories(./)	
include_directories(./inc)

#指定静态和动态文件目录
link_directories(./linux_so)

#该指令的主要作用就是将指定的源文件生成链接文件，然后添加到工程中去。
#arcsoft_face生成的库文件名		[STATIC | SHARED | MODULE] 库类型
add_executable(${PROJECT_NAME} 
        ./inc/amcomdef.h
        ./inc/arcsoft_face_sdk.h
		./inc/asvloffscreen.h
		./inc/merror.h
		./demo.cpp)


#-fPIC	在给定的作用域内设置一个命名的属性
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)


#链接库文件
target_link_libraries(${PROJECT_NAME}	
		arcsoft_face
		arcsoft_face_engine
		${OpenCV_LIBS}
        )