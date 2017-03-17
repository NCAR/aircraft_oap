#pragma once

namespace sp
{
	enum PacketType
	{
		HOUSEKEEPING = 0x484b,			//hk
		MASK = 0x4d4b,				//MK
		DATA = 0x3253,				//2S
		FLUSH = 0x4e4c,				//NL
		TAS_H = 0x5454,				//tt
		TAS_V = 0x4141,				//aa

		H_PARTICLE = 0x4848,			//HH
		V_PARTICLE = 0x5656,			//VV

		H_DEADTIME = 0x6C6C,			//LL
		V_DEADTIME = 0x7C7C,			//ll

		H_MASK	= 0x1C1C,			//FSFS
		V_MASK	= 0x2C2C,			//,,

		V_DUMMY_DEADTIME = 0x1414,		
		H_DUMMY_DEADTIME = 0x1424,

		V_BACKUP_DEADTIME = 0x2414,
		H_BACKUP_DEADTIME = 0x2424,

		V_STREAKER = 0x1818,
		H_STREAKER = 0x1919
	};
}
