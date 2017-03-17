#pragma once
#include "common.h"
#include <string>
namespace sp
{
	class Log;

	struct Packet
	{
		Word		PacketID;
	};

	template<class T>
	inline T&	operator >> (T& reader, Packet& in)
	{
		reader >> in.PacketID;
		return reader;
	}




}
