#pragma once
#include <string>
#include <fstream>
#include <vector>
#include "common.h"

namespace sp
{
	class Log;

	class File
	{
	public:
		File(const std::string& path):_in(path.c_str(), std::fstream::binary)
		{
			_buffer.resize(BUFFER_SIZE);

			_in.seekg( 0, std::ios::end );

			std::streamoff sizeBytes = _in.tellg();
			_megabytes = float(sizeBytes >> 20);
			_in.seekg( 0, std::ios::beg );



		//	_in.open(path.c_str());//, std::ifstream::binary);

			//	get_next_buffer();

			_srcEnd = ENDIAN_LITTLE;
			_dstEnd	= ENDIAN_LITTLE;
		}


		void process();

		File&	operator >> (float32& in) {return read(in);}
		File&	operator >> (word& in) {return read(in);}
	

		template<class T>
		File& read (T& in)
		{
			read(reinterpret_cast<byte*>(&in), sizeof(in));
			return *this;
		}

		void read(byte* bytes, unsigned int size)
		{
			_in.read(reinterpret_cast<char*>(bytes), size);
			std::streamsize count =  _in.gcount();
			if(count != size)
			{
				_buffer.clear();
			}
			return;
		}

		Endianness SourceEndian()const{return _srcEnd;}
		Endianness DestinationEndian()const{return _dstEnd;}


		bool empty()const
		{ return _buffer.empty(); }

		bool is_open()const{return const_cast<File*>(this)->_in.is_open();} //idiotically cygwin gcc appears to not have a const version of is_open...


		float MegaBytes()const
		{ return _megabytes; }

	private:

		void get_next_buffer()
		{
			_in.read(&_buffer[0],_buffer.size());
			std::streamsize count = _in.gcount();

			if(count < _buffer.size())
			{
				_buffer.resize(count);
			}

			_location = 0;
		}

		std::ifstream		_in;
		unsigned long long int	_Length;

		enum
		{
			BUFFER_SIZE = 1024*1024
		};

		typedef std::vector<char> Buffer;
		Buffer		_buffer;
		unsigned int	_location;
		float		_megabytes;


		Endianness	_srcEnd,_dstEnd;

	};
}
