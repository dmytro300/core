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

	template<size_t M>
	bool populate(std::vector<token>& tokens, size_t offset, const BaseCmd::pvec<M>& pvec_, BaseCmd& b)
	{
		size_t i = offset;
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
							std::cout  << "failed to assign param:" << p.name << " with value:"  << tokens[i].sref().as_string() << '\n'  << std::endl;
						}
					}
				}
				else
				{
					std::cout  << "no corresponding passed required param found:" << t.param().as_string() << '\n'  << std::endl;
				}
			}
		++i;
		}

		return true;
	}
};
