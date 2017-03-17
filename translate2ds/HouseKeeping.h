#pragma once
#include "Packet.h"

namespace sp
{


	enum CompressionModes
	{
		COMPRESSION_STERIO = 0,
		COMPRESSION_BOTH_CHANNELS,
		COMPRESSION_HORIZONAL_ONLY,
		COMPRESSION_VERTICAL_ONLY
	};

	struct HeaterBits 
	{
		struct  Bits
		{
			word	Zone0:1;
			word	Zone1:1;
			word	Zone2:1;
			word	Zone3:1;
			word	Zone4:1;
			word	Zone5:1;
			word	Zone6:1;
			word	Zone7:1;
			word	Zone8:1;
			word	Zone9:1;
			word	Zone10:1;
			word	Zone11:1;
			word	Zone12:1;
		};
		union
		{
			word AllBits;
			
			Bits bits;
		};
	
	};

	template<class T>
	inline T&	operator >> (T& reader, HeaterBits& in)
	{
		reader.read(in.AllBits);
		return reader;
	}

	inline Log&	operator << (Log& reader, HeaterBits& out)
	{
		reader << out.AllBits;
		return reader;
	}


//	static_assert(sizeof(HeaterBits) == sizeof(Word), "HeaterBits too big");

	struct Compression
	{
		struct Bits//unamed struct might not be avialable on old gcc so using this
		{
			word				CompressionMode:2;
			word				TimingWordResetPerformed:1;
		};
		union
		{
			word All;
			
			Bits bits; 
		};
	};

	template<class T>
	inline T&	operator >> (T& reader, Compression& in)
	{
		reader.read(in.All);
		return reader;
	}

	inline Log&	operator << (Log& reader, Compression& out)
	{
		reader << out.All;
		return reader;
	}

//	static_assert(sizeof(Compression) == sizeof(Word), "Compression too big");


	struct HouseKeeping: public Packet
	{
			Volts24		HorizontalElement_0_Voltage;
			Volts24		HorizontalElement_64_Voltage;
			Volts24		HorizontalElement_127_Voltage;

			Volts24		VerticalElement_0_Voltage;
			Volts24		VerticalElement_64_Voltage;
			Volts24		VerticalElement_127_Voltage;

			Volts48		RawPositivePowerSupply;
			Volts48		RawNegativePowerSupply;
		
			Celcius		HorizontalArm_Tx_Temp;
			Celcius		HorizontalArm_Rx_Temp;

			Celcius		VerticalArm_Tx_Temp;
			Celcius		VerticalArm_Rx_Temp;

			Celcius		Horizontal_Tip_Tx_Temp;
			Celcius		Horizontal_Tip_Rx_Temp;

			Celcius		RearOpticalBridgeTemp;
			Celcius		DSPBoardTemp;
			Celcius		ForwardVesselTemp;
			Celcius		HorizontalLaserTemp;
			Celcius		VerticalLaserTemp;
			Celcius		FrontPlateTemp;
			Celcius		PowerSupplyTemp;

			Volts48		Negative_5V_Supply;
			Volts48		Positive_5V_Supply;
				
			PSI			CanInternalPressure;	//psi

			Volts24		H_Element_21_Voltage;
			Volts24		H_Element_42_Voltage;
			Volts24		H_Element_85_Voltage;
			Volts24		H_Element_106_Voltage;

			Volts24		V_Element_21_Voltage;
			Volts24		V_Element_42_Voltage;
			Volts24		V_Element_85_Voltage;
			Volts24		V_Element_106_Voltage;

			Word		VerticalParticlesDetected;
			Word		HorizontalParticlesDetected;

			HeaterBits	HeaterOuputs;
		
			Volts12		H_Laser_Drive;
			Volts12		V_Laser_Drive;

			Word		H_Masked_Bits;
			Word		V_Masked_Bits;

			Word		NumStereoParticlesFound;
			Word		NumTimingWordMismatches;
			Word		NumSliceCountMismatches;

			Word		NumHorizontalOverloadPeriods;
			Word		NumVerticalOverloadPeriods;

