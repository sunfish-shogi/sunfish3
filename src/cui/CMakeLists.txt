cmake_minimum_required(VERSION 2.8)

project(sunfish CXX)

add_library(cui STATIC
	console/ConsoleManager.cpp
	solve/Solver.cpp
	book.cpp
	CMakeLists.txt
	dev.cpp
	learn.cpp
	network.cpp
	problem.cpp
	profile.cpp
	sunfish.cpp
	test.cpp
)
