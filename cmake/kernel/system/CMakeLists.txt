cmake_minimum_required(VERSION 3.14)

target_sources(kernel PUBLIC
        ${CHURLOS_SRC_DIR}/kernel/system/Globals.cc
        ${CHURLOS_SRC_DIR}/kernel/system/System.cpp
        )