			Compression	CompressionConfig;

			Word		NumberOfEmptyFIFOfaults;

			Word		Spare2;
			Word		Spare3;

			float32		TAS;//true air speed

			Timing		Timing_;		

	};



	template<class T>
	inline T&	operator >> (T& reader, HouseKeeping& in)
	{
		in.TAS = 0.0f;
		Word TAS1, TAS2;
		 reader >>	in.HorizontalElement_0_Voltage >> in.HorizontalElement_64_Voltage >> in.HorizontalElement_127_Voltage >>
			in.VerticalElement_0_Voltage >> in.VerticalElement_64_Voltage >> in.VerticalElement_127_Voltage >>
			in.RawPositivePowerSupply >> in.RawNegativePowerSupply >> 
			in.HorizontalArm_Tx_Temp >> in.HorizontalArm_Rx_Temp >>
			in.VerticalArm_Tx_Temp >> in.VerticalArm_Rx_Temp >> 
			in.Horizontal_Tip_Tx_Temp >> in.Horizontal_Tip_Rx_Temp >>
			in.RearOpticalBridgeTemp >> in.DSPBoardTemp >>
			in.ForwardVesselTemp >> in.HorizontalLaserTemp >> 
			in.VerticalLaserTemp >> in.FrontPlateTemp >> in.PowerSupplyTemp >>
			in.Negative_5V_Supply >> in.Positive_5V_Supply >> 
			in.CanInternalPressure >> 
			in.H_Element_21_Voltage >>	in.H_Element_42_Voltage >> in.H_Element_85_Voltage >> in.H_Element_106_Voltage >>
			in.V_Element_21_Voltage >> in.V_Element_42_Voltage >> in.V_Element_85_Voltage >> in.V_Element_106_Voltage >> 
			in.VerticalParticlesDetected >> in.HorizontalParticlesDetected >> 
			in.HeaterOuputs >> 
			in.H_Laser_Drive >> in.V_Laser_Drive >> 
			in.H_Masked_Bits >> in.V_Masked_Bits >> 
			in.NumStereoParticlesFound >> 	in.NumTimingWordMismatches >>	in.NumSliceCountMismatches >>
			in.NumHorizontalOverloadPeriods >> in.NumVerticalOverloadPeriods >>
			in.CompressionConfig >>
			in.NumberOfEmptyFIFOfaults >> 
			in.Spare2 >> in.Spare3 >> 
			TAS1 >> TAS2>>
			in.Timing_;

		// uint32_t rar = TAS1 + (uint32_t(TAS2) << 16);
		 uint32_t rar2 = TAS2 + (uint32_t(TAS1) << 16);
		// float t1 = *(float*)&rar;
		 float t2 = *(float*)&rar2;

		 in.TAS = *(float*)&rar2;

		 //swap_endian_force((byte*)&TAS1,2);
		 //swap_endian_force((byte*)&TAS2,2);

		 //uint32_t rar3 = TAS1 + (uint32_t(TAS2) << 16);
		 //uint32_t rar4 = TAS2 + (uint32_t(TAS1) << 16);
		 //float t3 = *(float*)&rar3;
		 //float t4 = *(float*)&rar4;

		 //swap_endian(reader, in.TAS);
		 return reader;
	};

