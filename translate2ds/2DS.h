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
	class Device2DS
	{
	public:
		template<class Reader, class Writer>
		void	Process(Reader& f, Writer& writer);

		
	private:
		template<class Writer>
		void	process_block( Block &block, Writer& writer );


		Log    _log;

	};
}

#include "2DS.h"
#include "File.h"
#include "block.h"
#include "PacketTypes.h"
#include "HouseKeeping.h"
#include "MaskData.h"
#include "2DSParticle.h"

namespace sp
{
	static int nHouses = 0;

	template<class Reader, class Writer>
	void Device2DS::Process( Reader& f, Writer& writer )
	{
		TimeStamp16	time_stamp;
		Word		check_sum;
		Block		block(SIZE_DATA_BUF, f.SourceEndian(), f.DestinationEndian());

		while(!f.empty())
		{
			f >> time_stamp;
			writer << time_stamp;
			f >> block;
			f >> check_sum;

			process_block(block, writer);
		}
		_log <<"\nTotal Housekeeping packets: " << nHouses <<"\n";
	}

	template< class Writer>
	void Device2DS::process_block( Block &block, Writer& writer )
	{
		//	_log << "\n\n+++++++++++++++++NEW BLOCK++++++++++++++++++++\n\n";
		size_t head = 0;
		static 		ParticleRecord particle;
		try
		{
			Word w;
			while(block.remaining() >= sizeof(Word))
			{
				head = block.head();

				block >> w;


				switch(w)
				{
				case HOUSEKEEPING:
					{
					HouseKeeping hk;
					block >> hk;
					writer << hk;
					//	_log << hk;
					nHouses++;
					//	_log << "HouseK\n";
					}break;
				case MASK:
					{
					MaskData md;
					block >>md;
					//	_log << "(:Mask:)\n";
					//	_log << md;
					}break;
				case DATA:
					{
					static int PC =0;
						
					particle.clear();

					block >> particle;
					writer << particle;
					PC++;
					/*if(word(particle.NumSlicesInParticle)  > particle.HorizontalImage._data.size())
					{
						_log <<"BAD\n";
					}*/
					//	_log << particle2;
	
					//	_log << "**ParticleFrame**\n";
					}break;
				case FLUSH:
					{
					//	_log << "FLUSH FRAME\n";
					block.go_to_end();			
					}break;
				case 0: //indicates the end of the file it seems?
					{
					//	_log << "END OF FILE\n";
					block.go_to_end();	
					}break;
				default:
					{
					static int count = 0;
					block.clear();
					word val= w;
					char first = val >> 8;
					char second = val & 0x00ff;

					_log <<"\n2DS (" <<count << ")Got a packet header that isn't recognized : " << word(w) << "  ASCI: " << first << " " << second;
					count++;
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

					}break;

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
