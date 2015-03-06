/* problem.cpp
 * 
 * Kubo Ryosuke
 */

#include "logger/Logger.h"
#include "solve/Solver.h"
#include "console/ConsoleManager.h"
#include <vector>
#include <fstream>

using namespace sunfish;

int solve(const std::vector<std::string>& problems, const ConsoleManager::Config& config) {

	// logger settings
	std::ofstream fout("problem.log", std::ios::out);
	if (fout) {
  	Loggers::message.addStream(fout);
	}
	Loggers::error.addStream(std::cerr, "\x1b[31m", "\x1b[39m");
	Loggers::warning.addStream(std::cerr, "\x1b[33m", "\x1b[39m");
	Loggers::message.addStream(std::cerr);
	Loggers::send.addStream(std::cerr, true, true, "\x1b[34m", "\x1b[39m");
	Loggers::receive.addStream(std::cerr, true, true, "\x1b[35m", "\x1b[39m");
#ifndef NDEBUG
	Loggers::debug.addStream(std::cerr, "\x1b[36m", "\x1b[39m");
	Loggers::test.addStream(std::cerr, "\x1b[32m", "\x1b[39m");
	Loggers::develop.addStream(std::cerr, "\x1b[37m", "\x1b[39m");
#endif

	Solver solver;
	solver.setProblems(problems);
	solver.setConfig(config);
	solver.solve();

	return solver.hasError() ? 1 : 0;

}
