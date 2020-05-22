#include "parser.h"

#include <master_h/macro/logger.h>

#include <apricot/shoco.h>
#include <apricot/zpaq.h>

#include "main.h"

namespace tul{
namespace project{
namespace prdn{

bool Parser::parse_lexer_output(const Lexer& lexer){
	LOG_DBUG("Entering parsing phase");

	Lexer_output lexer_vars = lexer.get_output();
	std::vector<Token> tokens = lexer.get_tokens();

	//this tracks which arrays we nest
	std::stack<std::size_t> array_stack;
	good_ = true && lexer.good();

	for (const Token& token: tokens){
		Code_point cp;

		switch (token.type){
		case Token_t::control_function:
			LOG_TRAC("Seeing function");
			cp.type = Code_point_t::function;
			cp.func_op_id = lexer_vars.token_func[token.index].func_id;
			break;
		case Token_t::control_operator:
			LOG_TRAC("Seeing operator");
			cp.type = Code_point_t::operator_;
			cp.func_op_id = lexer_vars.token_op[token.index].op_id;
			break;
		case Token_t::control_jump:
			LOG_TRAC("Seeing jump");
			cp.type = Code_point_t::jump;
			cp.jump = lexer_vars.token_jump[token.index];
			break;
		case Token_t::literal_integer:
			LOG_TRAC("Seeing integer");
			cp.type = Code_point_t::literal_push;

			var_stack.value_integer.insert(
				std::make_pair(var_stack.nonce, lexer_vars.token_int[token.index].data)
			);
			++var_stack.nonce;

			cp.value = Value{Type_t::integer, var_stack.nonce - 1};
			break;
		case Token_t::literal_string:
			{
			LOG_TRAC("Seeing string");
			cp.type = Code_point_t::literal_push;

			Literal_string& encoded = lexer_vars.token_str[token.index];
			std::string decoded;

			switch (encoded.encoding){
			case Str_encoding_t::shoco:
				try{
					decoded = apricot::Shoco().decompress(encoded.data);
				}
				catch (...){
					LOG_ERRR("Unable to decode string");
				}
				break;
			case Str_encoding_t::zpaq:
				//NOTE there is no error checking for this decoder. will fail silently
				decoded = apricot::Zpaq().decompress(encoded.data);
				break;
			default:
				LOG_ERRR("Implementation Not Found (INF)");
			}

			var_stack.value_string.insert(
				std::make_pair(var_stack.nonce, decoded)
			);
			++var_stack.nonce;

			cp.value = Value{Type_t::string, var_stack.nonce - 1};
			}
			break;
		case Token_t::literal_array:
			{
			LOG_TRAC("Seeing array");
			cp.type = Code_point_t::literal_push;

			Array array;
			array.size = lexer_vars.token_array[token.index].length;
			LOG_TRAC("Array has length {}", array.size);

			var_stack.value_array.insert(
				std::make_pair(var_stack.nonce, array)
			);
			++var_stack.nonce;

			cp.value = Value{Type_t::array, var_stack.nonce - 1};
			}
			break;
		default:
			LOG_ERRR("Hit bad location");
			good_ = false;
		}

		//add token to array
		if (array_stack.size() && var_stack.value_array[array_stack.top()].size){
			if (token.type == Token_t::literal_integer || token.type == Token_t::literal_string || token.type == Token_t::literal_array){
				Array& array = var_stack.value_array[array_stack.top()];

				//note that the array cannot be at capacity yet due the check later at the end of this loop
				//i.e. array.size will not underflow when decremented
				array.elements.push_back(cp.value);
				LOG_TRAC("Array value inserted at array stack level {}", array_stack.size() - 1);
				--array.size;

				//add current array to the stack of arrays to be resolved
				if (token.type == Token_t::literal_array){
					array_stack.push(var_stack.nonce - 1);
				}
			}
			else{
				LOG_ERRR("Array can only accept literals as elements");
				good_ = false;
			}
		}
		//add code point to code points
		else{
			cps.push_back(cp);

			//"first" array on the array stack
			//can be refactored out of the if/else, but this is more readable
			if (token.type == Token_t::literal_array){
				array_stack.push(var_stack.nonce - 1);
			}
		}

		//when the top array on the array stack is finished
		while (good_ && array_stack.size()
				&& var_stack.value_array[array_stack.top()].size == 0){
			array_stack.pop();
			LOG_TRAC("Array has been finished with {} left", array_stack.size());
		}
	}

	good_ &= array_stack.empty();

	LOG_INFO("Parsing phase {}", good_ ? "succeeded" : "FAILED");
}

std::vector<Code_point> Parser::get_cps() const{
	return cps;
}

Var_stack Parser::get_var_stack() const{
	return var_stack;
}

bool Parser::good() const{
	return good_;
}

}
}
}
