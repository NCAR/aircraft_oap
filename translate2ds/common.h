#pragma once
#include <string>
#include <cstring>
#include <stdint.h>
#include <sstream>

namespace sp
{
	typedef unsigned char	byte;
	typedef unsigned char	ubyte;

	typedef uint16_t	word;	//word which defaults to unsigned
	typedef word		uword;	//unsigned word
	typedef short		sword; //signed word
	typedef float		float32;

	enum DataBufferSize
	{
		SIZE_DATA_BUF = 4096,
		SIZE_HSKP_BUF = 164,
		SIZE_MASK_BUF = 54
	};

	enum Endianness
	{
		ENDIAN_BIG,
		ENDIAN_LITTLE
	};

	const uint64_t Fast2D_Sync = 0xAAAAAA0000000000LL;

	inline void swap_endian(byte* bytes, unsigned int nBytes, Endianness source, Endianness dest )
	{
		if (source == dest)
			return;

		byte* head = bytes;
		byte* last = bytes + nBytes - 1;

		while (head < last)
		{
			byte temp = *head;
			*head = *last;
			*last = temp;

			head++;
			last--;
		}

	}
	inline void swap_endian_force(byte* bytes, unsigned int nBytes){swap_endian(bytes, nBytes, ENDIAN_LITTLE, ENDIAN_BIG);}

	template<class Reader, class T>
	inline T& swap_endian(Reader& r, T& v)
	{
		swap_endian(reinterpret_cast<byte*>(&v), sizeof(v), r.SourceEndian(), r.DestinationEndian());
		return v;
	};


	template<class T>
	inline T swap_endian( T v)
	{
		swap_endian(reinterpret_cast<byte*>(&v), sizeof(v), ENDIAN_LITTLE, ENDIAN_BIG);
		return v;
	};

	

	struct Word
	{
		Word(){}
		Word(word v):val(v){}
		operator word(){return val;}
		operator double()const{return double(val);}
		bool operator < (const Word& w2) const{return val < w2.val;}
		bool operator > (const Word& w2) const{return val > w2.val;}
		bool operator == (const Word& w2) const{return val == w2.val;}

		std::string toString()const
		{
			char buffer[100];
			
			int v = val;
			sprintf(buffer, "%i", v);
			return std::string(buffer); 
		}

	private:
		word	val;
	};

		template<class T>
		inline T&	operator >> (T& reader, Word& in)
		{
			word temp;
			reader.read(temp);
			in = Word( swap_endian(reader, temp));
			return reader;
		}

		template<class T>
		inline T&	operator <<= (T& writer, Word& out)
		{
			writer << word(out);
			return writer;
		}

		template<class T>
		inline T&	operator << (T& writer, Word& out)
		{
			//writer << word(out);
			writer.write(reinterpret_cast<char*>(&out), sizeof(out));
			return writer;
		}


	//volts conversion is 0.00244140625
	struct Volts24
	{
		operator float(){return val;}
		float val;
	};

		template<class T>
		inline T&	operator >> (T& reader, Volts24& in)
		{
			word temp;
			reader.read(temp);
			temp = Word( swap_endian(reader, temp));
			in.val = float(temp)*0.00244140625f;
			return reader;
		};

		template<class T>
		inline T&	operator << (T& writer, Volts24& out)
		{
			return writer << out.val;
		};



	//volts conversion is 0.001220703
	struct Volts12
	{
		operator float(){return val;}
		float val;
	};

		template<class T>
		inline T&	operator >> (T& reader, Volts12& in)
		{
			word temp;
			reader.read(temp);
			temp = Word( swap_endian(reader, temp));
			in.val = float(temp)*0.001220703f;
			return reader;
		};

		template<class T>
		inline T&	operator << (T& writer, Volts12& out)
		{
			return writer << out.val;
		};



	//volts conversion is 0.00488400488
	struct Volts48
	{
		operator float(){return val;}
		float val;
	};

		template<class T>
		inline T&	operator >> (T& reader, Volts48& in)
		{
			word temp;
			reader.read(temp);
			temp = Word( swap_endian(reader, temp));
			in.val = float(temp)*0.00488400488f;
			return reader;
		};

		template<class T>
		inline T&	operator << (T& writer, Volts48& out)
		{
			return writer << out.val;
		};



	struct Celcius
	{
		operator float(){return val;}
		float val;
	};

