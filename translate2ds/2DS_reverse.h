#pragma once

#include "log.h"
#include <bitset>
#include "timestamp_ucar.h"

namespace sp
{
	class File;
	class Block;
	
	//128 bit compressed UCAR slice
	struct CompressedSlice
	{
	private:
		std::bitset<128> _bits;
	public:

		bool all() const
		//GCC doesn't have all() it seems .. 
		{ return _bits.count() == 128; }

		bool isEmpty()const
		{ return _bits.none(); }

		int numRecords()const
		{
			int ns = 0;
			bool last = false;
			for(int i = 0; i < 128; ++i)
			{
				bool b = _bits[i];
				if (!last && b)
				{
					ns++;
				}
				last = b;
			}
			return ns;
		}

		//starting from index, tell how many bits it is before the value does not equal V
		word countUntilNot(word& index,const bool v) const
		{
			word c = 0;
			for(;index<128;++index,c++)
			{
				if(v !=  _bits[index])
				{
					break;
				}
			}
			return c;
		}

		template<class T>
		void print(T& f) const
		{
			
			for(int i = 0;i<128;++i)
			{
				//f << (_bits[i] ? 'x' : '_');
				f << (_bits[i] ? 'X' : ' ');
			}
			f << '\n';
		}

		void		set(size_t index, bool val)
		{
			_bits.set(index, val);
		}
	};
	template<class T>
	inline T& operator >> (T& reader, CompressedSlice& in)
	{
		size_t c = 0;
		for(int i = 0;i<4;++i)
		{
			uint32_t bits;
			reader >> bits;
			for(int j = 0;j<32;j++, c++)
			{
				bool b = !((bits >> j)&0x00000001);
				in.set(c,b);
			}
			
		}
		
		return reader;
	}

	//reverse translate UCHAR data that was created from 2DS data, back into 2DS data
	class Device2DS_reverse
	{
	public:
		Device2DS_reverse(Options& opt):_options(opt)
		{
			_particleCount = 0;
			_byteReadCount = 0;
			_particleStartCount = 0;
			_LastSliceIncomplete = false;
		}
		template<class Reader, class Writer>
		void			Process(Reader& f, Writer& writer);

	


	private:
		template<class Writer>
		void process_block( Block &block, Writer& writer );


		template<class Writer>
		void write_particle( Writer& writer, Timing time, std::ofstream& textOut );

		template<class Writer>
		void Write2DS( Writer& writer );

		Log _log;

	
		typedef std::vector<TimestampUCAR> time_stamps;
		typedef std::vector<CompressedSlice> slices;
		typedef std::vector<uint32_t> time_slots;
		time_stamps		_time_stamps;
		time_slots		_time_slots;
		slices			_slices;

		unsigned long long	_particleCount;
		unsigned long long	_particleStartCount;
		unsigned long long	_byteReadCount;
		Options&		_options;

		bool			_LastSliceIncomplete;
	};
}


#include "File.h"
#include "block.h"
#include "PacketTypes.h"
#include "HouseKeeping.h"
#include "MaskData.h"
#include "2DSParticle.h"
#include "XML_header.h"


