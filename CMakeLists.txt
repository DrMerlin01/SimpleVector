cmake_minimum_required(VERSION 3.10)

project(SimpleVector)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif() 

set(INCLUDE_DIR inc)
set(SOURCE_DIR src)

set(FILES_VECTOR "${INCLUDE_DIR}/array_ptr.h" "${INCLUDE_DIR}/simple_vector.h")
set(FILES_MAIN "${SOURCE_DIR}/main.cpp")

source_group("Vector" FILES ${FILES_VECTOR})
source_group("Source" FILES ${FILES_MAIN}) 

add_executable("simple_vector" ${FILES_MAIN} ${FILES_VECTOR}) 
target_link_libraries("simple_vector" ${SYSTEM_LIBS}) 
