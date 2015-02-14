/* Solver.cpp
 * 
 * Kubo Ryosuke
 */

#include "Solver.h"
#include "logger/Logger.h"
#include "core/record/CsaReader.h"

namespace sunfish {

	void Solver::setProblems(const std::vector<std::string>& problems) {
		_problems.clear();
		_problems.reserve(problems.size());
		_problems = problems;
	}

	void Solver::solve() {
		auto searchConfig = _searcher.getConfig();
		searchConfig.maxDepth = 5;
		searchConfig.limitSeconds = 5;
		searchConfig.treeSize = 1;
		searchConfig.wokerSize = 1;
		_searcher.setConfig(searchConfig);

		_searcher.init();

		for (const auto& problem : _problems) {
			solve(problem);
		}

		double correctRatio = (double)_summary.correct / _summary.total;

		Loggers::message << "Complete!!";
		Loggers::message << "";
		Loggers::message << "Summary:";
		Loggers::message << "  correct   : " << _summary.correct;
		Loggers::message << "  correct(%): " << correctRatio * 100.0;
		Loggers::message << "  incorrect : " << _summary.incorrect;
		Loggers::message << "  total     : " << _summary.total;
		Loggers::message << "  ignore    : " << _summary.ignore;
	}

	void Solver::solve(const std::string& problem) {

		Loggers::message << "[" << problem << "]";

		// 棋譜を読み込む
		if (!CsaReader::read(problem, _record)) {
			_errors.push_back({ problem, ErrorCode::FILE_READ_ERROR });
			_summary.ignore++;
			Loggers::message << "read error: [" << problem << "]";
			return;
		}

		// 不正な手数
		if (_record.getTotalCount() == 0) {
			_errors.push_back({ problem, ErrorCode::INVALID_RECORD_LENGTH });
			_summary.ignore++;
			Loggers::message << "invalid record: [" << problem << "]";
			return;
		}

		while (_record.unmakeMove())
			;

		while (_record.makeMove()) {
			Move correct = _record.getNextMove();
			if (correct.isEmpty()) {
				break;
			}
  		solve(_record.getBoard(), correct);
		}

	}

	void Solver::solve(const Board& board, const Move& correct) {

		Loggers::message << board.toStringCsa();

		Move answer;
		bool ok = _searcher.idsearch(board, answer);

		bool result;
		if (ok && answer == correct) {
			_summary.correct++;
			result = true;
		} else {
			_summary.incorrect++;
			result = false;
		}
		_summary.total++;

		bool black = board.isBlack();
		Loggers::message << "correct: " << correct.toStringCsa(black);
		Loggers::message << "answer : " << (ok ? answer.toStringCsa(black) : "Resign");
		Loggers::message << "result : " << (result ? "Good" : "Bad");
		Loggers::message << "";

	}    

}
