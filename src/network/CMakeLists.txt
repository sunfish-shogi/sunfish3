cmake_minimum_required(VERSION 2.8)

project(sunfish CXX)

add_library(network STATIC
	Connection.cpp
	CsaClient.cpp
)
