#pragma once

#include <vector>
#include <array>
#include <map>
#include <string>
#include <string.h>
#include <tuple>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>



struct strref
{
	size_t len;
	const char* str;
	strref(const std::string& s_) : len(s_.length()), str(s_.data()) {}
	strref(const std::string_view& s_) : len(s_.length()), str(s_.data()) {}
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

inline std::ostream &operator<<(std::ostream &os, const strref &m) { 
	return os << m.str;
}

struct strref_hasher {
  std::size_t operator()(const strref &m)const{
    return std::hash<std::string_view>()(m.str);
  }
};

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

	constexpr std::string_view sview() const
	{
		return std::string_view(str,len);	
	}
	
	bool operator < (const cstr& rhs_) const
	{
		return ::memcmp(str, rhs_.str, len) < 0;
	}

	bool operator == (const strref& s_) const
	{
		if(len != s_.len)
			return false;
		return ::memcmp(str, s_.str, len) == 0;
	}

	bool operator == (const std::string& s_) const
	{
		return *this == strref(s_);
	}
};

template<size_t S>
inline std::ostream &operator<<(std::ostream &os, cstr<S> const &m) { 
	return os << m.str;
}

template<size_t S>
bool operator==(const cstr<S> &l, cstr<S> const &r) { 
	return l==r;
}

template<size_t S>
bool operator<(const cstr<S> &l, const std::string_view &r) { 
	return l.sview() < r;
}

template<size_t S>
bool operator<(const std::string_view &l, const cstr<S> &r) { 
	return l < r.sview();
}

template<size_t S>
struct cstr_hasher {
  std::size_t operator()(const cstr<S> &m)const{
    return std::hash<std::string_view>()(m.str);
  }
};

/*
namespace std {
	template<>
	struct less<strref> {
		bool operator()(const strref &l, strref const &r) const {
			return true;
		}
	};
}

namespace std {
	template<size_t S>
	struct less<cstr<S>> {
		bool operator()(const cstr<S> &l, std::string_view const &r) { 
				auto len = std::min(l.len,r.length());
				return ::memcmp(l.str,r.data(),len) < 0 || l.len < r.length();
		}
	};
}
*/

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
		return v != 0.0 || v_ == "0" || v_ == "0.0";
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
	
	using pvec = std::array<FldFunc,64>; 
};

#define FLDD(T,N,D,P) T N{D}; static constexpr BaseCmd::pfield N##F(BaseCmd* p) { return {#N,P,static_cast<self*>(p)->N};}
#define FLDN(T,N,P)      T N; static constexpr BaseCmd::pfield N##F(BaseCmd* p) { return {#N,P,static_cast<self*>(p)->N};}

/*
	Usage:

	FLDN(IntFld,sz,false)

	Expands to:

	IntFld sz;
	static BaseCmd::pfield szF(BaseCmd* p)
	{
		return {"sz",true,static_cast<self*>(p)->sz};
	}
*/

