cmake_minimum_required(VERSION 3.17)
project(isolate C)

set(CMAKE_C_STANDARD 11)

add_executable(isolate
        main.c
        parent.c parent.h
        child.c child.h
        util.c util.h
        )
