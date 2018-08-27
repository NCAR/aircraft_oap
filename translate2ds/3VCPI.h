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
		Device3VCPI(Options& opt):_options(opt)
		{
		}
		template<class Reader, class Writer>
		void ProcessData(Reader& f, Writer& writer);

	private:
		template<class Writer>
		void process_block( Block &block, Writer& writer );

		Log _log;

		Options&	_options;
	};
}


#include "File.h"
#include "block.h"
#include "PacketTypes.h"
#include "HouseKeeping3VCPI.h"
#include "Mask3VCPI.h"
#include "Particle3VCPI.h"
#include "ctime"

namespace sp
{

	template<class Reader, class Writer>
	void Device3VCPI::ProcessData( Reader& f, Writer& writer )
	{
		TimeStamp16	time_stamp;
		Word		check_sum;
		Block		block(SIZE_DATA_BUF, f.SourceEndian(), f.DestinationEndian());
		size_t 		numParticlesInRecord;

		while (!f.empty())
		{
			f >> time_stamp;
			if (f.empty()) break;

			// If a time offset was given on the command line, apply it here. Save
			// time to tm struct so can use built-in time conversions to handle 
			// day/month/year rollover, etc. Can't use tm everywhere because it doesn't
			// include milli-seconds, which are critical for this data.
			
			// Log time. Useful for debugging.
			//_log <<"\nTime before offset: " << time_stamp.toSimpleString().c_str() <<"\n";

			// Create a tm struct to copy time info too.
			struct tm timeinfo;

			// Overwrite the initialized time with the record time.
			//printf("%d\n",_options.TimeOffset);
			timeinfo.tm_year = time_stamp.wYear - 1900;
			timeinfo.tm_mon = time_stamp.wMonth -1;
			timeinfo.tm_mday = time_stamp.wDay;
			timeinfo.tm_hour = time_stamp.wHour;
			timeinfo.tm_min = time_stamp.wMinute;
			timeinfo.tm_sec = time_stamp.wSecond + _options.TimeOffset;

			// Let struct tm do it's magic (propogate offset to day/month/year as needed)
			time_t t = mktime(&timeinfo); // mktime returns the time as localtime...
			struct tm *tout = localtime(&t);// so use localtime to convert it back.

			// Put the adjusted time back into the 2D time struct
			time_stamp.wSecond = tout->tm_sec;
			time_stamp.wMinute = tout->tm_min;
			time_stamp.wHour = tout->tm_hour;
			time_stamp.wDay = tout->tm_mday;
			time_stamp.wMonth = tout->tm_mon + 1;
			time_stamp.wYear = tout->tm_year + 1900;

			writer << time_stamp;
			// Log time of block. Useful for debugging.
			//_log <<"Time  after offset: " << time_stamp.toSimpleString().c_str() <<"\n";
			f >> block;	// read in a block of data from the file
			f >> check_sum;

			// block should contain 4096 bytes, but this print statement shows it
			// gets more if in a high-particle area. Not sure what this indicates,
			// if anything.
			//_log << "Block size: " << block.size() <<"\n";

			// This will print out the contents of the block in hex. 
			// Useful for debugging
			//block.print();

			// Only process records with more than 5 particles. A count of <5
			// particles indicates a stuck bit. This was added to eliminate
			// runaway stuck bits that make the output file huge.
			numParticlesInRecord = block.countParticles();
			//_log << "Found " << block.countParticles() << " particles in this record.\n";
			if (numParticlesInRecord > 5) {
				process_block(block, writer); 
			} else {
				// If don't process block, need to clear it before read next one.
				block.go_to_end();
				block.clear();
			}
		}
		//_log <<"\nTotal Housekeeping packets: " << nHouses <<"\n";
	}

	template< class Writer>
	void Device3VCPI::process_block( Block &block, Writer& writer )
	{
		int PC = 0;
		static int NL = 0;

		//_log << "\n\n+++++++++++++++++NEW BLOCK++++++++++++++++++++\n\n";
		size_t head = 0;
		static ParticleRecord3VCPI particle;
		try
		{
			Word w;
			while(block.remaining() > 0)
			{
				head = block.head();

				block >> w; // Read a word and swap endian


				switch(w)
				{
				case HOUSEKEEPING:
					{
					HouseKeeping3VCPI hk;
					block >> hk;
					writer << hk;
					//_log << "HouseK\n";
					} break;

				case MASK:
					{
					MaskData3VCPI md;
					block >>md;
					//_log << "(:Mask:)\n";
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

					//_log << "**ParticleFrame**\n";
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
					//_log << "UNUSED FRAME\n";
					//block.go_to_end();			
					} break;

				case 0:	// No data.  After a flush.
					{
					//_log << "END OF FRAME\n";
					} break;

				default:
					{
					/*static int count = 0;
					block.clear();
					word val= w;
					char first = val >> 8;
					char second = val & 0x00ff;

					_log <<"\n3VCPI (" <<count << ")Got a packet header that isn't recognized : " << word(w) << "  ASCI: " << first << " " << second << "\n";
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
		// Independent particle count from during processing - sanity check.
		//_log <<"Total Particle Count for this record: "<< PC <<"\n";

		block.clear();
	}
}
