cmake_minimum_required(VERSION 2.8)

project(sunfish CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
include_directories(".")

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Clang|GNU|Intel)")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Wall -W -msse2 -fno-rtti -pthread")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG -fno-exceptions")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
	set(PROFILE_FLAGS "-pg")
	set(PROFILE_GENERATE_FLAGS "-fprofile-generate")
	set(PROFILE_USE_FLAGS "-fprofile-use")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX- /nologo /D WIN32 /D _CONSOLE /Oy- /Gm- /Gd /GS- /MT /fp:precise /arch:SSE2")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Ox /Ot /D NDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi /Od /D")
	find_library(WSOCK32_LIBRARY wsock32)
endif()

if("${BMI2}" MATCHES "(1|ON)")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BMI2=1 -mbmi2")
endif()

if("${PROFILE}" MATCHES "(1|ON)")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PROFILE_FLAGS}")
endif()

if("${PROFILE_GENERATE}" MATCHES "(1|ON)")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PROFILE_GENERATE_FLAGS}")
endif()

if("${PROFILE_USE}" MATCHES "(1|ON)")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PROFILE_USE_FLAGS}")
endif()

if(NOT ("${LEARNING}" MATCHES "(1|ON)"))
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNLEARN")
endif()

add_subdirectory(cui)
add_subdirectory(learning)
add_subdirectory(network)
add_subdirectory(searcher)
add_subdirectory(book)
add_subdirectory(config)
add_subdirectory(core)
add_subdirectory(logger)

add_executable(sunfish
	test/core/BitboardTest.cpp
	test/core/BoardTest.cpp
	test/core/CsaReaderTest.cpp
	test/core/HandTest.cpp
	test/core/MoveGeneratorTest.cpp
	test/core/MovesTest.cpp
	test/core/MoveTest.cpp
	test/core/PieceTest.cpp
	test/core/SquareTest.cpp
	test/core/WildcardTest.cpp
	test/core/ZobristTest.cpp
	test/network/CsaClientTest.cpp
	test/searcher/EvaluateEntityTest.cpp
	test/searcher/EvaluateTableTest.cpp
	test/searcher/EvaluatorTest.cpp
	test/searcher/MateTest.cpp
	test/searcher/SearcherTest.cpp
	test/searcher/SeeTest.cpp
	test/searcher/ShekTest.cpp
	test/searcher/TreeTest.cpp
)

target_link_libraries(sunfish cui)
target_link_libraries(sunfish learning)
target_link_libraries(sunfish network)
target_link_libraries(sunfish searcher)
target_link_libraries(sunfish book)
target_link_libraries(sunfish config)
target_link_libraries(sunfish core)
target_link_libraries(sunfish logger)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_link_libraries(sunfish wsock32)
endif()
