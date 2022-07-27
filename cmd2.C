#include <vector>
#include <array>
#include <map>
#include <string>
#include <tuple>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "warm.H"

Context context;

struct strref
{
	size_t len;
	const char* str;
	strref(const std::string& s_) : len(s_.length()), str(s_.data()) {}
	constexpr strref(const char* s_, size_t len_) : len(len_), str(s_) {}
	constexpr strref() : len(0), str(nullptr) {}

	template<size_t N>
	constexpr strref(const char (&s)[N]):strref(s,N-1){};

	template<size_t N>
	constexpr strref(char (&s)[N]) = delete;

	const std::string as_string() { return std::string(str,len); }

	constexpr bool operator == (const strref& s_) const
	{
		if(len != s_.len)
			return false;
		return ::memcmp(str,s_.str,s_.len) == 0;
	}

	constexpr bool operator != (const strref& s_) const
	{
		return !(*this == s_);
	}

	template<size_t N>
	constexpr bool operator == (const char (&s)[N]) const
	{ 
		return *this == strref(s);
	}
};

template<size_t S>
std::ostream &operator<<(std::ostream &os, const strref &m) { 
	return os << m.str;
}

template<size_t S>
struct cstr
{
	uint8_t len;
	char str[S+1];

	constexpr cstr()
	: len(0)
	{
	  str[0]=0;	
	}

	constexpr cstr(const char* s, size_t sz)
	: len(sz > S ? S : sz)
	{
		::memcpy(str,s,len);
	  str[len]=0;	
	}

	template<size_t N>
	constexpr cstr(const char (&s)[N]):cstr(s,N-1)
	{}

  cstr(cstr const&) = default;

	constexpr strref sref() const
	{
		return strref(str,len);	
	}
	
	bool operator < (const cstr& rhs_) const
	{
		return ::memcmp(str, rhs_.str, len) < 0;
	}

	bool operator == (const strref& s_) const
	{
		if(len != s_.len)
			return false;
		return ::memcmp(str,s_.str,len) == 0;
	}

	bool operator == (const std::string& s_) const
	{
		if(len != s_.length())
			return false;
		return ::memcmp(str,s_.c_str(),len) == 0;
	}
/*
	template<size_t N>
	cstr& operator = (const char (&s_)[N])
	{
		len = N > S ? S : N;	
		::memcpy(v,s_,len);
	}
*/
};

template<size_t S>
std::ostream &operator<<(std::ostream &os, cstr<S> const &m) { 
	return os << m.str;
}

struct BaseFld{
	virtual bool assign(const strref& v_)=0;
};

struct IntFld : public BaseFld{
	long long v = 0;
	IntFld(int v_) : v(v_) {}
	IntFld(){}
	virtual bool assign(const strref& v_)
	{
		v = atoll(v_.str);
		return v || v_ == "0";
	}
};

struct FltFld : public BaseFld{
	double v = 0;
	FltFld(double v_) : v(v_) {}
	FltFld(){}
	virtual bool assign(const strref& v_)
	{
		v = atof(v_.str);
		return v!=0.0 || v_ == "0" || v_ == "0.0";
	}
};


struct StrFld : public BaseFld{
	std::string v;
	StrFld(const std::string& v_) : v(v_) {}
	StrFld(){}
	virtual bool assign(const strref& v_)
	{
		v.assign(v_.str,v_.len);
		return true;
	}
};


struct FlgFld : public BaseFld{
	bool v = false;
	FlgFld(bool v_) : v(v_) {}
	FlgFld(){}
	virtual bool assign(const strref& v_)
	{
		bool v1 = v_ == "on" || v_ == "1" || v_ == "true";	
		bool p1 = v_ == "off"|| v_ == "0" || v_ == "false";	
		v = v1;
		return v1 || p1;
	}
};


#define GET_PARENT(identifier) (&std::remove_pointer_t<decltype(this)>::identifier)

#define FLDD(C,T,N,D,P) T N{D}; static constexpr BaseCmd::pfield N##F(BaseCmd* p) { return {#N,P,static_cast<C*>(p)->N};}
#define FLDN(C,T,N,P)      T N; static constexpr BaseCmd::pfield N##F(BaseCmd* p) { return {#N,P,static_cast<C*>(p)->N};}
/*
	IntFld sz;
	static BaseCmd::pfield szF(BaseCmd* p)
	{
		return BaseCmd::pfield("sz",true,static_cast<UserCmd*>(p)->sz);
	}
*/


struct BaseCmd{

	typedef cstr<4> FldName;

	struct FldInfo{
		FldName  name;
		bool   is_param;
		BaseFld&  fld;

		bool operator < (const FldInfo& rhs_) const
		{
			return name < rhs_.name;
		}
	};

	typedef BaseCmd::FldInfo pfield;
  typedef std::map<FldName,pfield*> FldInfoMap;
	typedef BaseCmd::pfield (*FldFunc)(BaseCmd *);
	
	template<size_t M> 
	using pvec = std::array<FldFunc,M>; 
};

struct token
{
	char  t='\0';
	const char* p1=0;
	const char* p2=0;

	strref sref() const
	{
		return strref(p1,size());
	}

	size_t size() const
	{
		return p2 - p1;
	}

	strref param() const
	{
		return ( size() > 1 ) ?  strref(p1+2,size()-2) : strref();
	}

	bool is_param() const
	{
		return ( size() > 1 ) ? (p1[0] == '-' && p1[1] == '-') : false;
	}
};


