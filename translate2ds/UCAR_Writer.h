#pragma once
#include <fstream>
#include <sstream>
#include "2DSParticle.h"
#include "HouseKeeping.h"
#include "MaskData.h"
#include "merge_files.h"
#include <limits>
#include <algorithm>
#include "log.h"
#undef max

namespace sp
{
	//binary write
	template<class T>
	inline std::ofstream& operator ^ (std::ofstream& s, T out)
	{
		 swap_endian_force(reinterpret_cast<byte*>(&out), sizeof(out)); // UCAR wants data in big endian format
		 s.write(reinterpret_cast<const char*>(&out),sizeof(out));
		 return s;
	}

	enum CharacterCodes
	{
		HORIZONTAL_2DS = 0x3248,
		VERTICAL_2DS = 0x3256,
		HORIZONTAL_3VCPI = 0x3348,
		VERTICAL_3VCPI = 0x3356,

		HVPS = 0x5348,
		SPEC_2D_128 = 0x5337
	};

	// http://www.eol.ucar.edu/data/software/pms2d/pms2d-data_format
	class UCAR_Writer
	{
		typedef std::vector<byte>	Buffer;

		struct Channel
		{
			Channel()
			{
				_buffer.reserve(SIZE_DATA_BUF);
				_bytes = 0;
			}

//			std::ofstream	_file;	// moved to outer class, since one output file.
			Buffer		_buffer;
			size_t		_bytes;
		};

		sword			_VerticalCode;
		sword			_HorizontalCode; 
		std::string		_ProbeType;
		std::string		_resolution;
		std::string		_nDiodes;
		std::string		_SuffixH, _SuffixV;


	public:
		UCAR_Writer(const std::string& fileName, const Options& options,
			sword HorizontalCode, sword VerticalCode, 
			const std::string& ProbeType, const std::string& resolution, const std::string& nDiodes,
			const std::string& SuffixH, const std::string& SuffixV) : _options(options)
		{
			_file.open(("temp/" + fileName + ".bin").c_str(), std::ios::binary);
//			_hChannel._file.open(("temp/" + fileName + "H.bin").c_str(),std::ios::binary);
//			_vChannel._file.open(("temp/" + fileName + "V.bin").c_str(),std::ios::binary);

			_FileName = fileName;

			_VerticalCode = VerticalCode;
			_HorizontalCode = HorizontalCode;
			_ProbeType = ProbeType;
			_resolution = resolution;
			_nDiodes = nDiodes;
			_SuffixH = SuffixH;
			_SuffixV = SuffixV;

			memset(&_MostRecentTimeStamp, 1, sizeof(_MostRecentTimeStamp));
		}

		~UCAR_Writer()
		{
			_hImage.WriteRemainder();
			_vImage.WriteRemainder();

			Write(_hChannel, _hImage.Bits(), _HorizontalCode, true);
			Write(_vChannel, _vImage.Bits(), _VerticalCode, true);

			_file.close();
//			_hChannel._file.close();
//			_vChannel._file.close();

			GenerateXMLHeader(	"temp/" + _FileName + ".xml",
						(const char *)&_HorizontalCode, _SuffixH,
						(const char *)&_VerticalCode, _SuffixV);
//			GenerateXMLHeader("temp/" + _FileName + "V.xml",_VerticalCode, _SuffixV.c_str());

			std::string outputdir(_options.OutputDir);
			if (outputdir.size() > 0)
				outputdir += "/";

			MergeFiles("temp/" + _FileName + ".xml","temp/" + _FileName + ".bin", outputdir + _FileName + ".2d");

			if (!_options.ascii_art)
			{
				g_Log <<"  HORIZONTAL PARTICLES FOUND: " << _hImage.NParticlesCompleted() << "\n";
				g_Log <<"  VERTICAL PARTICLES FOUND: " << _vImage.NParticlesCompleted() << "\n";
				g_Log <<"  TOTAL PARTICLES FOUND: " << _hImage.NParticlesCompleted() + _vImage.NParticlesCompleted()<< "\n";
			}
		}

