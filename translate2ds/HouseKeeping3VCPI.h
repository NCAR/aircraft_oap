#pragma once
#include "Packet.h"
#include <cmath>

namespace sp
{
	struct HouseKeeping3VCPI: public Packet
	{
		// Skip ID, len, and check_sum
		word	_data[80];

		float true_airspeed()
		{
			float tas;
			short *s = (short *)&_data[73];
			short *d = (short *)&tas;
			d[0] = s[1];
			d[1] = s[0];
			return tas;
		}

		float relative_humidity()
		{
			float rh = 0.002515185 * _data[26] - 28.02198;
			return rh;
		}

		/// 48 bit timing word / clock counter.
		long long fclk()
		{
			long long fclk = 0;
			short *s = (short *)&_data[70];
			fclk += s[0]; fclk <<= 16;
			fclk += s[1]; fclk <<= 16;
			fclk += s[2];
			return fclk;
		}

		/// First 26 shorts of the housekeeping packet are all temperatures.
		float temperature(int i)
		{
			if (i < 0 && i > 24)
				return NAN;

			float rt = 6.5536e9 * (1.0 - (float)_data[i]/65536.0) / (5.0*_data[i]);
			float t = -273.15 + pow(1.1117024e-3 + 237.02702e-6*log(rt)+75.78814e-9*pow(log(rt),3.0), -1.0);
			return t;
		}
	};


	template<class T>
	inline T&	operator >> (T& reader, HouseKeeping3VCPI& in)
	{
		reader.read(reinterpret_cast<byte *>(&in._data[0]), sizeof(in._data));
		in.PacketID = in._data[0];
		return reader;
	}

}
