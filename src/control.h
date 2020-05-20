#ifndef uuid_guard_c62d51a6_b186af38_182d4f15_f089c30d
#define uuid_guard_c62d51a6_b186af38_182d4f15_f089c30d

namespace tul{
namespace project{
namespace prdn{

struct Control_function{
	int func_id;
};

struct Control_operator{
	int op_id;
};

//type of jump
enum class Jump_type_t{
	goto_,
	comefrom
};

//where the jump address is located
enum class Jump_addr_loc_t{
	//runtime
	stack,
	//interpret time
	code,
	//interpret time
	implicit
};

//how the jump works
enum class Jump_mode_t{
	relative,
	absolute
};

struct Control_jump{
	Jump_type_t type;
	Jump_addr_loc_t addr_loc;
	Jump_mode_t mode;

	//only used for addr_loc = implicit
	int delta;
};

}
}
}

#endif // uuid_guard_c62d51a6_b186af38_182d4f15_f089c30d
