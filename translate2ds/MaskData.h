#pragma once
#include "Packet.h"
//#include <bitset>
namespace sp
{
	struct MaskBits
	{
		Word	one,two,three, four,five, six,seven, eight;
	};

//	static_assert(sizeof(MaskBits) == 8*2, "MaskBits wrong size");

	template<class T>
	inline T&	operator >> (T& reader, MaskBits& in)
	{
		reader >> in.one >> in.two >> in.three >> in.four >> in.five >> in.six >> in.seven >> in.eight;
		return reader;
	}

	struct MaskData: public Packet
	{
		Timing			Time;
	
		//enum 
		//{
		//	BitWords = 11 - 4
		//};
	//	typedef 	std::bitset<sizeof(Word)*(BitWords)>	MaskBits;

		MaskBits		Horizontal;
		MaskBits		Vertical;
		Timing			StartTime;
		Timing			EndTime;

	};


	//static_assert(sizeof(MaskData) == 24*2, "Mask Data wrong size");

	template<class T>
	inline T&	operator >> (T& reader, MaskData& in)
	{
		reader >> in.Time >> in.Horizontal >> in.Vertical >> in.StartTime >> in.EndTime;
		return reader;
	}
}
