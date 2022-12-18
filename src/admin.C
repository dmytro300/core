#include "meta.h"
#include "admin.h"


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

void AdminMgr::run(const char* cmd_)
{
	std::vector<token> tokens;
	tokenize(tokens, cmd_);
	adminF[tokens[0].sref().as_string()](tokens,1);	
}

bool AdminMgr::populate(std::vector<token>& tokens, size_t offset, const BaseCmd::pvec& pvec_, BaseCmd& b)
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

void AdminMgr::tokenize(std::vector<token>& tokens, const char* cmd_)
{
	char p = ' ';
	char t = '\0';
	bool e = false;

	const char* pos = cmd_;
	
	while( *pos )
	{
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