		void GenerateXMLHeader(	const std::string& file,
					const char *hProbID, const std::string& hSuffix,
					const char *vProbID, const std::string& vSuffix)
		{
			std::ofstream xml(file.c_str(),std::ios::binary);

			std::string flightNum = _FileName;
			flightNum.erase(flightNum.begin(), flightNum.begin()+4);

			xml << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n" <<
				"<OAP version=\"" << 1 << "\">\n" << 
				" <Source>ncar.ucar.edu</Source>\n" << 
				" <FormatURL>http://www.eol.ucar.edu/raf/Software/OAPfiles.html</FormatURL>\n" << 
				" <Project>" << _options.Project << "</Project>\n" <<
				" <Platform>" << _options.Platform << "</Platform>\n" << 
				" <FlightNumber>" << _options.FlightNumber << "</FlightNumber>\n" <<
				" <FlightDate>" << _MostRecentTimeStamp.wMonth.toString() << "/" << _MostRecentTimeStamp.wDay.toString() << "/" << _MostRecentTimeStamp.wYear.toString() << "</FlightDate>\n" <<
				" <probe id=\"" << hProbID[1] << hProbID[0] << "\" type=\"" << _ProbeType
						<< "\" resolution=\"" << _resolution
						<< "\" nDiodes=\"" << _nDiodes
						<< "\" serialnumber=\"" << _options.SerialNumber
						<< "\" suffix=\"" << hSuffix << "\"/>\n" <<

				" <probe id=\"" << vProbID[1] << vProbID[0] << "\" type=\"" << _ProbeType
						<< "\" resolution=\"" << _resolution
						<< "\" nDiodes=\"" << _nDiodes
						<< "\" serialnumber=\"" << _options.SerialNumber
						<< "\" suffix=\"" << vSuffix << "\"/>\n";
			xml << "</OAP>\n";
		}


		UCAR_Writer& operator << (TimeStamp16& timeStamp)
		{
			_MostRecentTimeStamp = timeStamp;
			return *this;
		}

		UCAR_Writer& operator << (ParticleRecord& pr)
		{ 
			if (pr.HorizontalImage._Description.bits.NumDataWords > 0)
			{
				StoreParticle(_hChannel, pr.HorizontalImage, _hImage, pr.ParticleCount, _HorizontalCode);
			}
			else if(pr.VerticalImage._Description.bits.NumDataWords > 0)
			{
				StoreParticle(_vChannel, pr.VerticalImage, _vImage, pr.ParticleCount, _VerticalCode);
			}
			return *this;
		}

		UCAR_Writer& operator << (ParticleRecord3VCPI& pr)
		{ 
			if (pr.HorizontalImage._Description.bits.NumDataWords > 0)
			{
				StoreParticle(_hChannel, pr.HorizontalImage, _hImage, pr.ParticleCount, _HorizontalCode);
			}
			else if(pr.VerticalImage._Description.bits.NumDataWords > 0)
			{
				StoreParticle(_vChannel, pr.VerticalImage, _vImage, pr.ParticleCount, _VerticalCode);
			}
			return *this;
		}

		

		UCAR_Writer& operator << (HouseKeeping& hk)
		{
			_MostRecentHouseKeeping = hk;
			return *this;
		}

		UCAR_Writer& operator << (HouseKeeping3VCPI& )
		{
			//don't really have any use for this data
			return *this;
		}

	private:
		//adds the header that matches the PD2 format from UCAR
		bool AddHeaderPD2(Channel& channel, word CharacterCode)
		{
			const TimeStamp16& timeStamp = _MostRecentTimeStamp;
			if(_options.InRange(timeStamp))
			{
				if(_options.ascii_art)
					return true;

				//NOTE: not sure yet what the overLoad value should be set to
				word overLoad = 0;
/*
				std::cout <<  CharacterCode << " " << timeStamp.wHour << ":" << timeStamp.wMinute
					<< ":" <<  timeStamp.wSecond << "." << timeStamp.wMilliseconds
					<< " " <<  timeStamp.wYear << "/" << timeStamp.wMonth
					<< "/" << timeStamp.wDay << std::endl;
*/
				_file	^  CharacterCode ^ timeStamp.wHour ^ timeStamp.wMinute
					^  timeStamp.wSecond ^  timeStamp.wYear ^ timeStamp.wMonth
					^ timeStamp.wDay ^ word(_MostRecentHouseKeeping.TAS)
					^ timeStamp.wMilliseconds ^ overLoad;

				return true;
			}

			return false;
		}

