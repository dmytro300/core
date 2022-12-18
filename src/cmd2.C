#include "admin.H"
#include <unordered_map>

struct UserCmd : public BaseCmd{

	typedef UserCmd self;

	FLDN(IntFld,sz,false)
	FLDN(FlgFld,eval,true)
	FLDN(StrFld,sd,true)

	static constexpr BaseCmd::pvec init()
	{
		return { &self::szF, &self::evalF, &self::sdF };
	}
};

void cmdF(UserCmd& cmd)
{
	std::cout
			<< " Execution:"
			<< " sz="  << cmd.sz.v
			<< " ev="  << cmd.eval.v
			<< " sd="  << cmd.sd.v
			<< '\n'  << std::endl;
}
	
int main(int argc, const char * argv[]) {

	if(argc != 2)
	{
		std::cout  << "examlpe: cmd2.bin \"tail 111 --eval on --sd sdstring\"" <<  '\n'  << std::endl;
		return 1;
	}
	
	AdminMgr adminMgr;

	adminMgr.reg<UserCmd>("tail", cmdF);

	adminMgr.run(argv[1]);

	return 0;

}
