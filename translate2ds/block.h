#pragma once

#include <vector>
#include "common.h"
#include "PacketTypes.h"

namespace sp
{

	struct block_incomplete: protected std::exception
	{

	};

	class Block
	{
	public:
		Block(unsigned int lineSize, Endianness s, Endianness d) : _head(0), _lineSize(lineSize), _srcEnd(s), _dstEnd(d){_data.reserve(1024*1024*10);}

		Block&	operator >> (float32& in) {return read(in);}
		Block&	operator >> (word& in) {return read(in);}
		Block&	operator >> (uint32_t& in) {return read(in);}
		Block&	operator >> (uint64_t& in) {return read(in);}

		template<class D>
		Block& read(D& obj)
		{
			read(reinterpret_cast<byte*>(&obj), sizeof(obj));
			return *this;
		}


		void	read( byte* pBytes, size_t length )
		{
			size_t remains = remaining();
			if(remains < length)
			{
				throw block_incomplete();
				return;
			}

			memcpy(pBytes, &_data[_head], length);
			_head += length;
		}


		Endianness	SourceEndian()const{return _srcEnd;}
		Endianness	DestinationEndian()const{return _dstEnd;}


		void add_line()
		{_data.resize(_data.size() + _lineSize);}

		byte *fill_begin()
		{return &_data[_data.size() - _lineSize];}
		unsigned int fill_length()
		{return _lineSize;}

		int countParticles()
		{
			// Count the number of instances of DATA marker in block. This is equal to 
			// the number of particles in the current record.
			int count = 0;
			unsigned short val;
			for (int i = 0 ; i < _data.size(); i+=2) {
				// Byte swap next 4 bytes, cast to unsigned short, and 
				// compare with DATA from PacketTypes.h
				val = (_data[i+1] <<8) | _data[i];
				if (val == DATA) {count++;}
			}
			return count;
		}

		// Print the contents of a block (in hex). Useful for debugging.
		void print() const
		{
			for (int i = 0 ; i < _data.size(); i++) {
				printf("%x ",_data[i]);
		 	}
			printf("\n");
		}

		size_t size() const
		{return _data.size();}

		size_t remaining() const
		{return size() - _head;}

		void clear()
		{_data.erase(_data.begin(), _data.begin() + _head); _head = 0;}

		size_t head() const
		{return _head;}

		void go_to(size_t s)
		{ _head = s;}


		void go_to_end()
		{ _head = size(); }

	private:

		typedef std::vector<byte> Data;

		size_t		_head;
		unsigned int	_lineSize;
		Data		_data;
		Endianness	_srcEnd,_dstEnd;
	};

	template<class T>
	inline T&	operator >> (T& reader, Block& in)
	{
		in.add_line();
		reader.read(in.fill_begin(), in.fill_length());
		return reader;
	}

}