		//to allow for decompression 
		struct ImageSlice
		{
			ImageSlice()
			{
				decompressed.reserve(1000);
				byteCount = 0;
				_slice_pixel_count = 0;
				_nParticlesCompleted = 0;
				numWritten = 0;
				_particleCount = -1;
			}

			template<class BufferImage>
			const Buffer& Decompress(const BufferImage& src, size_t nParticles, const Timing& timing, bool asciiArt )
			{
				if(src.empty())
				{
					return _empty;
				}

				_timing = timing;

				if(asciiArt)
				{
					decompressed.clear();
				}

				bits.clear();

				if(nParticles != _particleCount && !asciiArt)
				{
					NextParticle(nParticles);
				}

				for(typename BufferImage::const_iterator i = src.begin(); i != src.end(); ++i)
				{
					const typename BufferImage::value_type& chunk = *i;
					if (chunk.GetShadedCount() == 128)
					{
						WriteClearEnding();
						decompressed.resize(decompressed.size()+128, 0);
						_slice_pixel_count += 128;
					}
					else
					if (chunk.IsUncompressed())
					{
						WriteClearEnding();
						size_t j, size = 16 / sizeof(word);
						for (j = 0; j < size; ++j)
						{
							if (i+1 == src.end())
								break;
							const typename BufferImage::value_type& chunk = *(++i);
							AddUncompressedChunk(*(word *)&chunk);
						}
						WriteClearEnding();
					}
					else
					{
						AddChunk(chunk);
					}
				}
				WriteClearEnding();

				if(asciiArt)
				{
					std::replace(decompressed.begin(),decompressed.end(),byte(0), byte('x'));
					std::replace(decompressed.begin(),decompressed.end(),byte(1), byte('_'));
					return decompressed;
				}

				//makes bytes into bits
				ToBits();

				return bits;
			}

			void AddUncompressedChunk(const word chunk)
			{
				size_t size = sizeof(word)*8;
				for (size_t i = 0; i < size; ++i)
					decompressed.resize(decompressed.size()+1, ((word)chunk >> i) & 0x01);
				_slice_pixel_count += size;
			}

			void ToBits()
			{
				byte b = 0;
				for (size_t i = 0; i < decompressed.size(); ++i)
				{
					int slide = i % 8;
					b |= decompressed[i] << slide;

					if(slide == 7)
					{
						bits.push_back(b);
						byteCount++;
						b = 0;

					}
				}

				size_t remaining = decompressed.size() % 8;
				decompressed.erase(decompressed.begin(), decompressed.end() - remaining);
			}

			template<class Image>
			void AddChunk( const Image &chunk ) 
			{
				word clearCount = chunk.GetClearCount();
				word shadedCount = chunk.GetShadedCount();

				if (chunk.IsStartOfSlice())
				{
					WriteClearEnding();
				}

				decompressed.resize(decompressed.size()+clearCount,1); //UCAR expects 1 for clear, 0 for shaded
				decompressed.resize(decompressed.size()+shadedCount,0);

				_slice_pixel_count += (clearCount+shadedCount);
			}

			void WriteClearEnding() 
			{
				int remains = 128 - _slice_pixel_count%128;

				_slice_pixel_count = 0;
				if (remains == 128)
				{
					return;
				}
				assert(remains >= 0);
				decompressed.resize(decompressed.size()+size_t(remains), 1); 
			}

			Buffer&	WriteRemainder()
			{
				if (numWritten <= 0)
					return bits;
				WriteClearEnding();
				ToBits();

				assert((byteCount % 16) == 0);

				WriteSyncTimingWord(); //get last timing section
				return bits;
			}

			Buffer&	Bits()
			{
				return bits;
			}
			size_t NParticlesCompleted() const { return _nParticlesCompleted; }

		private:
			void NextParticle(size_t newCount)
			{
				if (numWritten++ > 0)
				{
					WriteClearEnding();
					ToBits();

					assert((byteCount % 16)==0);

					WriteSyncTimingWord();
				}

				assert((byteCount % 16)==0);

				_particleCount = newCount;
			}

