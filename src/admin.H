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

#include "meta.h"

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

	bool populate(std::vector<token>& tokens, size_t offset, const BaseCmd::pvec& pvec_, BaseCmd& b);

	template <typename T>
	void reg(const std::string& name_, std::function<void(T&)> f)
	{
		adminF[name_] = [&](std::vector<token>& tokens, int offset) { 
			T t; 
			populate(tokens, offset, t.init(), t);
			f(t); 
		};
	}

	void run(const char* cmd_);

	void tokenize(std::vector<token>& tokens, const char* cmd_);

};