struct AdminMgr{

	typedef std::function<void(std::vector<token>&, int)> AdminFunc;
	std::map<std::string,AdminFunc> adminF;

	template <typename T>
	void reg(const std::string& name_, std::function<void(T&)> f)
	{
		adminF[name_] = [&](std::vector<token>& tokens, int offset) { 
			T t; 
			PerfBuckets pb;
			populate(tokens, offset, t.init(), t);
			auto p = pb.probe();

			std::cout  << " probe="<< p << '\n'  << std::endl;

			f(t); 
		};
	}

	void run(const char* cmd_)
	{
		std::vector<token> tokens;
		tokenize(tokens, cmd_);
		adminF[tokens[0].sref().as_string()](tokens,1);	
	}

	void tokenize(std::vector<token>& tokens, const char* cmd_)
	{
		char p = ' ';
		char t = '\0';
		bool e = false;

		const char* pos = cmd_;
		
		while( *pos )
		{
			/*
			std::cout  
				<< " pos="<< pos
				<< " t=["  << t <<"]"
				<< " p=["  << p <<"]"
				<< " e="  << e
				<< '\n'  << std::endl;
			*/

			if( *pos == ' ' && t && t != 'f' && (p != t || e) )
			{
				if(*(pos+1) == '\0')
				{
					std::cout  << "unclosed quote or at:"  << (pos - cmd_) << '\n'  << std::endl;
					return;
				}
			}
			else if( *pos == ' ' || (t == 'f' && *(pos+1) == '\0') )
			{
				if ( *(pos+1) == '\0' && *pos != ' ' )
				{
					if(t && t != 'f' && t != *pos)
					{
						std::cout  << "unclosed f at:"  << (pos - cmd_) << '\n'  << std::endl;
						return;
					}
					if(tokens.empty() || tokens.back().p2)
					{
						std::cout  << "internal parsing error at ["  << pos << "] p2:" << tokens.back().p2 << '\n'  << std::endl;
						return;
					}
					tokens.back().p2 = pos+1;
				}
				else if( p != ' ' )
				{
					tokens.back().p2 = pos;
				}
				t = '\0';
			}
			else if( p == ' ' && t && t != 'f')
			{
			}
			else if( p == ' ' )
			{
				if( *pos == '\"' || *pos == '\'' )
					t=*pos;
				else
					t = 'f';

				if(!tokens.empty() && !tokens.back().p2)
				{
					std::cout  << "internal parsing error at ["  << pos << "] p1:" << tokens.back().p1 << '\n'  << std::endl;
					return;
				}

				tokens.push_back(token());
				tokens.back().p1 = pos;

				if( *(pos+1) == '\0' )
				{
					tokens.back().p2 = pos+1;
				}
			}
			e = (!e && p == '\\');
			p = *pos;
			++pos;
		}
	}

	template<size_t M>
	bool populate(std::vector<token>& tokens, int offset, const BaseCmd::pvec<M>& pvec_, BaseCmd& b)
	{

/*
		for(auto& t : tokens)
		{
			std::cout  
				<< " p1=" << t.p1 - cmd_
				<< " p2=" << t.p2 - cmd_
				<< " c =" << t.as_string()
				<< '\n'  << std::endl;
		}

*/


		int i=offset;
		while(i<tokens.size())
		{
			auto& t = tokens[i];

			if( t.is_param() )
			{
				auto it = std::find_if(pvec_.begin(), pvec_.end(), [&](BaseCmd::FldFunc pfunc)
				{ 
					const auto& p = pfunc(&b);
					return p.is_param && p.name == t.param();
				});

				if( it == pvec_.end() )
				{
					std::cout  << "failed to find param:" << t.param().as_string() << '\n'  << std::endl;
					return false;
				}

				if( ++i < tokens.size() )
				{
					const auto& p = (*it)(&b);
					if ( !p.fld.assign(tokens[i].sref()) )
					{
						std::cout  << "failed to assign param:" << p.name << " with value ["  << tokens[i].sref().as_string() << "]\n"  << std::endl;
						std::cout  << "failed to assign with value len ["  << tokens[i].sref().len << "]\n"  << std::endl;
					}
				}
			}
			else
			{
				if(i-offset < pvec_.size())
				{
					const auto& p = pvec_[i-offset](&b);
					if(!p.is_param)
					{
						if ( !p.fld.assign(tokens[i].sref()) )
						{
							//std::cout  << "failed to assign param:" << p.name << " with value:"  << tokens[i].sref() << '\n'  << std::endl;
						}
					}
				}
			}
		++i;
		}

	  context.checkpoint<PrefetchCheckpointId<PrefetchId::Work> >();

		return true;
	}
};

struct UserCmd : public BaseCmd{

	FLDN(UserCmd,IntFld,sz,false)
	FLDN(UserCmd,FlgFld,eval,true)
	FLDN(UserCmd,StrFld,sd,true)

	static constexpr BaseCmd::pvec<3> init()
	{
		return { &UserCmd::szF, &UserCmd::evalF, &UserCmd::sdF };
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
			std::cout  << "pass quoted command with format: int str onoff ex: 12 str off" <<  '\n'  << std::endl;
			return 1;
		}
		else
		{
			std::cout  << "command:" << argv[1] << '\n' << std::endl;
		}
		
		AdminMgr adminMgr;

		context.run();

		adminMgr.reg<UserCmd>("tail", cmdF);


		adminMgr.run(argv[1]);


    return 0;
}
