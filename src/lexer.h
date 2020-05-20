#ifndef uuid_guard_a41dc52d_4e477626_fac62e42_ea88dbd7
#define uuid_guard_a41dc52d_4e477626_fac62e42_ea88dbd7

#include <iosfwd>
#include <vector>

#include "control.h"
#include "literal.h"
#include "token.h"

namespace tul{
namespace project{
namespace prdn{

//TODO figure out how to refactor this
struct Lexer_output{
	std::vector<Control_function> token_func;
	std::vector<Control_operator> token_op;
	std::vector<Control_jump> token_jump;

	std::vector<Literal_integer> token_int;
	std::vector<Literal_string> token_str;
	std::vector<Literal_array> token_array;
};

class Lexer{
public:
	Lexer();
	Lexer(std::istream& in);

	bool good() const;
	std::vector<Token> get_tokens() const;
	Lexer_output get_output() const;
private:
	void lex(std::istream& in);

	bool good_ = false;
	std::vector<Token> tokens;
	Lexer_output output;
};

}
}
}

#endif // uuid_guard_a41dc52d_4e477626_fac62e42_ea88dbd7
