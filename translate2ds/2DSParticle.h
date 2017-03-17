#pragma once
#include "Packet.h"
#include <vector>
#include <assert.h>

namespace sp
{
	struct ParticleInfo
	{
		struct Bits
		{
			word	NumDataWords:12;
			word	TimingWordsNotFound:1; //bit 12
			word	TimingWordMismatch:1;  //only seems to be used on the vertical part..?
			word	FIFO_Empty:1;	
			word	HasOverloadTimingWords:1; //indicates that last two words of data record are timing words
		};
		union
		{
			word All;
	
			Bits bits;
		};

		bool HasTimingWord()const{return bits.TimingWordsNotFound == 0;}//bits.HasOverloadTimingWords == 1;}
	};

	template<class T>
	inline T&	operator >> (T& reader, ParticleInfo& in)
	{
		reader.read(in.All);
		return reader;
	};

	template<class T>
	inline T&	operator << (T& writer, ParticleInfo& in)
	{
		 writer.write(reinterpret_cast<char*>(&in.All),sizeof(in.All));
		 return writer;
	};

	inline Log&	operator << (Log& log, ParticleInfo& out)
	{
		return log << "\nNumDataWords: " << out.bits.NumDataWords <<
			"\nTimingWordsNotFound: " << out.bits.TimingWordsNotFound;
	};

	struct ImageChunk
	{
		word	GetClearCount()const
		{
			const word* all = reinterpret_cast<const word*>(this);
			if(0x7FFF == *all)
			{
				return 128;
			}
			else if(0x4000 == *all)
			{
				return 0;
			}
			return NumClearPixels;
		}

		word	GetShadedCount()const
		{
			const word* all = reinterpret_cast<const word*>(this);
			if(0x4000 == *all)
			{
				return 128;
			}
			else if(0x7FFF == *all)
			{
				return 0;
			}
			return NumShadedPixels;
		}

		void	SetMaxShade()
		{ *reinterpret_cast<word*>(this) = 0x4000; }

		void	SetMaxClear()
		{ *reinterpret_cast<word*>(this) = 0x7FFF; }

                bool    IsUncompressed() const
                { return *reinterpret_cast<const word*>(this) == 0x7FFF; }

		bool	IsStartOfSlice() const
		{ return StartOfSlice == 1; }

		void	swapEndian()
		{ swap_endian_force(reinterpret_cast<byte*>(this),sizeof(ImageChunk)); }


		word NumClearPixels:7;
		word NumShadedPixels:7;
		word StartOfSlice:1;  //1 for start of slice, 0 for continuation
		word ImageWord:1;	   //0 indicates image word..?
	};

	template<class T>
	inline T& operator << (T& writer, ImageChunk& im)
	{
		writer.write(reinterpret_cast<char*>(&im),sizeof(word));
		return writer;
	}
	struct ImageData
	{
		ParticleInfo _Description;
		typedef std::vector<ImageChunk>	Data;
		Data	_data;
		Timing	_time;

		void clear() { _data.clear(); }

		Timing Time() const
		{
			return _time;
		}
	};

	inline Log& operator << (Log& log, ImageData& out)
	{
		return log;
		//return log << "\nImageWord: " << out.bits.ImageWord <<
		//	"\nStartOfSlice: " << out.bits.StartOfSlice << 
		//	"\nNumShadedPixels: " << out.bits.NumShadedPixels << 
		//	"\nNumClearPixels: " << out.bits.NumClearPixels;
	};

	template<class T>
	inline T& operator >> (T& reader, ImageData& in)
	{
		int WordsToRead = in._Description.bits.NumDataWords;

		if(WordsToRead == 0)
		{
			return reader;
		}
		

		bool HasTiming = in._Description.HasTimingWord();
		if(HasTiming)
		{
			assert(WordsToRead >1);
			WordsToRead -= 2;
		}

		if(WordsToRead > 0)
		{
			in._data.resize(WordsToRead);
			reader.read(reinterpret_cast<byte*>(&in._data[0]),WordsToRead*sizeof(word) );
		}

		if(HasTiming)
		{
			reader >> in._time;
		}

		return reader;
	};

	template<class T>
	inline T& operator << (T& writer, ImageData& im)
	{
		for(size_t i = 0;i<im._data.size();++i)
		{
			writer << im._data[i];
		}
		if(im._Description.HasTimingWord())
		{
			writer << im._time;
		}
		return writer;
	}
	

	struct ParticleRecord: public Packet
	{
		Word		ParticleCount;
		Word		NumSlicesInParticle;

		ImageData	HorizontalImage;
		ImageData	VerticalImage;

		void		clear(){HorizontalImage.clear(); VerticalImage.clear();}
	};


	template<class T>
	inline T& operator >> (T& reader, ParticleRecord& in)
	{
		reader >> in.HorizontalImage._Description >> in. VerticalImage._Description >>
					in.ParticleCount >> in.NumSlicesInParticle;

//		if(in.HorizontalImage._Description.bits.NumDataWords > 0)
		{
			reader >> in.HorizontalImage;
		}
//		if(in.VerticalImage._Description.bits.NumDataWords > 0)
		{
			reader >> in.VerticalImage;
		}
		return reader;
//		in.HorizontalImage >> in.VerticalImage;
	};


	template<class T>
	inline T& operator << (T& writer, ParticleRecord& pr)
	{
		writer	<< pr.PacketID << pr.HorizontalImage._Description 
			<< pr. VerticalImage._Description
			<< pr.ParticleCount  << pr.NumSlicesInParticle;

		writer << pr.HorizontalImage;
		writer << pr.VerticalImage;
	
		return writer;
	};


	//inline Log&	operator << (Log& log, ParticleRecord& out)
	//{
	//	return log <<	"\n\n***Horizontal Image***:" << out.HorizontalImage._Description <<
	//		"\n\n***Vertical Image***:"  << out.VerticalImage._Description  << 
	//		"\nHParticleCount:\t" << out.ParticleCount <<
	//		"\nNumSlicesInParticle:\t" << out.NumSlicesInParticle <<
	//		"\n\n***Horizontal Image***:" <<out.HorizontalImage <<
	//		"\n\n***Vertical Image***:" <<out.VerticalImage;
	//		

	//};

}