			void WriteSyncTimingWord() 
			{
//				WriteBlankSlice();

				uint64_t timingBE = _timing;
				timingBE &= 0x0000FFFFFFFFFFFFLL;
//				swap_endian_force(reinterpret_cast<byte*>(&timingBE), sizeof(timingBE));
				write_buffer(bits, timingBE);

				uint64_t sync = Fast2D_Sync;
//				swap_endian_force(reinterpret_cast<byte*>(&sync), sizeof(sync));
				write_buffer(bits, sync);

				_nParticlesCompleted++;
			}

			void WriteBlankSlice()
			{
				//end of last particle 3 blanks then timing word..(blanks are inverted to all 1's)
				uint64_t blank = 0xFFFFFFFFFFFFFFFFLL;
				write_buffer(bits, blank);
				write_buffer(bits, blank);
			}

			template<class T>
			void write_buffer(Buffer& buf, T& v)
			{
				byte* end = reinterpret_cast<byte*>(&v)+sizeof(v);
				for(byte* b = reinterpret_cast<byte*>(&v); b != end;++b )
				{
					buf.push_back(*b);
				}
			}

			Timing		_timing;
			Buffer		decompressed;
			Buffer		bits;
			Buffer		_empty;
			size_t		byteCount;
			int		_slice_pixel_count;
			public:
			long long	_particleCount;
			size_t		numWritten;
			size_t		_nParticlesCompleted;

		};

		template<class T>
		void StoreParticle(Channel& channel, const T& imd, ImageSlice& slice, size_t particleCount, word CharacterCode)
		{
			if(particleCount < _options.MinParticle || particleCount > _options.MaxParticle)
				return;

			const Buffer& in = slice.Decompress(imd._data, particleCount, imd.Time(), _options.ascii_art);

			Write(channel, in, CharacterCode);
		}

		void Write(Channel &channel, const Buffer &in, word CharacterCode, bool ForceIt = false) 
		{
			Buffer& out = channel._buffer;

			out.insert(out.end(),in.begin(),in.end());

			if(out.size() >= SIZE_DATA_BUF)
			{
				WriteParticle(channel, CharacterCode);
			}

			if(ForceIt && !out.empty())
			{

			//	g_Log <<"Forcing last " <<out.size() <<" bytes\n";
				out.resize(SIZE_DATA_BUF, 0);
				WriteParticle(channel, CharacterCode);
			}
		}

		void WriteParticle(Channel& channel,word CharacterCode)
		{
			if(_options.ascii_art)
			{
				WriteDebugParticle(channel);
				return;
			}

			Buffer& buf = channel._buffer;
			while(buf.size() >= SIZE_DATA_BUF)
			{
				if(AddHeaderPD2(channel,CharacterCode))
				{

					_file.write(reinterpret_cast<const char*>(&buf[0]), SIZE_DATA_BUF);
					buf.erase(buf.begin(),buf.begin() + SIZE_DATA_BUF) ;
				}
				else
				{
					//didn't write so need to erase until the start of current particle
					//so search backward for last SYNC marker
					union Combo
					{
						uint64_t sync;
						char   s[8];
					};
					Combo c;
					c.sync = Fast2D_Sync;
					c.sync=swap_endian(c.sync);
					char*	rbegin	= &c.s[0];
					char*	rend	= rbegin + sizeof(c.s);
					Buffer::reverse_iterator ri = std::search(buf.rbegin(), buf.rend(), rbegin, rend);
					if(ri != buf.rend())
					{
						buf.erase(buf.begin(), (ri.base())-sizeof(uint32_t));
					}

					break;
				}
				
			}
		
	
		}

		void WriteDebugParticle(Channel& channel)
		{
			Buffer& buf = channel._buffer;
			while(buf.size() >= 128)
			{
				_file.write(reinterpret_cast<const char*>(&buf[0]), 128);
				_file << "\n";
				buf.erase(buf.begin(),buf.begin() + 128) ;
			}
		}

		std::string		_FileName;

		HouseKeeping		_MostRecentHouseKeeping;
		TimeStamp16		_MostRecentTimeStamp;	

		Channel			_vChannel, _hChannel;
		ImageSlice		_vImage, _hImage;

		/// Output file pointers, xml header and binary data.
		std::ofstream		_xmlF, _file;

		word			_TAS, _overloadTimeMS;

		const Options&		_options;
	};
}