namespace sp
{
	template<class Reader, class Writer>
	void Device2DS_reverse::Process( Reader& f, Writer& writer )
	{

		{
			std::ofstream out_slicedata("temp/slicedata.bin",std::ios::binary);

			XMLHeader xml_header;
			f >> xml_header;

			Word check_sum;
			Block block(SIZE_DATA_BUF, f.SourceEndian(), f.DestinationEndian());

			while(!f.empty())
			{
				TimestampUCAR	time_stamp;
				f >> time_stamp;
	
				_time_stamps.push_back(time_stamp);
				f >> block;
			}
		//	g_Log <<block.size() <<" bytes or " << double(block.size())/4096.0 <<" blocks!\n";
			process_block(block, out_slicedata);
			out_slicedata.close();
		}

		Write2DS(writer);
	}

	
	namespace
	{
		struct IsEndOfParticle
		{
			template<class T>
			bool operator () (Block &block, T& v) const
			{
				if(v.size() < 3)
					return false;

				size_t lastIndex = v.size() -1;
				if(v[lastIndex].isEmpty() && v[lastIndex-1].isEmpty() && v[lastIndex-2].isEmpty() )
				{
					return true;
				}
				return false;
			}
		};
	}
	template< class Writer>
	void Device2DS_reverse::process_block( Block &block, Writer& writer )
	{
		std::ofstream asciiOut;
		if(_options.ascii_art)
		{
			asciiOut.open("spec2d/asci_reverse.txt");
		}
		
		try
		{
			uint32_t sync;
			while(block.remaining() >= sizeof(sync))
			{
				block >> sync;

				if(!(sync == Fast2D_Sync))
				{
					g_Log <<"Reached end of file\n";	
					break;
					//g_Log <<"Got (" << sync << ") instead of correct sync uint32_t ("<<Fast2D_Sync<<"), file is corrupt?\n";
					//block.clear();
					//throw std::logic_error("corrupt file, shutting down");
				}
				_particleStartCount++;

				//if(_particleStartCount == 1432547 )
				//{
				//	block.clear();

				//	g_Log << block.size();
				//}

				//now we have N particle slices, followed by 3 blank slices, an ending sync uint32_t and a timing uint64_t
				//slices are 128 bits each, or 16 bytes
				IsEndOfParticle endOfParticle;
				while(!endOfParticle(block,_slices))
				{
					CompressedSlice s;
					block >> s;
					_slices.push_back(s);
				}

			/*	if(_particleStartCount == 1432546)
				{
					size_t head = block.head();
					block.go_to(head-8);
					CompressedSlice s;
					block >> s;
					_slices.push_back(s);
					asciiOut.open("spec2d/asci_EAT.txt");
					g_Log <<"\nslices on exception: " << _slices.size() <<"\n";
					for(int i = 0;i<_slices.size();++i)
					{
						_slices[i].print(asciiOut);
					}
					asciiOut.close();
				}*/
				
				uint64_t time;
				block >> time;
			
				swap_endian_force(reinterpret_cast<byte*>(&time), sizeof(time)); //convert to little endian

				_slices.erase(_slices.end()-3, _slices.end()); //remove the last three blank slices
				while(!_slices.empty())
				{
					write_particle(writer, Timing(time), asciiOut);
				}
				_particleCount++;
				
			}
				
		}
		catch (block_incomplete&)
		{
			/*asciiOut.open("spec2d/asci_reverse.txt");
			g_Log <<"\nslices on exception: " << _slices.size() <<"\n";
			for(int i = 0;i<_slices.size();++i)
			{
				_slices[i].print(asciiOut);
			}*/
			//if hit here, last particle isn't complete so it won't be written
		}
		g_Log <<"Found " << _particleCount <<" particles\n";
		g_Log <<"Started " << _particleStartCount <<" particles\n";

		block.clear();
	}

