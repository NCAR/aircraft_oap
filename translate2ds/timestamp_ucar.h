#pragma once

#include "Packet.h"
#include "log.h"
namespace sp
{
	//UCAR timestamp, data is in Big Endian Format
	struct TimestampUCAR
	{
		word	CharacterCode;
		Word	Hour;
		Word	Minute;
		Word	Second;
		Word	Year;
		Word	Month;
		Word	Day;
		word	TAS;
		Word	Milliseconds;
		word	overload;

		TimeStamp16		As2DSTimeStamp() const
		{
			TimeStamp16 ts;
			ts.wDay			=	 swap_endian(Day);
			ts.wYear		=	 swap_endian(Year);
			ts.wMonth		=	 swap_endian(Month);
			ts.wSecond		=	 swap_endian(Second);
			ts.wMilliseconds	=	 swap_endian(Milliseconds);
			ts.wHour		=	 swap_endian(Hour);
			ts.wMinute		=	 swap_endian(Minute);
			ts.wDayOfWeek		=	0;
			return ts;

		}
	};

	template<class T>
	inline T&	operator >> (T& reader, TimestampUCAR& in)
	{
		reader >> in.CharacterCode >> in.Hour >> in.Minute >> in.Second >> in .Year >> in.Month >> in.Day >> in.TAS >> in.Milliseconds >> in.overload;

		return reader;
	};
}
