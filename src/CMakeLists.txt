cmake_minimum_required(VERSION 3.28)
project(library_seat_manager CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Seat)

target_sources(Seat PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${CMAKE_SOURCE_DIR}/modules
	FILES modules/seat.cxx
)


add_executable(library_seat_manager main.cxx)
target_link_libraries(library_seat_manager Seat)