	template< class Writer>
	void Device2DS_reverse::write_particle(Writer& writer, Timing time, std::ofstream& asciiOut )
	{
		static uint32_t lineCount = 0;
		size_t nSlices = std::min(_slices.size(), size_t(500)); //only 12 bits for num data words
	
		ParticleRecord	pr;
		pr.PacketID = DATA;
		pr.NumSlicesInParticle	= nSlices;
		pr.ParticleCount		= word(_particleCount); //this wraps around, apparently by design
	/*	if(word(pr.ParticleCount) == 12260)
		{
			pr.ParticleCount= 12260;
		}*/

		pr.VerticalImage._Description.bits.HasOverloadTimingWords = 0;
		pr.VerticalImage._Description.bits.TimingWordMismatch = 0;
		pr.VerticalImage._Description.bits.FIFO_Empty = 0;
		pr.VerticalImage._Description.bits.NumDataWords = 0;
		pr.VerticalImage._Description.bits.TimingWordsNotFound =1;

		pr.HorizontalImage._Description.bits.NumDataWords = 2;
		pr.HorizontalImage._time = time;
		pr.HorizontalImage._Description.bits.HasOverloadTimingWords = 1;
		pr.HorizontalImage._Description.bits.TimingWordsNotFound = 0;
		pr.HorizontalImage._Description.bits.FIFO_Empty = 0;
		pr.HorizontalImage._Description.bits.TimingWordMismatch = 0;


		for(size_t i = 0;i<nSlices;++i,lineCount++)
		{
			
			CompressedSlice& s = _slices[i];

			if(_options.ascii_art)
			{
				s.print(asciiOut);
			}
			
			const int nRecords = s.numRecords();
			
			
			if(0 == nRecords)
			{
				pr.HorizontalImage._Description.bits.NumDataWords++;
				ImageChunk chunk;
				chunk.SetMaxClear();
				pr.HorizontalImage._data.push_back(chunk);
			}
			else if(s.all())
			{
				pr.HorizontalImage._Description.bits.NumDataWords++;
				ImageChunk chunk;
				chunk.SetMaxShade();
				pr.HorizontalImage._data.push_back(chunk);
			}
			else
			{
				pr.HorizontalImage._Description.bits.NumDataWords += nRecords;

				word pixelIndex = 0;
				bool startOfSlice = true;
				for(int j = 0;j<nRecords;++j)
				{
					ImageChunk chunk;
					chunk.StartOfSlice =  word(startOfSlice);
					chunk.NumClearPixels = s.countUntilNot(pixelIndex, false);
					chunk.NumShadedPixels = s.countUntilNot(pixelIndex, true);
					chunk.ImageWord = 1;
					pr.HorizontalImage._data.push_back(chunk);
					startOfSlice = false;
				}
			}

			
			//if(pixelIndex < 128)
			//{
			////	_LastSliceIncomplete = true;
			//	ImageChunk chunk;
			//	chunk.StartOfSlice = 0;
			//	chunk.NumClearPixels = 128 - pixelIndex;
			//	chunk.NumShadedPixels = 0;
			//	chunk.ImageWord = 1;
			//	pr.HorizontalImage._Description.bits.NumDataWords++;
			//	if(pixelIndex == 0)
			//	{
			//	//	chunk.NumClearPixels = 64;
			//		//pr.HorizontalImage._data.push_back(chunk);
			//		chunk.SetMaxClear();
			//	}

			//	pr.HorizontalImage._data.push_back(chunk);
			//}

			
		}

		assert(pr.HorizontalImage._data.size() >= nSlices);
	
		std::streamoff sizeBytes = writer.tellp();
		writer << pr;
		std::streamoff NewsizeBytes = writer.tellp();

		if(sizeBytes%SIZE_DATA_BUF < NewsizeBytes%SIZE_DATA_BUF)
		{
			//this tells us which timestamp to use later when reassembling the entire timestamp + block thing
			_time_slots.push_back(_byteReadCount/SIZE_DATA_BUF);
		}
		_byteReadCount += _slices.size()*16 + 8;

		_slices.erase(_slices.begin(), _slices.begin() + nSlices);
	}


	template<class Writer>
	void Device2DS_reverse::Write2DS( Writer& writer) 
	{
		std::ifstream in_slicedata("temp/slicedata.bin",std::ios::binary);

		char	data_block[SIZE_DATA_BUF];

		int nBlocks = 0;
		while(!in_slicedata.bad() && !in_slicedata.eof()/* && nBlocks < _time_stamps.size() */)
		{
			uint32_t slot = _time_slots[nBlocks++];

			TimestampUCAR& time_stamp = _time_stamps[slot];
			memset(data_block,0,SIZE_DATA_BUF);
			in_slicedata.read(data_block,SIZE_DATA_BUF);

			TimeStamp16 ts = time_stamp.As2DSTimeStamp();
			writer << ts;

			//a checksum.. not sure if it means entire file or just the last data block? and is it summed by bytes or shorts?
			word checksum = 0;
			for(int i =0;i<SIZE_DATA_BUF;++i)
			{
				checksum += data_block[i];
			}
			writer.write(&data_block[0], SIZE_DATA_BUF);

			writer.write(reinterpret_cast<char*>(&checksum),sizeof(checksum));
		}
	}
}
