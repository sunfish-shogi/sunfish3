cmake_minimum_required(VERSION 2.8)

project(sunfish CXX)

add_library(book STATIC
	Book.cpp
	BookGenerator.cpp
)
