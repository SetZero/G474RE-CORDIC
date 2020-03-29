include(toolchains/stm32f4.txt)

cmake_minimum_required(VERSION 3.7)
project(G474RE-CORDIC ASM C CXX)

add_subdirectory(src)
