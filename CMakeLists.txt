cmake_minimum_required(VERSION 2.8)

set(PROJECT RoboMobile)
project(${PROJECT})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -std=c++11 -Wall -g")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

find_package(OpenCV)

set(LIBS ${LIBS}
	pthread
)

aux_source_directory(sources SOURCES)
set(SOURCES ${SOURCES}
	${PROJECT}.cpp
)

include_directories(
	sources
	${OpenCV_INCLUDES}
)

add_executable(${PROJECT} ${SOURCES})

target_link_libraries(${PROJECT}
	${LIBS}
	${OpenCV_LIBS} 
)
