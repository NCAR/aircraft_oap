#pragma once
#include "Packet.h"
//#include <bitset>
namespace sp
{
	

	struct MaskData3VCPI: public Packet
	{
		word _data[25];
	};

	template<class T>
	inline T&	operator >> (T& reader, MaskData3VCPI& in)
	{
		reader.read(reinterpret_cast<byte*>(&in._data[0]),sizeof(in._data));
		in.PacketID = in._data[0];
		return reader;
	}
}
