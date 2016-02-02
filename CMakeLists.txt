cmake_minimum_required(VERSION 2.8)

set(PROJECT robotractor)
project(${PROJECT})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
# 	add_definitions(-g)

# 	add_definitions("-Wall")
# 	add_definitions("-Wextra")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

find_package(OpenCV)

set(LIBS ${LIBS}
	pthread
)

aux_source_directory(sources/osal SOURCES)
aux_source_directory(sources SOURCES)
set(SOURCES ${SOURCES}
	${PROJECT}.cpp
)

include_directories(
	sources
	${OpenCV_INCLUDES}
)

# use, i.e. don't skip the full RPATH for the build tree
# SET(CMAKE_SKIP_BUILD_RPATH TRUE)

add_executable(${PROJECT} ${SOURCES})

target_link_libraries(${PROJECT}
	${LIBS}
	${OpenCV_LIBS} 
)
