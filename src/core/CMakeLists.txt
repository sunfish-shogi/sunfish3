cmake_minimum_required(VERSION 2.8)

project(sunfish CXX)

add_library(core STATIC
	base/Piece.cpp
	base/Position.cpp
	board/Bitboard.cpp
	board/Board.cpp
	board/Zobrist.cpp
	dev/CodeGenerator.cpp
	dev/MoveGenChecker.cpp
	dev/MoveGeneratorExpr.cpp
	move/Move.cpp
	move/MoveGenerator.cpp
	move/MoveTable.cpp
	record/CsaReader.cpp
	record/CsaWriter.cpp
	record/Record.cpp
	util/Data.cpp
	util/FileList.cpp
	util/Wildcard.cpp
)
