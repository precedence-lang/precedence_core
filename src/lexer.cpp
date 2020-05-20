#include "lexer.h"

#include <iostream>
#include <sstream>
#include <stack>

#include <master_h/macro/logger.h>

#include "control.h"
#include "literal.h"
#include "main.h"

namespace tul{
namespace project{
namespace prdn{

enum class State{
	null
};

//does not do any value checking
unsigned char bit_isolate(unsigned char data, int offset, int num){
	unsigned char ret = data << offset;
	//MAGIC char has 8 bits
	return ret >> (8 - num);
}

Lexer::Lexer(){
}

Lexer::Lexer(std::istream& in){
	lex(in);
}

bool Lexer::good() const{
	return good_;
}

std::vector<Token> Lexer::get_tokens() const{
	return tokens;
}

Lexer_output Lexer::get_output() const{
	return output;
}

void Lexer::lex(std::istream& in){
	LOG_DBUG("Entering lexing phase");

	std::stack<State> state;
	state.push(State::null);

	char byte;
	std::stringstream buf;

	std::vector<Control_function> token_func;
	std::vector<Control_operator> token_op;
	std::vector<Control_jump> token_jump;

	std::vector<Literal_integer> token_int;
	std::vector<Literal_string> token_str;
	std::vector<Literal_array> token_array;

	auto get_buf = [this, &buf](){
		auto ret = buf.str();

		buf.str("");
		buf.clear();

		return ret;
	};

	good_ = true;

	while (in.read(&byte, 1) && good_){
		//0 means control
		if (!bit_isolate(byte, 0, 1)){
			LOG_TRAC("Reading control byte(s) sequence");
			//00 means operator
			if (!bit_isolate(byte, 1, 2)){
				Control_operator op;

				int op_id = bit_isolate(byte, 3, 5);
				switch (op_id){
				case 0:
					LOG_TRAC("Reading op add");
					break;
				case 1:
					LOG_TRAC("Reading op increment");
					break;
				case 2:
					LOG_TRAC("Reading op subtract");
					break;
				case 3:
					LOG_TRAC("Reading op decrement");
					break;
				case 4:
					LOG_TRAC("Reading op multiply");
					break;
				case 5:
					LOG_TRAC("Reading op exponentiate");
					break;
				case 6:
					LOG_TRAC("Reading op divide");
					break;
				case 7:
					LOG_TRAC("Reading op modulo");
					break;
				//8-9
				case 10:
					LOG_TRAC("Reading op rotate 2");
					break;
				case 11:
					LOG_TRAC("Reading op rotate 3 up");
					break;
				case 12:
					LOG_TRAC("Reading op bitwise not");
					break;
				case 13:
					LOG_TRAC("Reading op bitwise and");
					break;
				case 14:
					LOG_TRAC("Reading op bitwise or");
					break;
				case 15:
					LOG_TRAC("Reading op bitwise xor");
					break;
				case 16:
					LOG_TRAC("Reading op dupe top");
					break;
				case 17:
					LOG_TRAC("Reading op dupe under");
					break;
				case 18:
					LOG_TRAC("Reading op size");
					break;
				case 19:
					LOG_TRAC("Reading op set");
					break;
				case 20:
					LOG_TRAC("Reading op get");
					break;
				case 21:
					LOG_TRAC("Reading op get and set");
					break;
				case 22:
					LOG_TRAC("Reading op commit");
					break;
				case 23:
					LOG_TRAC("Reading op less than");
					break;
				case 24:
					LOG_TRAC("Reading op less than equal");
					break;
				case 25:
					LOG_TRAC("Reading op equal");
					break;
				case 26:
					LOG_TRAC("Reading op nop");
					break;
				//27-29
				case 30:
					LOG_TRAC("Reading op return");
					break;
				case 31:
					LOG_TRAC("Reading op exit");
					break;
				default:
					LOG_ERRR("Implementation Not Found (INF)");
					good_ = false;
				}
				op.op_id = op_id;
				token_op.push_back(op);

				Token token{Token_t::control_operator, token_op.size() - 1};
				tokens.push_back(token);
			}
			//01 means jump
			else if (bit_isolate(byte, 1, 2) == 1){
				Control_jump jump;
				jump.type = bit_isolate(byte, 3, 1) ? Jump_type_t::comefrom : Jump_type_t::goto_;

				int jump_data = bit_isolate(byte, 4, 4);
				switch (jump_data){
				case 0:
				case 1:
					jump.addr_loc = Jump_addr_loc_t::stack;
					jump.mode = (jump_data == 0) ? Jump_mode_t::relative : Jump_mode_t::absolute;
					LOG_TRAC("Reading {} jump {} addr from stack", (jump.type == Jump_type_t::comefrom) ? "comefrom" : "goto", (jump.mode == Jump_mode_t::relative) ? "relative" : "absolute");
					break;
				case 14:
				case 15:
					jump.addr_loc = Jump_addr_loc_t::code;
					jump.mode = (jump_data == 14) ? Jump_mode_t::relative : Jump_mode_t::absolute;
					LOG_TRAC("Reading {} jump {} addr from code", (jump.type == Jump_type_t::comefrom) ? "comefrom" : "goto", (jump.mode == Jump_mode_t::relative) ? "relative" : "absolute");
					break;
				default:
					jump.mode = Jump_mode_t::relative;
					//jump forward
					if (jump_data < 8){
						//MAGIC arbitrary choices
						int deltas[] = {1, 2, 3, 5, 7, 10};
						jump.delta = deltas[jump_data - 2];
					}
					//jump backward
					else{
						//MAGIC arbitrary choices
						int deltas[] = {-2, -3, -5, -7, -9, -12};
						jump.delta = deltas[jump_data - 8];
					}
					LOG_TRAC("Reading {} jump relative addr delta {}", (jump.type == Jump_type_t::comefrom) ? "comefrom" : "goto", jump.delta);
				}

				token_jump.push_back(jump);

				Token token{Token_t::control_jump, token_jump.size() - 1};
				tokens.push_back(token);
			}
			//TODO add raw reading capability
			//1 means function
			else{
				Control_function func;
				unsigned long data = bit_isolate(byte, 2, 6);
				//60-63 means the function id is at the next 1-4 byte(s)
				if (data >= 60 && data <= 63){
					int byte_count = 64 - data;
					data = 0;

					for (int i = 0; i < byte_count; ++i){
						if (!in.read(&byte, 1)){
							LOG_ERRR("Unexpected end of code input");
							good_ = false;
							break;
						}
						//add 8 bits to data
						data <<= 8;
						data |= bit_isolate(byte, 0, 8);
					}
				}

				func.func_id = data;
				LOG_TRAC("Reading function id {}", func.func_id);
				token_func.push_back(func);

				Token token{Token_t::control_function, token_func.size() - 1};
				tokens.push_back(token);
			}
		}
		//1 means literal
		else{
			LOG_TRAC("Reading literal byte(s) sequence");
			//00 means integer
			if (!bit_isolate(byte, 1, 2)){
				Literal_integer integer;

				bool positive_sign = bit_isolate(byte, 3, 1);
				unsigned long data = bit_isolate(byte, 4, 4);
				//1111 means literal is at next byte(s)
				if (data == (1ul << 4) - 1){
					//read next byte and ignore current byte's last 4 bits as data
					data = 0;

					bool cont = true;
					while (cont){
						if (!in.read(&byte, 1)){
							LOG_ERRR("Unexpected end of code input");
							good_ = false;
							break;
						}
						//add 7 bits to the data
						data <<= 7;
						data |= bit_isolate(byte, 0, 7);
						//extract continue bit
						cont = bit_isolate(byte, 7, 1);
					}
				}

				integer.data = (positive_sign ? 1 : -1) * (long)data;
				LOG_TRAC("Reading integer {}", integer.data);
				token_int.push_back(integer);

				Token token{Token_t::literal_integer, token_int.size() - 1};
				tokens.push_back(token);
			}
			//01 means string
			else if (bit_isolate(byte, 1, 2) == 1){
				Literal_string string;
				std::string data;

				int encoding = bit_isolate(byte, 3, 2);
				unsigned long length = 0;

				//TODO refactor later
				auto read_length = [&length, &in](){
					char byte;
					bool cont = true;
					while (cont){
						if (!in.read(&byte, 1)){
							LOG_ERRR("Unexpected end of code input");
							return false;
						}
						//add 7 bits to the length
						length <<= 7;
						length |= bit_isolate(byte, 0, 7);
						//extract continue bit
						cont = bit_isolate(byte, 7, 1);
					}
					return true;
				};

				auto read_into_buf = [&in, &buf](std::size_t len){
					char byte;
					for (std::size_t i = 0; i < len; ++i){
						//want to read one byte at a time to indicate error later
						//TODO refactor this though
						if (!in.read(&byte, 1)){
							LOG_ERRR("Unexpected end of code input");
							return false;
						}
						buf << byte;
					}
					return true;
				};

				//00 means shoco encoding
				if (encoding == 0){
					//+1 changes length from being in [0, 8) to [1, 9)
					//notably this prevents 1 byte shoco empty strings
					length = bit_isolate(byte, 5, 3) + 1;
					//shifted(111) means length is stored at next byte(s)
					if (length == 8){
						length = 0;
						//note that the read length is NOT shifted by +1
						if (read_length() && read_into_buf(length)){
							data = get_buf();
						}
						else{
							LOG_ERRR("Bad string found");
							good_ = false;
						}
					}
					else{
						if (read_into_buf(length)){
							data = get_buf();
						}
						else{
							LOG_ERRR("Bad string found");
							good_ = false;
						}
					}
				}
				//01 means alphabetic encoding
				else if (encoding == 1){
					LOG_ERRR("Implementation Not Found (INF)");
					good_ = false;
				}
				//10 means compressed encoding
				else if (encoding == 2){
					LOG_ERRR("Implementation Not Found (INF)");
					good_ = false;
				}
				//11 means custom encoding
				else{
					LOG_ERRR("Implementation Not Found (INF)");
					good_ = false;
				}

				string.data = data;
				LOG_TRAC("Reading encoded string {} bytes", string.data.size());
				token_str.push_back(string);

				Token token{Token_t::literal_string, token_str.size() - 1};
				tokens.push_back(token);
			}
			//10 means array
			else if (bit_isolate(byte, 1, 2) == 2){
				Literal_array array;

				unsigned long data = bit_isolate(byte, 3, 5);
				//11111 means length is at next byte(s)
				if (data == (1ul << 5) - 1){
					data = 0;

					bool cont = true;
					while (cont){
						if (!in.read(&byte, 1)){
							LOG_ERRR("Unexpected end of code input");
							good_ = false;
							break;
						}
						data <<= 7;
						data |= bit_isolate(byte, 0, 7);
						cont = bit_isolate(byte, 7, 1);
					}
				}

				array.length = data;
				LOG_TRAC("Reading array length {}", array.length);
				token_array.push_back(array);

				Token token{Token_t::literal_array, token_array.size() - 1};
				tokens.push_back(token);
			}
			//11 means [currently reserved]
			else{
				LOG_ERRR("Byte sequence is currently reserved and invalid");
				good_ = false;
			}
		}
	}

	output.token_func = token_func;
	output.token_op = token_op;
	output.token_jump = token_jump;

	output.token_int = token_int;
	output.token_str = token_str;
	output.token_array = token_array;

	//all extra environments should be escaped by now
	if (state.top() != State::null){
		good_ = false;
	}
	LOG_INFO("Lexing phase {}", good_ ? "succeeded" : "FAILED");
}

}
}
}
