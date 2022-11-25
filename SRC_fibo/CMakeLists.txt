cmake_minimum_required(VERSION 3.13)

set(SOURCE_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BINARY_TOP_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_C_COMPILER_FORCED   1)
set(CMAKE_CXX_COMPILER_FORCED 1)

set(out_hex_dir ${CMAKE_CURRENT_BINARY_DIR}/hex)
set(out_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(out_rel_dir ${CMAKE_CURRENT_BINARY_DIR}/rel)
set(out_inc_dir ${CMAKE_CURRENT_BINARY_DIR}/include)
set(tools_dir ${CMAKE_CURRENT_SOURCE_DIR}/tools)

include_directories(inc)

project(Q161_LC610N)
set(target _Q161_LC610N)

file(GLOB APP_SOURCES_FILE ${CMAKE_SOURCE_DIR}/vanstone_app/src/*.c)

add_library(${target} STATIC
   ${APP_SOURCES_FILE} 
)

set(LIBRARY_OUTPUT_PATH ${out_lib_dir})

set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${out_lib_dir})

set(dbg_lib_file ${out_lib_dir}/lib${target}.a)
set(realse_lib_file ${SOURCE_TOP_DIR}/../lib)

#add 3rd lib and auto copy
#file(COPY ${dbg_lib_file} DESTINATION ${realse_lib_file} FOLLOW_SYMLINK_CHAIN)

#set(origin_lib ${dbg_lib_file})
#set(destination_lib ${realse_lib_file})
#add_custom_command(TARGET ${target}
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#    ${origin_lib} ${destination_lib}
#)

target_include_directories(${target} PUBLIC inc)
