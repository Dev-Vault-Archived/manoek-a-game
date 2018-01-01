cmake_minimum_required(VERSION 3.8)
project(Manukan)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES manukan.cpp)

LINK_DIRECTORIES(allegro/lib)
INCLUDE_DIRECTORIES(allegro/include)

add_executable(Manukan ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(Manukan  allegro_acodec
        allegro_audio
        allegro_color
        allegro_dialog
        allegro_image
        allegro_main
        allegro_memfile
        allegro_physfs
        allegro_primitives
        allegro_ttf
        allegro_font
        allegro)

file(GLOB LIBRARIES "allegro/lib/*.dll.a")
message("LIBRARIES = ${LIBRARIES}")

TARGET_LINK_LIBRARIES(Manukan  ${LIBRARIES})