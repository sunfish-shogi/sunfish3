/* Solver.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SOLVER__
#define __SUNFISH_SOLVER__

#include "searcher/Searcher.h"
#include "core/record/Record.h"
#include <vector>
#include <string>

namespace sunfish {

	class Solver {
	public:

		enum class ErrorCode : unsigned {
			FILE_READ_ERROR,
			INVALID_RECORD_LENGTH,
		};

		struct Error {
			std::string filename;
			ErrorCode errorCode;
		};

		struct Summary {
			int correct;
			int incorrect;
			int total;
			int ignore;
			Summary() : correct(0), incorrect(0), total(0), ignore(0) {}
		};

	private:

		Searcher _searcher;

		Record _record;

		std::vector<std::string> _problems;

		std::vector<Error> _errors;

		Summary _summary;

		void solve(const std::string& problem);

		void solve(const Board& board, const Move& correct);

	public:

		void clear() {
			_problems.clear();
			_problems.shrink_to_fit();
			_errors.clear();
			_errors.shrink_to_fit();
			_summary = Summary();
		}

		void setProblems(const std::vector<std::string>& problems);

		void solve();

		bool hasError() const {
			return !_errors.empty();
		}

		const std::vector<Error>& getErrors() const {
			return _errors;
		}

		const Summary& getSummary() const {
			return _summary;
		}

	};

}

#endif // __SUNFISH_SOLVER__