		template<class T>
		inline T&	operator >> (T& reader, Celcius& in)
		{
			word temp;
			reader.read(temp);
			temp = Word( swap_endian(reader, temp));
			in.val = 1.6f + float(sword(temp))*0.00244140625f;
			return reader;
		};

		template<class T>
		inline T&	operator << (T& writer, Celcius& out)
		{
			return writer << out.val;
		};



	struct PSI
	{
		operator float(){return val;}
		float val;
	};

		template<class T>
		inline T& operator >> (T& reader, PSI& in)
		{
			word temp;
			reader.read(temp);
			temp = Word( swap_endian(reader, temp));
			in.val = -3.846f + float(temp)*0.018356f;
			return reader;
		}

		template<class T>
		inline T& operator << (T& writer, PSI& out)
		{
			return writer << out.val;
		}


	struct Timing
	{
		Timing(){}
		Timing(uint64_t v):time(v){}
		operator uint64_t()const{return time;}

	private:
		uint64_t time;
	};

		template<class T>
		inline T& operator >> (T& reader, Timing& in)
		{
			uint64_t temp;
			reader.read(temp);
			in = Timing(swap_endian(reader, temp));
			return reader;
		}


		template<class T>
		inline T& operator << (T& writer, Timing& out)
		{
			uint64_t v = out;
			writer.write(reinterpret_cast<char*>(&v), sizeof(v));
			return writer;
		}



	//16 byte timestamp based on GetSystemTime
	struct TimeStamp16
	{
		Word wYear;
		Word wMonth;
		Word wDayOfWeek;
		Word wDay;
		Word wHour;
		Word wMinute;
		Word wSecond;
		Word wMilliseconds;

		double TimeAsSingleValue()const
		{
			return double(wYear)*60.0*24.0*30.0*12.0 + double(wMonth)*60.0*24.0*30.0 + double(wDay)*60.0*24.0 + double(wHour)*60.0 + double(wMinute) + double(wSecond)*(1.0/60.0) + double(wMilliseconds) /(1000.0);
		}
		std::string toSimpleString()
		{
			std::stringstream ts;
			ts << wYear.toString() << "/" << wMonth.toString() << "/" << wDay.toString() << " "
				<< wHour.toString() << ":" << wMinute.toString() << ":" << wSecond.toString()
				<< "." << wMilliseconds.toString();
			return ts.str();
		}
		bool operator < (const TimeStamp16& o) const
		{
			double v1 = TimeAsSingleValue();
			double v2 = o.TimeAsSingleValue();
			return v1 < v2;
		}

		bool operator > (const TimeStamp16& o) const
		{
			return o < *this;
		}
	};


		template<class T>
		inline T& operator >> (T& reader, TimeStamp16& in)
		{
			reader.read(reinterpret_cast<byte*>(&in), sizeof(in));
			return reader;
		}

		template<class T>
		inline T& operator << (T& writer, TimeStamp16& ts)
		{
			return writer << ts.wYear << ts.wMonth << ts.wDayOfWeek << ts.wDay << ts.wHour << ts.wMinute << ts.wSecond << ts.wMilliseconds;
		}


	struct Options
	{
		Options()
		{
			Compressed = false;
			ascii_art = false;
			memset(&StartTime, 0, sizeof(StartTime)); //min date so defaults to always passing
			memset(&EndTime, 255, sizeof(EndTime)); //max date so defaults to always passing
			EndTime.wDay = 30;
			EndTime.wHour = 24;
			EndTime.wMinute = 60;
			EndTime.wMilliseconds = 1000;
			EndTime.wMonth = 12;
			EndTime.wSecond = 60;
			Platform = "?";
			Project = "?";
			FlightNumber = "?";
			SerialNumber = "SPEC001";
			MinParticle = 0;
			MaxParticle = 0xFFFFFFFF;//std::numeric_limits<size_t>::max(); <-- limits appears to be missing from some cygwin installs
			
		}

		bool InRange(const TimeStamp16& ts)const
		{
			return StartTime < ts && ts < EndTime;
		}

		TimeStamp16	StartTime;
		TimeStamp16	EndTime;
		std::string	OutputDir;
		bool		Compressed;
		bool		ascii_art;	//outputs ascii art that lets us see the particles in text form

		std::string	Platform;
		std::string	Project;
		std::string	FlightNumber;
		std::string	SerialNumber;
		
		size_t		MinParticle, MaxParticle; 
	};
}
