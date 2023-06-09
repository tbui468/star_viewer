cmake_minimum_required(VERSION 3.7)


project(test)


set (SDL2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/SDL3/include)
set (SDL2_LIB_DIR ${CMAKE_SOURCE_DIR}/SDL3/build/Release)
#set (SDL2_LIB_DIR ${CMAKE_SOURCE_DIR}/SDL/lib/x64)

set(
    Sources
    main.c
    )

set(
    Headers
    )

add_executable(
                test
                ${Sources}
                ${Headers}
              )
target_include_directories(test PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(test ${SDL2_LIB_DIR}/SDL3.lib)
find_library(${SDL2_LIB_DIR} SDL3.dll)
#target_link_libraries(test ${SDL2_LIB_DIR}/SDL2main.lib ${SDL2_LIB_DIR}/SDL2.lib)

if(WIN32)
    target_link_libraries(test wsock32 ws2_32) 
endif()
#if(MSVC)
#  target_compile_options(test PRIVATE /W4 /WX)
#else()
#  target_compile_options(test PRIVATE -Wall -Wextra -Wpedantic -Werror)
#endif()

