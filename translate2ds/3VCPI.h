#pragma once
#include "log.h"

namespace sp
{
	class File;
	class Block;
	//16 byte timestamp
	//4096 byte block
	//2 byte checksum
	//blocks can contain any of: verticle particle, horizontal particle, housekeeping, or mask
	class Device3VCPI
	{
	public:
		Device3VCPI()
		{
		}
		template<class Reader, class Writer>
		void ProcessData(Reader& f, Writer& writer);

	private:
		template<class Writer>
		void process_block( Block &block, Writer& writer );

		Log _log;
	};
}


#include "File.h"
#include "block.h"
#include "PacketTypes.h"
#include "HouseKeeping3VCPI.h"
#include "Mask3VCPI.h"
#include "Particle3VCPI.h"

namespace sp
{

	template<class Reader, class Writer>
	void Device3VCPI::ProcessData( Reader& f, Writer& writer )
	{
		TimeStamp16	time_stamp;
		Word		check_sum;
		Block		block(SIZE_DATA_BUF, f.SourceEndian(), f.DestinationEndian());

		while (!f.empty())
		{
			f >> time_stamp;
			writer << time_stamp;
			f >> block;
			f >> check_sum;

			process_block(block, writer);
		}
	}

	template< class Writer>
	void Device3VCPI::process_block( Block &block, Writer& writer )
	{
		static int PC = 0;
		static int NL = 0;
	
		size_t head = 0;
		static ParticleRecord3VCPI particle;
		try
		{
			Word w;
			while(block.remaining() > 0)
			{
				head = block.head();

				block >> w;


				switch(w)
				{
				case HOUSEKEEPING:
					{
					HouseKeeping3VCPI hk;
					block >> hk;
					writer << hk;
					} break;

				case MASK:
					{
					MaskData3VCPI md;
					block >>md;
					} break;

				case DATA:
					{
					particle.clear();
					particle.setData(true);
					block >> particle;
					writer << particle;
					PC++;
					/*if(word(particle.NumSlicesInParticle)  > particle.HorizontalImage._data.size())
					{
						_log <<"BAD\n";
					}*/
					//	_log << particle2;

				//	_log << "**ParticleFrame**\n";
					} break;

				case FLUSH:
					{
					/*	if(NL == 185)
					{
					NL++;
					}*/
					particle.clear();
					particle.setData(false);
					block >> particle;
					NL++;
				//	_log << "UNUSED FRAME\n";
				//	block.go_to_end();			
					} break;

				case 0:	// No data.  After a flush.
					{
					} break;

				default:
					{
					/*static int count = 0;
					block.clear();
					word val= w;
					char first = val >> 8;
					char second = val & 0x00ff;

					_log <<"\n3VCPI (" <<count << ")Got a packet header that isn't recognized : " << word(w) << "  ASCI: " << first << " " << second;
					count++;*/
					//head = block.head();
					//block.go_to(head - sizeof(word)*5);
					//for(int  i = 0;i<10;++i)
					//{
					//	block >> w;
					//	word val= w;
					//	char first = val >> 8;
					//	char second = val & 0x00ff;

					//	_log <<"\nGot a packet header that isn't recognized : " << word(w) << "  ASCI: " << first << " " << second;
					//}
					//		throw std::exception("bad");

					} break;
				}
			}
		}
		catch (block_incomplete&)
		{
			//	_log <<"\nBAD READ: " << "head: " << head << "cur: " << block.head();
			//this means we ran into the end of the block before reading finished
			block.go_to(head);
		}

		block.clear();
	}
}
