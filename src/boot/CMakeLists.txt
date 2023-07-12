cmake_minimum_required(VERSION 3.25)

set(BUILD_NAME boot32)
project(${BUILD_NAME} C ASM)


add_subdirectory(boot16)

set(CMAKE_CXX_COMPILER gcc)
#set(CMAKE_ASM_COMPILER as)
set(CMAKE_LINKER ld)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(C_WARNING_FLAGS "")
set(CMAKE_C_FLAGS "-Os -m32 -fno-pie -ffreestanding -nostdlib -nostdinc -fno-stack-protector -nostartfiles ${C_WARNING_FLAGS}")
#set(CMAKE_ASM_FLAGS "--32")
set(LINKER_FLAGS "-Wl,--oformat=binary,-T${CMAKE_CURRENT_LIST_DIR}/linker.ld,-melf_i386")

file(GLOB_RECURSE ASM_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.S")
file(GLOB_RECURSE SOURCES  "${CMAKE_CURRENT_LIST_DIR}/*.c")
file(GLOB_RECURSE HEADERS "../*.h" "./*.h")

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../
)

add_executable(${BUILD_NAME} ${ASM_SOURCES} ${SOURCES} ${HEADERS})
set_target_properties(${BUILD_NAME} PROPERTIES COMPILE_FLAGS ${CMAKE_C_FLAGS} LINK_FLAGS ${LINKER_FLAGS})

set(DISK_IMAGE_PATH ${CMAKE_BINARY_DIR}/../topaz.img)


