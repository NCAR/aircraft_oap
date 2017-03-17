#pragma once
#include "Packet.h"
#include <vector>
#include <assert.h>

namespace sp
{
	struct ParticleInfo3VCPI
	{
		struct Bits
		{
			word		NumDataWords:12;
			word		ExceededMaxSize:1;
			word		Unused:1;				 //always 0
			word		ParticleTriggered:1;
			word		FIFO_Overflow:1;	
		};
		union
		{
			word All;

			Bits bits;
		};

		bool HasTimingWord() const
		{
			return bits.ExceededMaxSize == 0 || bits.ParticleTriggered == 1;
		}

		bool HasData() const
		{
			return bits.FIFO_Overflow == 0;
		}
	};

	template<class T>
	inline T&	operator >> (T& reader, ParticleInfo3VCPI& in)
	{
		reader.read(in.All);
		return reader;
	};

	template<class T>
	inline T&	operator << (T& writer, ParticleInfo3VCPI& in)
	{
		writer.write(reinterpret_cast<char*>(&in.All),sizeof(in.All));
		return writer;
	};

	struct ImageChunk3VCPI
	{
		word	GetClearCount()const
		{
			const word* all = reinterpret_cast<const word*>(this);
                        if(0x4000 == *all)
                        {
                                return 0;
                        }
                        else if(0x7FFF == *all)
                        {
                                return -1;	// signal uncompressed data
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
                                return -1;	// signal uncompressed data
                        }
			return NumShadedPixels;
		}

		void	SetMaxShade()
		{
			*reinterpret_cast<word*>(this) = 0x4000;
		}

		void	SetMaxClear()
		{
			*reinterpret_cast<word*>(this) = 0x7FFF;
		}
		bool	IsUncompressed() const
		{
			return *reinterpret_cast<const word*>(this) == 0x7FFF;
		}
		bool	IsStartOfSlice() const
		{
			return StartOfSlice == 1;
		}
		void	swapEndian()
		{
			swap_endian_force(reinterpret_cast<byte*>(this),sizeof(ImageChunk));
		}


		word NumClearPixels:7;
		word NumShadedPixels:7;
		word StartOfSlice:1;	//1 for start of slice, 0 for continuation
		word ImageWord:1;	//0 indicates image word..?
	};

	template<class T>
	inline T&	operator << (T& writer, ImageChunk3VCPI& im)
	{
		writer.write(reinterpret_cast<char*>(&im),sizeof(word));
		return writer;
	}
	struct ImageData3VCPI
	{
		ParticleInfo3VCPI _Description;
		typedef std::vector<ImageChunk3VCPI> Data;
		Data _data;
		bool hasData;

		word _TimingWords[3];

		Timing Time()const
		{ return *reinterpret_cast<const Timing*>(&_TimingWords[0]); }
	

		void clear()
		{_data.clear();}

		bool HasData()const
		{ return hasData /*&& _Description.HasData(*/; }
	};

	inline Log&	operator << (Log& log, ImageData3VCPI& out)
	{
		return log;
		//return log << "\nImageWord: " << out.bits.ImageWord <<
		//	"\nStartOfSlice: " << out.bits.StartOfSlice << 
		//	"\nNumShadedPixels: " << out.bits.NumShadedPixels << 
		//	"\nNumClearPixels: " << out.bits.NumClearPixels;
	};

	template<class T>
	inline T&	operator >> (T& reader, ImageData3VCPI& in)
	{
		int WordsToRead = in._Description.bits.NumDataWords;

		if(WordsToRead == 0)
		{
			return reader;
		}

		bool HasTiming = in._Description.HasTimingWord() && WordsToRead >= 3;
		if(HasTiming)
		{
			assert(WordsToRead > 2);
			WordsToRead -= 3;
		}

		if(WordsToRead > 0 && in.HasData())
		{
			in._data.resize(WordsToRead);
			reader.read(reinterpret_cast<byte*>(&in._data[0]),WordsToRead*sizeof(word) );

		}

		if(HasTiming)
		{
			reader.read(reinterpret_cast<byte*>(&in._TimingWords[0]), sizeof(in._TimingWords));
		}

		return reader;
	};

	template<class T>
	inline T&	operator << (T& writer, ImageData3VCPI& im)
	{
		for(size_t i = 0;i<im._data.size();++i)
		{
			writer << im._data[i];
		}
		if(im._Description.HasTimingWord())
		{
			writer.write(reinterpret_cast<byte*>(&im._TimingWords[0]), sizeof(im._TimingWords));
		}
		return writer;
	}


	struct ParticleRecord3VCPI: public Packet
	{
		Word		ParticleCount;
		Word		NumSlicesInParticle;

		ImageData3VCPI	HorizontalImage;
		ImageData3VCPI	VerticalImage;
	
		void clear()
		{HorizontalImage.clear(); VerticalImage.clear();}

		void setData(bool hasData)
		{
			HorizontalImage.hasData = hasData;
			VerticalImage.hasData = hasData;
		}
	};


	template<class T>
	inline T&	operator >> (T& reader, ParticleRecord3VCPI& in)
	{
		reader >> in.HorizontalImage._Description >> in. VerticalImage._Description >>
			in.ParticleCount >> in.NumSlicesInParticle;

		// if(in.HorizontalImage._Description.bits.NumDataWords > 0)
		{
			reader >> in.HorizontalImage;
		}
		// if(in.VerticalImage._Description.bits.NumDataWords > 0)
		{
			reader >> in.VerticalImage;
		}
		return reader;
		//		in.HorizontalImage >> in.VerticalImage;
	};


	template<class T>
	inline T&	operator << (T& writer, ParticleRecord3VCPI& pr)
	{
		writer << pr.PacketID << pr.HorizontalImage._Description  << pr. VerticalImage._Description  <<
			pr.ParticleCount  << pr.NumSlicesInParticle;

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
