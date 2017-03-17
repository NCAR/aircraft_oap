#pragma once

#include "Packet.h"
#include "log.h"

namespace sp
{
	//UCAR XML header 
	struct XMLHeader
	{
		std::string		_header;
		
		std::string		GetFileName() const
		{

		}
	};

	template<class T>
	inline T&	operator >> (T& reader, XMLHeader& in)
	{
		std::vector<char> buf;
		const char* xml_end = "</OAP>\n";
		char t = 0;
		int max_loops = 100000; //so we don't go forever on invalid file

		while (max_loops-- && !reader.empty())
		{
			reader.read(reinterpret_cast<byte*>(&t),sizeof(t));
			in._header += t;

			if(in._header.find(xml_end) != std::string::npos)
			{
				break;
			}
		}
		if(max_loops == 0 || reader.empty())
		{
			g_Log <<"ERROR reading XML header, could not find end of XML\n";
		}

		return reader;
	};
}
