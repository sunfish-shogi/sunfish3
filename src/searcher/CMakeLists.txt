cmake_minimum_required(VERSION 2.8)

project(sunfish CXX)

add_library(searcher STATIC
	eval/Evaluator.cpp
	eval/Material.cpp
	mate/Mate.cpp
	progress/Progression.cpp
	see/See.cpp
	time/TimeManager.cpp
	tree/Tree.cpp
	tree/Worker.cpp
	tt/TTE.cpp
	RandomSearcher.cpp
	Searcher.cpp
)
