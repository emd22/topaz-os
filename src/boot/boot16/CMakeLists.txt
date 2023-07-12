cmake_minimum_required(VERSION 3.25)

set(BUILD_NAME boot16)


project(${BUILD_NAME} ASM_NASM)

set(CMAKE_ASM_NASM_COMPILER /bin/nasm)
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS asm)

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -f bin -o <OBJECT> <SOURCE>")


file(GLOB_RECURSE ASM_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.asm")

foreach(source ${ASM_SOURCES})
    get_filename_component(BASE_NAME ${source} NAME_WE)

    # CMake's nasm support is garbage
    execute_process(COMMAND ${CMAKE_ASM_NASM_COMPILER} -fbin ${source} -o ${CMAKE_BINARY_DIR}/${BASE_NAME}.bin)
endforeach ()
