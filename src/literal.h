#ifndef uuid_guard_cc16d513_16aaed40_ebf3f1e7_ec3c3462
#define uuid_guard_cc16d513_16aaed40_ebf3f1e7_ec3c3462

#include <string>

namespace tul{
namespace project{
namespace prdn{

//TODO should be arbitrary length integer, but efficient for integers that fit in 64 bits
struct Literal_integer{
	long data;
};

enum class Str_encoding_t{
	shoco = 0x0,
	alphabetic = 0x1,
	compressed = 0x2,
	//this is just a placeholder, replace with appropriate type
	custom = 0x3,
	//custom encodings:
	auto_nullable = 0x10,
	lrzip,
	lzma,
	rle,
	zpaq
};

struct Literal_string{
	Str_encoding_t encoding;
	std::string data;
};

struct Literal_array{
	int length;
};

}
}
}

#endif // uuid_guard_cc16d513_16aaed40_ebf3f1e7_ec3c3462