	/*inline Log&	operator << (Log& log, HouseKeeping& out)
	{
		return log <<	"Horizontal Element 0 Voltage:\t" << out.HorizontalElement_0_Voltage <<
			"\nHorizontal Element 64 Voltage:\t" << out.HorizontalElement_64_Voltage << 
			"\nHorizontalElement_127_Voltage:\t" << out.HorizontalElement_127_Voltage <<
			"\nVerticalElement_0_Voltage:\t" << out.VerticalElement_0_Voltage <<
			"\nVerticalElement_64_Voltage:\t" << out.VerticalElement_64_Voltage <<
			"\nVerticalElement_127_Voltage:\t" << out.VerticalElement_127_Voltage <<
			"\nRawPositivePowerSupply:\t" << out.RawPositivePowerSupply << 
			"\nRawNegativePowerSupply:\t" << out.RawNegativePowerSupply << 
			"\nHorizontalArm_Tx_Temp:\t" << out.HorizontalArm_Tx_Temp <<
			"\nHorizontalArm_Rx_Temp:\t" << out.HorizontalArm_Rx_Temp <<
			"\nVerticalArm_Tx_Temp:\t" << out.VerticalArm_Tx_Temp <<
			"\nVerticalArm_Rx_Temp:\t" << out.VerticalArm_Rx_Temp << 
			"\nHorizontal_Tip_Tx_Temp:\t" << out.Horizontal_Tip_Tx_Temp <<
			"\nHorizontal_Tip_Rx_Temp:\t" << out.Horizontal_Tip_Rx_Temp <<
			"\nRearOpticalBridgeTemp:\t" << out.RearOpticalBridgeTemp <<
			"\nDSPBoardTemp:\t" << out.DSPBoardTemp <<
			"\nForwardVesselTemp:\t" << out.ForwardVesselTemp <<
			"\nHorizontalLaserTemp:\t" << out.HorizontalLaserTemp << 
			"\nVerticalLaserTemp:\t" << out.VerticalLaserTemp <<
			"\nFrontPlateTemp:\t" << out.FrontPlateTemp <<
			"\nPowerSupplyTemp:\t" << out.PowerSupplyTemp <<
			"\nNegative_5V_Supply:\t" << out.Negative_5V_Supply <<
			"\nPositive_5V_Supply:\t" << out.Positive_5V_Supply << 
			"\nCanInternalPressure:\t" << out.CanInternalPressure << 
			"\nH_Element_21_Voltage:\t" << out.H_Element_21_Voltage <<
			"\nH_Element_42_Voltage:\t" << out.H_Element_42_Voltage <<
			"\nH_Element_85_Voltage:\t" << out.H_Element_85_Voltage <<
			"\nH_Element_106_Voltage:\t" << out.H_Element_106_Voltage <<
			"\nV_Element_21_Voltage:\t" << out.V_Element_21_Voltage <<
			"\nV_Element_42_Voltage:\t" << out.V_Element_42_Voltage <<
			"\nV_Element_85_Voltage:\t" << out.V_Element_85_Voltage << 
			"\nV_Element_106_Voltage:\t" << out.V_Element_106_Voltage << 
			"\nVerticalParticlesDetected:\t" << out.VerticalParticlesDetected << 
			"\nHorizontalParticlesDetected:\t" << out.HorizontalParticlesDetected << 
			"\nHeaterOuputs:\t" << out.HeaterOuputs << 
			"\nH_Laser_Drive:\t" << out.H_Laser_Drive <<
			"\nV_Laser_Drive:\t" << out.V_Laser_Drive << 
			"\nH_Masked_Bits:\t" << out.H_Masked_Bits <<
			"\nV_Masked_Bits:\t" << out.V_Masked_Bits << 
			"\nNumStereoParticlesFound:\t" << out.NumStereoParticlesFound <<
			"\nNumTimingWordMismatches:\t" << out.NumTimingWordMismatches <<
			"\nNumSliceCountMismatches:\t" << out.NumSliceCountMismatches <<
			"\nNumHorizontalOverloadPeriods:\t" << out.NumHorizontalOverloadPeriods <<
			"\nNumVerticalOverloadPeriods:\t" << out.NumVerticalOverloadPeriods <<
			"\nCompressionConfig:\t" << out.CompressionConfig <<
			"\nNumberOfEmptyFIFOfaults:\t" << out.NumberOfEmptyFIFOfaults << 
			"\nSpare2:\t" << out.Spare2 <<
			"\nSpare3:\t" << out.Spare3 << 
			"\nTAS:\t" << out.TAS << 
			"\nTiming:\t" << out.Timing_;

	};
*/
	//static_assert(sizeof(HouseKeeping) == 54*sizeof(Word), "Housekeeping wrong size");


}
