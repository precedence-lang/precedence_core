#ifndef uuid_guard_8c75d2ac_9dc33fe1_5551b926_d4d2dbf3
#define uuid_guard_8c75d2ac_9dc33fe1_5551b926_d4d2dbf3

#include <vector>

#include "code_point.h"
#include "lexer.h"
#include "variable_stack.h"

namespace tul{
namespace project{
namespace prdn{

class Parser{
public:
	bool parse_lexer_output(const Lexer& lexer);

	bool good() const;
	std::vector<Code_point> get_cps() const;
	Var_stack get_var_stack() const;
private:
	bool good_ = false;
	std::vector<Code_point> cps;
	Var_stack var_stack;
};

}
}
}

#endif // uuid_guard_8c75d2ac_9dc33fe1_5551b926_d4d2dbf3
