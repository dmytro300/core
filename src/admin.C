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

