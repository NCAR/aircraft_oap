/*
header.h
Defines the new ads tape header.

03/04/92 Version 2.1
         Modified to include the Vme interface based Pms 1d, the uv hygrometer, 
         and the ophir radiometer structures. jvp.

08/28/92 Version 2.2
         Added a Serial struct for generic serial blocks.  Added a serial_num 
         field to the Pms1V2 and Uvhyg structs.  Changed the ps_gate field
         of Pms1V2 struct to a float type. version 2.2, jvp

01/19/93 Added GreyParticle and GreySlice structs for PMS 2d Grey scale
         data blocks. jvp.

04/16/93 Reversed pressure and temp fields in Uv_smp, jvp.

08/25/93 Version 3.0 
         Added dsm location fields to item descriptors. Created dsm
         location strings.  
         Added ch_gain and ch_offset fields to the Sh struct.
         Changed strnam data types to str8 types.
         Added locn fields to Hdr and Uvhyg structs. jvp.

07/25/94 Added RATE_1000 and RATE_5000 #defines.
	 Added "C", and "O" #defines for counter and digital output variables.

08/19/94 Added cnvrt_offset field to Sh struct, and ads_type field to the
         Fl struct.  Added ADS_TYPE_1_STR and ADS_TYPE_2_STR #defines.

08/22/94 Added RATE_2000 #define.

02/14/95 Version should have been uped to 3.1 on 8/19, doing it now.

09/13/95 Version 3.2
	Added IrsV2_blk for acquiring 3 extra variables from HwIrs.

02/05/97 Version 3.3
	Added PMS2DV2, with housekeeping.  CJW

11/15/97 Added Mcr block  MPS

06/19/01 Version 3.4
	PMS2D now has original sync word.

08/09/01 Version 3.5
	Added 'rate' to the Blk struct

*/

#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>

#define VERSION_STR	"3.5"

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif
 
#define LEN_12     	12            	/* 12 character string length */
#define LEN_8      	8             	/* 8 character string length */
#define LEN_4      	4             	/* 4 character string length */
#define LEN_2      	2             	/* 2 character string length */
 
#define MX_COF     	10            	/* max # of cal coefficients */
#define MX_PHYS    	65535        	/* max physical record size, 32768 */
#define MX_DME	   	4		/* max number of dme channels */
#define MX_EVT	   	8	       	/* max number event blocks */
#define MX_GREY	   	2		/* max number of grey scale probes */
#define MX_HVPS    2                    /* max number of hvps probes */
#define MX_HSKP    	30             	/* max number houskeeping channels */
#define MX_IRS	   	2	      	/* max number irs blocks */
#define MX_P1D     	4             	/* max number old intfc pms 1d probes */
#define MX_P1DV2   	3              	/* max # vme pms 1d probes per intfc */
#define MX_P2D     	4	      	/* max number pms 2d probes sampled */
#define MX_SDI     	150            	/* max number sampled data channels*/
#define MX_UV_COF  	5		/* max number of UV hygrometer coefs */
#define MAX_PMS1        6               /* max num pms 1d chans per dsm */
#define MAX_PMS2        4               /* max num pms 2d chans per dsm */
#define MAX_AVAPS	4		/* Max DropSondes.		*/
 
#define P1DV2_AUX  	12		/* # of version 2 pms 1d aux chans */
#define P1DV2_COF  	3		/* # of version 2 housekeeping coefs */
#define P1DV3_AUX  	8		/* # of version 3 pms 1d aux chans */
#define P1D_AUX    	4   		/* number of PMS 1D aux channels */
#define P1D_16     	16              /* PMS 1D 16 bin probe bin count */
#define P1D_260    	63              /* PMS 1D 260X probe bin count */
#define P1D_300    	32              /* PMS 1D F300 probe bin count */

#define BINS_16	   	16		/* ver 2 pms 1d 16 bin array size */
#define BINS_32	   	32		/* ver 2 pms 1d 32 bin array size */
#define BINS_40	   	40		/* ver 3 pms 1d 40 bin array size */
#define BINS_64	   	64		/* ver 2 pms 1d 64 bin array size */
			 
#define GPS_SATS   	4               /* number of gps satellites */

#define LRN_REC    	2            	/* Loran record id char length */
#define LRN_DATA   	21           	/* Loran data record array size */

#define P2DLRPR    	7             	/* # pms 2d log recs per phys rec */
#define P2D_DATA   	4096           	/* PMS 2D image buffer array size */
#define P2D_HOUSE	10		/* # PMS 2D housekeeping data */
#define P2D_HOUSE_RATE	5		/* # PMS 2D housekeeping data */
#define GREY_BYTES_PER_SLICE	16    	/* grey scale bytes per image slice */
#define GREY_END_OF_RECORD	-1	/* slice count of -1 indicates EOR */
#define GREY_HOUSE	2		/* # PMS Grey housekeeping data */

#define MCR_CHAN_DATA   5000		/* Mcr high rate data per channel */
#define IRS_HIGH_RATE   50		/* No. pitch, roll samples per sec. */

#define RATE_5000  	5000           	/* 5000 hz sample array size */
#define RATE_2000  	2000           	/* 1000 hz sample array size */
#define RATE_1000  	1000           	/* 1000 hz sample array size */
#define RATE_250   	250           	/* 250 hz sample array size */
#define RATE_55    	55	       	/* 50 hz sample array size */
#define RATE_50    	50	       	/* 50 hz sample array size */
#define RATE_25    	25	       	/* 25 hz sample array size */
#define RATE_10    	10	       	/* 10 hz sample array size */
#define RATE_5     	5       	/* 5 hz sample array size */
#define RATE_2     	2       	/* 2 hz sample array size */
#define RATE_1     	1             	/* 1 hz sample array size */
 
typedef char str12[LEN_12];            	/* 12 character string type */
typedef char str8[LEN_8];              	/* 8 character string type */
typedef char str4[LEN_4];              	/* 4 character string type */
typedef char str2[LEN_2];              	/* 2 character string type */
typedef char strnam[LEN_8];             /* variable name string type */


#define FIRST_REC_STRING 	"ADS DATA TAPE"	/* first tape record string */  
#define FIRST_REC_SIZE 		20	 	/* size of first tape record */

#define FIRST_XML_STRING	"<?xml version=\"1.0\" standalone=\"no\" ?>"


/* Define the aircraft data system type strings. */
#define ADS_TYPE_1_STR	"ADS"		/* original ADS system */
#define ADS_TYPE_2_STR	"DSM"		/* distributed sampling module system */

/* Define the identifying strings for the start of each descriptor type. */
#define AEROSOL_STR    	"AEROSOL"
#define ADC_AOA_STR	"ADCAOA"
#define ASYNC_STR    	"ASYNC"
#define AVAPS_STR    	"AVAPS"
#define CLIMET_STR   	"CLIMET"
#define CMIGITS3_STR	"CMGTS3"
#define DIGOUT_STR   	"DIGOUT"		/* old ads only */
#define DME_STR      	"DME"
#define DPRES_STR	"DPRES"
#define EVNT_STR     	"EVNT"
#define GPS_MAG_STR  	"GPSM"
#define GPS_TRIM_STR 	"GPS"
#define HW_GPS_STR	"HWGPS"
#define GPS_TANS2_STR 	"GPST2"
#define GPS_TANS3_STR 	"GPST3"
#define GPS_GARMIN_STR  "GARMIN"
#define GREYVMEH_STR    "GREYVMEH"
#define PPS_GPS_STR	"PPSGPS"
#define HDR_STR      	"HDR"
#define HSKP_STR     	"HSKP"			/* old ads only */
#define INS_STR      	"INS"
#define IRS_STR      	"IRS"
#define JPLTDL_STR	"JPLTDL"
#define H2OTDL_STR	"H2OTDL"
#define LHTDL_STR	"LHTDL"
#define MCA_STR         "MCA"
#define MASP_STR	"MASP"
#define MASSPEC_STR	"MASSPEC"
#define MCR_STR		"MCR"
#define NEPH_STR	"NEPH"			/* UWA Only	*/
#define NEPH903_STR	"NEPH903"		/* M903		*/
#define NOAA_STR	"NOAA"
#define NONOY_STR	"NONOY"
#define LRNC_STR     	"LRNC"
#define OPHIR3_STR	"OPHIR3"
#define OZONE_STR	"OZONE"
#define PMS1D_STR    	"PMS1D"
#define PMS1V2_STR    	"PMS1V2"
#define PMS1V3_STR	"PMS1V3"
#define PMS2D_STR    	"PMS2D"
#define PMS2DH_STR    	"PMS2DH"
#define RDMA_STR	"RDMA"
#define SDI_STR      	"SDI"
#define SER_STR      	"SER"
#define TAPEHDR_STR  	"THDR"
#define UVHYG_STR	"UVHYG"

/* Define probe name strings. */

#define ANALOG_STR   	"A"		/* analog input sdi type string */
#define ANALOG_AUX_STR 	"X"		/* auxiliary analog input sdi type */
#define DIGITAL_STR 	"D"		/* digital input sdi type string */
#define COUNTER_STR	"C"		/* counter input sdi type string */
#define DIGITAL_OUT_STR	"O"		/* digital output sdi type string */
#define SYNCHRO_STR	"S"		/* synchro input sdi type string */

#define PCAS_STR     	"PCAS"
#define FSSP_STR     	"FSSP"
#define F300_STR     	"F300"
#define X200_STR     	"200X"
#define Y200_STR     	"200Y"
#define X260_STR     	"260X"
#define ASAS_STR     	"ASAS"
#define SPP100_STR     	"S100"
#define SPP200_STR     	"S200"
#define SPP300_STR     	"S300"

#define P2D_C_STR	"2D-C"		/* Pms 2d 'C' probe name */
#define P2D_P_STR	"2D-P"		/* Pms 2d 'P' probe name */
#define P2D_G_STR	"2D-G"		/* Pms 2d grey scale probe name */
#define P2D_H_STR       "2D-H"          /* Spec hvps probe name */

#define SAMPLED_ID	0x8681		/* sampled data id word */
#define C1_STR          "C1"    	/* pms 2d 'C' probe #1 data id string */
#define P1_STR          "P1"    	/* pms 2d 'P' probe #1 data id string */
#define C2_STR          "C2"    	/* pms 2d 'C' probe #2 data id string */
#define P2_STR          "P2"    	/* pms 2d 'P' probe #2 data id string */
#define G1_STR          "G1"    	/* grey scale probe #1 data id string */
#define G2_STR          "G2"    	/* grey scale probe #2 data id string */
#define H1_STR          "H1"            /* hvps probe #1 data id string */
#define H2_STR          "H2"            /* hvps probe #2 data id string */

#define ASAS		0
#define FSSP		1
#define X200    	2
#define Y200    	3
#define X260    	4
#define F300    	5
#define MASP    	6
#define S100    	7
#define S200    	8
#define S300    	9
#define RDMA		10
#define NEPH		11

#define M1_STR		"MCR-1"		/* Mcr channel 1 */
#define M2_STR		"MCR-2"		/* Mcr channel 2 */
#define M3_STR		"MCR-3"		/* Mcr channel 3 */
#define M4_STR		"MCR-4"		/* Mcr channel 4 */
#define M5_STR		"MCR-5"		/* Mcr channel 5 */
#define M6_STR		"MCR-6"		/* Mcr channel 6 */
#define M7_STR		"MCR-7"		/* Mcr channel 7 */

#define SER_NONE 	0			/* serial port no parity */
#define SER_ODD  	1			/* serial port odd parity */
#define SER_EVEN 	2			/* serial port even parity */

#define BYTE_TYPE  	0		/* block consists of byte data */
#define SHORT_TYPE 	1		/* block consists of short data */
#define LONG_TYPE  	2		/* block consists of long data */
#define FLOAT_TYPE 	3		/* block consists of float data */
#define ASCII_TYPE 	4		/* block consists of ascii data */

#define OPCCHAN		40		/* SPP-100 40 channels max */

/* Distributed sampling module location strings. */

#define DSM_CTL_STR	"CTL"		/* dsm control computer */
#define DSM_1_STR	"AFT"		/* aft cabin */
#define DSM_2_STR	"FORWARD"	/* forward cabin */
#define DSM_3_STR	"PODLW"		/* left wing pod */
#define DSM_4_STR	"PODRW"		/* right wing pod */
#define DSM_5_STR	"LEFT"		/* left cabin */
#define DSM_6_STR	"RIGHT"		/* right cabin */
#define DSM_9_STR	"TOP"		/* top in rack */
#define DSM_10_STR	"BOTTOM"	/* bottom in rack */


/* Flight descriptor.  Each tape header always starts with this structure */
 
struct Fl {
  str8 thdr;          			/* I.D. "THDR" */
  int32_t item_len;			/* Length of this struct in bytes */
  str8 version;       			/* header version number */
  str8 prnum;         			/* project number */
  str8 fltnum;        			/* flight number */
  str8 tpnum;         			/* tape number */
  str12 date;         			/* "mm/dd/yyyy */
  str12 time;         			/* "hh:mm:ss" */
  str4 tzone;         			/* i.e. "MST" */
  str8 acraft;        			/* i.e. "N308D" */
  int32_t n_items;			/* # of items excluding this one */
  int32_t lrlen;			/* logical record length */
  int32_t lrppr;			/* logical records per physical rec */
  int32_t thdrlen;			/* length of this tape header */
  str8 ads_type;			/* data system type */
};
typedef struct Fl Fl;

/* Synchronous block descriptors for defining blocks that exist within the 
   1 second sampled data records.
*/
 
/* General structure to declare simple blocks such as the HDR block, and
   those read in via the isio such as the GPS and LRNC.
   To date, these block types go here:
   HDR, GPS, LRNC, GPSM, CLIMET, OPHIR3.
*/
 
struct Blk {
  str8 item_type;   			/* 'HDR','GPS','LRNC','JPLTDL','CLIMET' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  str8 locn;    			/* sensor location */
  str8 dsm_locn;			/* sampling module location */
  int32_t rate;        			/* sample rate			*/
};
typedef struct Blk Blk ;


/* Dme sampling capability is not included in version 3.0 and above. */
struct Dme {
  str8 item_type;   			/* block name 'DME' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  int32_t rate;				/* sample rate */
  int32_t rdadr1;			/* 1st dme read channel address */
  int32_t rdadr2;			/* 2nd dme read channel address */
  int32_t wradr;			/* dme write channel address */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Dme Dme;
 
/* Event blocks are identified by the aircraft locations of their source. 
   These are defined as 'ckp'-cockpit, 'fwd'-forward cabin, 'ctl'-ads station, 
   'rcl'-rear cabin left, 'rcr'-rear cabin right.  Additional stations up to
   MX_EVT may be defined.  Any changes to these names must also be made to
   the netconfig file onboard the aircraft centralf host computer.

   In version 3.0 and above, events are sent to and recorded directly by the
   ads 'ctl' station. Therefore, no rms location is defined.
*/

struct Evt	 {
  str8 item_type;   			/* block name 'EVNT' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  str8 locn;    			/*ckp,fwd,ctl,obl,ibl,obr,ibr*/ 
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Evt Evt;

/* Litton LTN-51 descriptor. 
   LTN-51 sampling capability is not included in version 3.0 and above.
*/

struct Ins {
  str8 item_type;   			/* block name 'INS' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  int32_t rate;				/* sample rate = 250 */
  int32_t adsadr1;			/* 1st ins word channel address */
  int32_t adsadr2;			/* 2nd ins word channel address */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Ins Ins;

/* IRS blocks are identified by the aircraft location: 
	pitl -- pit left
        pitr -- pit right
        cab -- cabin
*/

struct Irs {
  str8 item_type;   			/* block name 'IRS' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  str8 locn;	       			/* acrft locn, */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Irs Irs;

/* Pms1 probe names will be the 1st 4 characters to identify the probe
   type. The 'locn' field will contain the proble location.
   Currently defined probe locations:

   IBL					inboard left
   IBR					inboard right
   OBL					outboard left 
   OBR					outboard right
   OTH					other
*/
 
/* ADS bus pms 1d interface sampling capability is not included in version 3.0
   and above. */
struct Pms1 {
  str8 item_type;   			/* block name 'PMS1D' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  int32_t rate;				/* sample rate */
  int32_t adsaddr;			/* probe base channel address */
  int32_t auxaddr;			/* aux channel address */
  str8 name;        			/* probe name, FSSP, 200X,... */
  str8 locn;	       			/* probe location */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Pms1 Pms1;

/* Vme interface pms 1d header descriptor structure.  This structure is used
   to define all probes which are sampled using the VME bus based pms 1d
   interface board. 
*/

struct Pms1v2 {
  str8 item_type;                     	/* descriptor name 'PMS1V2' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  int32_t rate;				/* sample rate */
  int32_t ps_start;			/* part spacing blk start, 0 = none */
  int32_t ps_length;			/* part spacing blk length, 0 = none */
  float ps_gate;                        /* spacing gate size, 8 to .125 uS */
  int32_t intf_num;			/* interface number, 0 or 1	*/
  int32_t intf_chan;			/* interface channel 0, 1, or 2	*/
  str8 name;                         	/* probe name, FSSP, 200X,...	*/
  str8 hsk_name[P1DV2_AUX];           	/* housekeeping parameter names	*/
  float hsk_cof[P1DV2_AUX][P1DV2_COF];  /* houskeeping cal coefs array */
  str8 locn;                          	/* probe location */
  str8 serial_num;			/* probe serial number */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Pms1v2 Pms1v2;

struct Pms1v3 {
  str8 item_type;                       /* descriptor name 'PMS1V3' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  int32_t rate;				/* sample rate */
  int32_t intf_chan;			/* interface channel 0, 1, or 2 */
  str8 name;                            /* probe name, SPP100, SPP200X,...   */
  int32_t threshold;			/* trigger threshold */
  int32_t transRej;			/* Avg. Transit Reject Flag */
  int32_t bins;				/* 10, 20, 30, or 40 */
  int32_t dofRej;			/* DOF Reject flag */
  int32_t avgTransWt;			/* calc. avg. trans t0 -2 power */
  int32_t attAccept;			/* % trans. time avg. for accept */
  int32_t divisorFlag;			/* 0 = /2, 1 = /4 */
  int32_t ct_method;			/* 1 = sum/peak */ 
  str8 locn;                            /* probe location */
  str8 serial_num;                      /* probe serial number */
  str8 dsm_locn;                        /* sampling module location */
  int32_t defaultRange;
  uint16_t OPCthreshold[OPCCHAN];
  int32_t max_width;			/* max width in A/D counts */
};
typedef struct Pms1v3 Pms1v3;


struct Masp {
  str8 item_type;                     	/* descriptor name 'MASP' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;			/* byte address in the data record */
  int32_t length;			/* block length in bytes */
  int32_t rate;				/* sample rate		*/
  str8 hsk_name[P1DV2_AUX];           	/* housekeeping parameter names	*/
  float hsk_cof[P1DV2_AUX][P1DV2_COF];  /* houskeeping cal coefs array */
  str8 locn;                          	/* probe location */
  str8 serial_num;			/* probe serial number */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Masp Masp;

/* Generic serial blocks.  Used to define a block for an instrument which
   provides a serial interface which is not otherwise defined, i.e. a user
   instrument which is used only for a specific project.

   Serial blocks must be either of fixed length, and begin with an alignment
   string, or they must end with a termination string.  If blocks are variable
   in length, then the length field specifies the longest block length.

   The serial blocks are treated as an array of the data type defined by the 
   data_type field.
*/

struct Serial {
  str8 item_type;                     	/* block name 'SER' */
  int32_t item_len;                        /* length of this struct in bytes */
  int32_t start;                           /* byte address in the data record */
  int32_t length;                          /* block length in bytes */
  int32_t rate;                            /* expected number of blocks per sec */
  int32_t data_type;                       /* BYTE_TYPE, SHORT_TYPE, etc */
  str8 name;                          	/* data block name */
  int32_t port_num;                        /* serial port number, 1 - 8 */
  int32_t parity;                          /* SER_EVEN, SER_ODD, SER_NONE */
  int32_t flow_ctl;                        /* flow ctl enabled, TRUE or FALSE */
  int32_t baud_rate;                       /* baud rate */
  int32_t data_bits;                       /* 7 or 8 */
  float stop_bits;                      /* 1.0 to 2.0 */
  str4 term_str;                        /* termination string, NULL = none */
  str4 align_str;                       /* alignment string, NULL = none */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Serial Serial;

/* Sampled data, housekeeping, and digital output data item descriptors.
   Each sampled data variable requires a Sh struct entry in the header.  The
   250 hz channels must be grouped together, followed by the 50 hz channels,
   followed by the 5 hz channels, followed by the 1 hz channels.

   The channels for a given dsm should be grouped within a particular sampling
   frequency block.

   The housekeeping channels and digital output channels must each be grouped
   together also.

   The digital output channels allow toggling between 2 16 bit patterns with
   each pattern held for a fixed number of seconds.  The default startup
   patterns and cycle times are defined in the adscmd startup file, and may
   be changed dynamically by the user during operation.  The digital output
   data is recorded as a block with 1 a second sample rate.

   NOTE: The SDI, HSKP, and DIGOUT blocks must be of a length divisible by 4.
   To accomplish this there must be an even number of SDI, HSKP, and DIGOUT
   variables declared in the tape header.  An odd number of variables will be
   considered an error.
*/

struct Sh {
  str8 item_type;   			/* desc type, 'SDI' 'HSKP' 'DIGOUT' */
  int32_t item_len;      			/* Length of this struct in bytes */
  int32_t start;         			/* byte address in the data record */
  int32_t length;        			/* data item length in bytes */
  int32_t rate;          			/* sample rate */
  int32_t adsaddr;       			/* ADS channel address */
  str8 name;        			/* name of variable */
  str4 type;          			/* "A", "D", or "C" */
  int32_t offset;        			/* offset between samples */
  float convert;      			/* conversion factor, bits to units */
  int32_t order;         			/* number of cal coefficients */
  float cof[MX_COF];  			/* cal coefficients array */
  int32_t ch_gain;				/* channel gain setting, 1, 5, 10, 20 */
  int32_t ch_offset;			/* channel offset, 0 or 10 volts */
  str8 dsm_locn;			/* sampling module location */
  int32_t convert_offset;			/* conversion offset counts at 0 volts*/
};
typedef struct Sh Sh;
 
/* UV hygrometer item descriptor. */

struct Uvhyg {
  str8 item_type;   			/* 'UVHYG' */
  int32_t item_len;      			/* Length of this struct in bytes */
  int32_t start;         			/* byte address in the data record */
  int32_t length;        			/* block length in bytes */
  float cof_p[MX_UV_COF];		/* pressure coefficients */
  float cof_t[MX_UV_COF];		/* temperature coefficients */
  float cof_mr[MX_UV_COF];		/* mixing ratio coefficients */
  str8 locn;        			/* sensor location */
  str8 serial_num;			/* instrument serial number */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Uvhyg Uvhyg;

struct Mcrh {
  str8 item_type;                       /* block name 'MCR' */
  int32_t item_len;                        /* Length of this struct in bytes */
  int32_t lrlen;                           /* logical record length */
  int16_t lrppr;                          /* logical recs per physical record */
  str8 name;                            /* Probe name */
  str8 locn;                            /* Probe location */
  str8 dsm_locn;                        /* sampling module location */
};
typedef struct Mcrh Mcrh;

/* Asynchronous record descriptors which and not part of the normal 1
   second data records.  This includes PMS2D and AXBT.  Each record type
   has a record structure defined below.  Pms2 struct is used to define
   both single level and grey scale PMS 2d probes.
*/
 
struct Pms2 {
  str8 item_type;   			/* block name 'PMS2D' */
  int32_t item_len;      			/* Length of this struct in bytes */
  int32_t lrlen;         			/* logical record length */
  int16_t lrppr;        			/* logical recs per physical record */
  int16_t resolution;   			/* Probe resolution (microns) */
  str8 name;        			/* Probe name */
  str8 locn;        			/* Probe location */
  str8 serial_num;        		/* Probe serial number */
  str8 dsm_locn;			/* sampling module location */
  int32_t intf_chan;			/* interface channel no. */
};
typedef struct Pms2 Pms2;
 
/* PMS2D Housekeeping header block (this is actually synchronous).
*/
struct Pms2h {
  str8 item_type;			/* block name 'PMS2DH' */
  int32_t item_len;			/* Length of this struct in bytes */
  int32_t start;				/* byte address in the data record */
  int32_t length;				/* block length in bytes */
  str8 name;				/* Probe name */
  str8 hsk_name[P2D_HOUSE];           	/* housekeeping parameter names	*/
  float hsk_cof[P2D_HOUSE][P1DV2_COF];  /* houskeeping cal coefs array */
  str8 locn;                            /* Probe location */
  str8 serial_num;                      /* Probe serial number */
  str8 dsm_locn;                        /* sampling module location */
  int32_t intf_chan;			/* interface channel no. */
};
typedef struct Pms2h Pms2h;

/* Grey Housekeeping header block (this is actually synchronous).
*/
struct Greyh {
  str8 item_type;                       /* block name 'PMS2DH' */
  int32_t item_len;                        /* Length of this struct in bytes */
  int32_t start;                           /* byte address in the data record */
  int32_t length;                          /* block length in bytes */
  str8 name;                            /* Probe name */
  str8 hsk_name[GREY_HOUSE];             /* housekeeping parameter names */
  float hsk_cof[GREY_HOUSE][P1DV2_COF];  /* houskeeping cal coefs array */
  str8 locn;                            /* Probe location */
  str8 serial_num;                      /* Probe serial number */
  str8 dsm_locn;                        /* sampling module location */
};
typedef struct Greyh Greyh;
 
struct Asy {
  str8 item_type;   			/* "ASYNC" */
  int32_t item_len;      			/* Length of this struct in bytes */
  str8 name;        			/* record name, 'AXBT', ...*/
  int32_t lrlen;         			/* logical record length */
  int32_t lrppr;         			/* logical records per physical rec */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Asy Asy;
 
/*****************************************************************************/
 
/* Now define the structure of the various data blocks within the 1 second
   data records.

   NOTE: Any new structs or modifications to these structs must result in a
   sizeof(struct ..) which is divisible by 4.
*/
 
/* Sampled data header block structure.  The header block has been redefined
   for version 3.0 and above, but its length remains the same, and the id,
   hour minute and second fields are the same. */
 
struct Hdr_blk {
  int16_t id;				/* block id pattern */
  int16_t hour;				/* time stamp */
  int16_t minute;
  int16_t second;
  int16_t year;
  int16_t month;
  int16_t day;
  int16_t rec_cnt;			/* record counter */
  str8 dsm_locn;			/* sampling module location */
};
typedef struct Hdr_blk Hdr_blk;

/* Pre version 3.0 Hdr_blk. */
struct Hdr_blk_old {
  int16_t id;
  int16_t hour;
  int16_t minute;
  int16_t second;
  int16_t ads_stat;
  int16_t async_stat;
  int16_t ins_stat;
  int16_t tape_drv;
  int16_t tape_stat;
  int16_t sync_stat;
  int16_t spare1;
  int16_t spare2;
};
typedef struct Hdr_blk_old Hdr_blk_old;


struct Climet_blk {
  uint16_t pof;
  uint16_t counts[6]; 		/* particle counts histogram */
};
typedef struct Climet_blk Climet_blk;

struct Mca_blk {
  uint16_t counts[256];		/* particle counts histogram */
};
typedef struct Mca_blk Mca_blk;

struct Mca_status {
  char DataChkSum[4];
  char PresetTime[3];
  char Battery;
  char RealTime[4];
  char LiveTime[4];
  char Threshold[2];
  char Flags;
  char CheckSum;
};
typedef struct Mca_status Mca_status;

/* DME block structure */
 
struct Dme_blk {
  int32_t freq;          			/* frequency in Bi-Quinary */
  int32_t dist;          			/* distance data in BCD */
};
typedef struct Dme_blk Dme_blk;

/* Event block structure. */

struct Evt_blk {
  int32_t evt_data;		  	/* event data */
};
typedef struct Evt_blk Evt_blk;
		   
/* Trimble receiver GPS struct */

struct Gps_blk {
  int32_t  ghealth;			/* gps health */
  float glat;                 		/* latitude */
  float glon;                 		/* longitude */
  float galt;                 		/* altitude */
  float bias;                 		/* clock bias */
  float postime;              		/* time of position data */
  float veleast;              		/* east/west velocity */
  float velnrth;              		/* north/south velocity */
  float velup;                		/* vertical velocity */
  float biasrt;               		/* bias rate */
  float veltime;              		/* time of velocity data */
  int32_t addstat;			/* additional stat. pkt 4b */
  int32_t gpsmode;			/* mode of operation */
  unsigned char gpssats[GPS_SATS]; 	/* satellites being used */
  float gpstime;              		/* current gps time */
  int32_t gpsweek;			/* current gps week */
  float gpsutc;               		/* upc difference */
};
typedef struct Gps_blk Gps_blk;

/* Honeywell Arinc GPS struct */
 
struct HwGps_blk {
  int32_t meas_status;                   /* measurement status */
  int32_t utc_time;                      /* UTC time */
  int32_t altitude;                      /* altitude (MSL) */
  int32_t hdop;                          /* HDOP */
  int32_t vdop;                          /* VDOP */
  int32_t latitude;                      /* latitude */
  int32_t longitude;                     /* longitude */
  int32_t ground_speed;                  /* ground speed */
  int32_t lat_fine;                      /* latitude fine */
  int32_t long_fine;                     /* longitude fine */
  int32_t vertical_fom;                  /* vertical FOM */
  int32_t vertical_velocity;             /* vertical velocity */
  int32_t velocity_ns;                   /* north/south velocity */
  int32_t velocity_ew;                   /* east/west velocity */
  int32_t horizontal_fom;                /* horizontal FOM */
  int32_t sensor_status;                 /* senstor status */
};
typedef struct HwGps_blk HwGps_blk;

/* Garmin receiver GPS struct */

struct Garmin_blk {
  char  utctime[8];		/* gps UTC time */
  char  utcdate[8];		/* gps UTC date */
  float glat;			/* latitude */
  float glon;			/* longitude */
  int16_t quality;		/* GPS quality */
  int16_t nsat;			/* number of satellites in use */
  float dilution;		/* dilution of horiz. pos. */
  float height;			/* antenna height above sea */
  float geoidal_height;		/* geoidal height */
  float ground_speed;		/* ground speed */
  float course;			/* course over ground */
  char  north;			/* north/south lat. */
  char  east;			/* east/west lon. */
  char  status;			/* status */
//  float mag_var;			/* magnetic variation */
//  char mag_var_dir;			/* magnetic variation dir. */
};
typedef struct Garmin_blk Garmin_blk;

 
/* NOAA/AOC Air data computer aoa struct */

struct AdcAoa_blk {
  int32_t aoa;                     /* angle-of-attack */
  int32_t adcpalt;                 /* pressure alt - feet */
  int32_t adcbcalt;                /* baro corrected alt - feet */
  int32_t adcmach;                 /* mach number */
  int32_t adccas;                  /* calib airspeed - knots */
  int32_t adctas;                  /* true airspeed - knots */
  int32_t adctat;                  /* total air temp - C */
  int32_t adcsat;                  /* static air temp - C */
};
typedef struct AdcAoa_blk AdcAoa_blk;

/* Magnavox MX4200 receiver GPS struct */

struct GpsMag_blk {
  str8 gpstime;
  float glat;                  		/* latitude  */
  float glon;                  		/* longitude */
  float galt;                  		/* altitude (meters) */
  float geo_height;            		/* geoidal height */
  float veleast;              		/* east/west velocity */
  float velnrth;              		/* north/south velocity */
  int32_t gpsmode;			/* mode of operation */
};
typedef struct GpsMag_blk GpsMag_blk;
 
/* INS block structure */
 
struct Ins_blk   {
  int16_t second;        			/* sample second */
  int16_t t250;         			/* 250th of a second */
  int32_t lat;           			/* latitude */
  int32_t vx1;           			/* x velocity #1 */
  int32_t vy1;           			/* y velocity #1 */
  int32_t alpha1;        			/* wander angle #1 */
  int32_t truehd1;       			/* true heading #1 */
  int32_t vx2;           			/* x velocity #2 */
  int32_t vy2;           			/* y velocity #2 */
  int32_t windang;       			/* wind angle, not valid */
  int32_t vx3;           			/* x velocity #3 */
  int32_t vy3;           			/* y velocity #3 */
  int32_t alpha2;        			/* wander angle #2 */
  int32_t truehd2;       			/* true heading #2 */
  int32_t vx4;           			/* x velocity #4 */
  int32_t vy4;           			/* y velocity #4 */
  int32_t track;         			/* track angle */
  int32_t vx5;           			/* x velocity #5 */
  int32_t vy5;           			/* y velocity #5 */
  int32_t alpha3;        			/* wander angle #3 */
  int32_t truehd3;       			/* true heading #3 */
  int32_t vx6;           			/* x velocity #6 */
  int32_t vy6;           			/* y velocity #6 */
  int32_t gndspd;        			/* ground speed */
  int32_t vx7;           			/* x velocity #7 */
  int32_t vy7;           			/* y velocity #7 */
  int32_t alpha4;        			/* wander angle #4 */
  int32_t truehd4;       			/* true heading #4 */
  int32_t vx8;           			/* x velocity #8 */
  int32_t vy8;           			/* y velocity #8 */
  int32_t lon;           			/* longitude */
  int32_t vx9;           			/* x velocity #9 */
  int32_t vy9;           			/* y velocity #9 */
  int32_t alpha5;        			/* wander angle #5 */
  int32_t truehd5;       			/* true heading #5 */
  int32_t vx10;          			/* x velocity #10 */
  int32_t vy10;          			/* y velocity #10 */
};
typedef struct Ins_blk Ins_blk;

/* IRS block structure */

struct Irs_blk{
 
/* The 50 Hertz parameters */
  int32_t integ_vert_accel[RATE_50];   	/* Label 265 */
  int32_t pitch_angle[RATE_50];        	/* Label 324 */
  int32_t roll_angle[RATE_50];         	/* Label 325 */
  int32_t pitch_rate[RATE_50];         	/* Label 326 */
  int32_t roll_rate[RATE_50];          	/* Label 327 */
  int32_t yaw_rate[RATE_50];           	/* Label 330 */
  int32_t long_accel[RATE_50];         	/* Label 331 */
  int32_t lat_accel[RATE_50];          	/* Label 332 */
  int32_t normal_accel[RATE_50];       	/* Label 333 */
  int32_t pitch_att_rate[RATE_50];     	/* Label 336 */
  int32_t roll_att_rate[RATE_50];      	/* Label 337 */
  int32_t pot_vert_speed[RATE_50];     	/* Label 360 */
  int32_t vertical_accel[RATE_50];     	/* Label 364 */

/* The 25 Hertz parameters */
  int32_t true_heading[RATE_25];       	/* Label 314 */
  int32_t inertial_alt[RATE_25];       	/* Label 361 */
  int32_t inrt_vert_speed[RATE_25];    	/* Label 365 */

/* The 10 Hertz parameters */
  int32_t ground_speed[RATE_10];       	/* Label 312 */
  int32_t wind_speed[RATE_10];         	/* Label 315 */
  int32_t wind_dir_true[RATE_10];      	/* Label 316 */
  int32_t platform_hdg[RATE_10];       	/* Label 334 */
  int32_t velocity_ns[RATE_10];        	/* Label 366 */
  int32_t velocity_ew[RATE_10];        	/* Label 367 */
 
/* The 5 Hertz parameters */
  int32_t present_lat[RATE_5];         	/* Label 310 */
  int32_t present_lon[RATE_5];         	/* Label 311 */
  int32_t norm_accel[RATE_5]; 	  	/* Label 370 */
 
/* The 2 Hertz parameters */
  int32_t irs_discretes[RATE_2];       	/* Label 270 */
  int32_t irs_maint_discretes[RATE_2]; 	/* Label 350 */
  int32_t time_to_nav_ready[RATE_2];   	/* Label 351 */
  int32_t equipment_id[RATE_2];        	/* Label 371 */
 
/* The time lags to parameters (ms ADS time when first sample was received) */
  int32_t lag_50hz_frame;  		/* Time first 50 hz frame was rx */
  int32_t lag_25hz_frame; 			/* Time first 25 hz frame was rx */
  int32_t lag_10hz_frame; 			/* Time first 10 hz frame was rx */
  int32_t lag_5hz_frame;  			/* Time first 5 hz frame was rx */

/* Additional variables should go at the bottom for backwards compatibilty. */
  int32_t track_ang_rate[RATE_50];		/* Label 335 */
  int32_t track_angle_true[RATE_25];	/* Label 313 */
  int32_t drift_angle[RATE_25];		/* Label 321 */
}; 
typedef struct Irs_blk Irs_blk;

struct Cmigits3_blk{

/* 10 Hz parameters */

  int32_t Clatitude[RATE_10];
  int32_t Clongitude[RATE_10];
  int32_t Caltitude[RATE_10];
  int32_t Cvelocity_north[RATE_10];
  int32_t Cvelocity_east[RATE_10];
  int32_t Cvelocity_up[RATE_10];
  int32_t Cpitch[RATE_10];
  int32_t Croll[RATE_10];
  int32_t Ctrue_heading[RATE_10];

/* 1 Hz parameters */

  int32_t Caccel_east;
  int32_t Caccel_north;
  int32_t Caccel_up;
};
typedef struct Cmigits3_blk Cmigits3_blk;

/* JplTdl block structure */

struct TdlJp_blk {
  float vmrs;				/* volume mixing ratio */
  float vmrw;				/* volume mixing ratio */
  float press;				/* pressure */
  float temp;				/* temperature */
};
typedef struct TdlJp_blk TdlJp_blk;

struct Jpltdl_blk {
  struct TdlJp_blk jptdl[RATE_10];
};
typedef struct Jpltdl_blk Jpltdl_blk;

/* RDMA block structure */

struct Rdma_blk {
  str8 item_type;   			/* 'hk','cnts','concup','concdn' */
  float vd;				/* control valve dilution */
  float vsh;				/* control valve sheath */
  float qa;				/* diff. pressure aerosol */
  float qsh;				/* diff. pressure sheath */
  float qs;				/* diff. pressure sample */
  float qex;				/* diff. pressure exhaust */
  float pabs;				/* pressure absolute */
  float rh;				/* relative humidity */
  float temp;				/* temperature */
  float hvps;				/* high voltage power supply */
  float scan[200];			/* scan dia. and concentrations */	
};
typedef struct Rdma_blk Rdma_blk;

/* UW NEPH block structure */

struct Neph_UW_blk {
  float time;		/* seconds since 00:00:00 on Jan 01, 2000 (UTC) */
  float tsB_10;		/* blue scatter for D<10 (Mm-1) */
  float tsG_10;		/* green scatter for D<10 (Mm-1) */
  float tsR_10;		/* red scatter for D<10 (Mm-1) */
  float Bap_10;		/* absorption for D<10 (Mm-1) */
  float tsB_1;		/* green scatter for D<1 (Mm-1) */
  float tsG_1;		/* green scatter for D<1 (Mm-1) */
  float tsR_1;		/* green scatter for D<1 (Mm-1) */
  float tsG_180;	/* green scatter at 180 degrees */
  float P_10;		/* pressure in the D<10 neph (mb) */
  float P_1;		/* pressure in the D<1 neph (mb) */
  float T_10;		/* temperature in the D<10 neph (K) */
  float T_1;		/* temperature in the D<1 neph (K) */
  float RH_10;		/* relative humidity in the D<10 neph (%) */
  float RH_1;		/* relative humidity in the D<1 neph (%) */
  float fl_10;		/* status flag for D<10 neph */
};
typedef struct Neph_UW_blk Neph_UW_blk;

/* NEPH block structure */

struct Neph_blk {
  float bscat; 		/* bscat */
  float cal_coef;	/* Calibrator coef */
  float press;		/* pressure mbar */
  float temp;		/* temperature deg K */
  float RH;		/* Rh, percent */
  float relay;		/* relay status */
};
typedef struct Neph_blk Neph_blk;

/* LhTdl block structure */

struct TdlLh_blk {
  float ppmv;                           /* concentration */
  float vmr;                            /* volume mixing ratio */
  float relhum;                         /* relative humidity */
  float press;                          /* pressure */
  float temp;                           /* temperature */
  float pp2f;				/* p-p 2f amplitude counts */
  float Pv0;				/* laser power */
  float Indexmax;			/* line position */
  float Yzero;				/* det. zero level */
};
typedef struct TdlLh_blk TdlLh_blk;

struct LhTdl_blk {
  struct TdlLh_blk tdllh[4];
};
typedef struct LhTdl_blk LhTdl_blk;

/* NOAA block structure */

struct Noaa_blk {
  int16_t day;				/* day (0-366) */
  int16_t time;				/* time (sec from midnight) */
  float press;				/* pressure */
  float lah2o;				/* Lyman Alpha H2O */
  float fph0;				/* Frost Point 0 */
  float fph1;				/* Frost Point 1 */
};
typedef struct Noaa_blk Noaa_blk;

/* Aerosol block structure */

struct Aerosol_blk {
  float BCUT;				/* Nominal cut diameter (micrometers)*/
  float BABS;				/* Light absorption coef. (1/Mm) */
  float BSPD;				/* light scat. coef. (1/Mm) at low RH */
  float BSPW;				/* light scat. coef. (1/Mm) at hi RH */
  float BSPWRH;				/* RH (percent) for BSPW measurement */
};
typedef struct Aerosol_blk Aerosol_blk;

/* Mass_spec block structure */

struct Mass_blk {
  float mass1;
  float mass2;
  float mass3;
  float mass4;
};
typedef struct Mass_blk Mass_blk;

/* NOAA Ozone block structure */
struct Ozone_blk {
  int16_t day;				/* day (0-366) */
  int16_t time;				/* time (sec from midnight) */
  float press;				/* pressure */
  float ozone;				/* ozone */
};
typedef struct Ozone_blk Ozone_blk;

/* Digital Pressure */

struct Dpres_blk {
  float press[5];				/* Digital pressure */
};
typedef struct Dpres_blk Dpres_blk;

/* NONOY block structure */
struct Nonoy_blk {
  float analog[10];			/* analog data */
  int16_t pio;				/* vlave state */
  int16_t c0;				/* counter c0 */
  int16_t c1;				/* counter c1 */
  int16_t c2;				/* counter c2 */
  int16_t c3;				/* counter c3 */
  int16_t hour;				/* hours */
  int16_t minute;			/* minutes */
  int16_t second;			/* seconds */
  char label;				/* block label */
  char state;				/* instrument state */
  char other;				/* other info. string */
};
typedef struct Nonoy_blk Nonoy_blk;

/* Loran-C block structure */
 
struct Lrn_blk   {
  int16_t stat;                   	/* status */
  int16_t second;                    	/* second of record */
  int16_t msec;                   	/* milliseconds of record */
  char rec1[LRN_REC];           	/* ascii "01" */
  int16_t data1[LRN_DATA];        	/* 1st record of data */
  char rec2[LRN_REC];           	/* ascii "02" */
  int16_t data2[LRN_DATA];        	/* 2nd record of data */
  char rec3[LRN_REC];           	/* ascii "03" */
  int16_t data3[LRN_DATA];        	/* 3rd record of data */
  char rec4[LRN_REC];           	/* ascii "04" */
  int16_t data4[LRN_DATA];        	/* 4th record of data */
  int16_t spare;                  	/* long boundary */
};
typedef struct Lrn_blk Lrn_blk;

/* Ophir 3 radiometer block structure. */
/* Valid black body codes are as follows:

	"b"	normal operation
	"z"	detector zero
	"c"	black body full cool cycle
	"h"	black body ful heat cycle
*/
 
struct Oph3_blk {
  int16_t gain;			/* electronic gain, 1, 2, or 3 */
  str2 bb_pos;			/* blackbody code, see above*/
  uint16_t det_bias;		/* detector dc bias */
  uint16_t snout_tmp;		/* optical snout temperature */
  uint16_t bench_tmp;		/* optical bench temperature */
  uint16_t det_tmp;		/* HgCdTe detector temperature */
  uint16_t cool_tmp;		/* cooled blackbody temperature */
  uint16_t float_tmp;		/* floating blackbody temperature */
  uint16_t motor_spd;		/* motor speed */
  uint16_t det_sig;		/* detector signal */
  uint16_t box_tmp;		/* electronics box temperature */
  uint16_t spare;		/* long word boundary */
};
typedef struct Oph3_blk Oph3_blk;

/* ADS bus interface pms 1d probe block structures. */

/* PMS 1D 16 bin probe block structure */
 
struct P16_blk {
  int16_t rng_flg;      			/* range or flag word */
  int16_t aux[P1D_AUX]; 			/* aux channel data */
  uint16_t bins[P1D_16]; 			/* data bins */
  int16_t spare;        			/* long boundary */
};
typedef struct P16_blk P16_blk;

/* PMS 1D 260X probe block structure */
 
struct P260_blk {
  int16_t flag;         			/* flag word */
  int16_t aux[P1D_AUX]; 			/* aux channel data */
  uint16_t bins[P1D_260]; 			/* data bins */
  int16_t spare1;
  int16_t spare2;
};
typedef struct P260_blk P260_blk;
 
/* PMS 1D F300 probe block structure */
 
struct P300_blk {
  int16_t flag;      			/* flag word */
  int16_t aux[P1D_AUX]; 			/* aux channel data */
  uint16_t bins[P1D_300]; 			/* data bins */
  int16_t spare1;
};
typedef struct P300_blk P300_blk;

/* Vme bus interface Pms 1d data blocks.  These are used when pms 1d probes are 
   sampled using the VME based pms 1d interface board.
*/

/* Vme bus interface pms 1d 16 bin probe block structure */

struct P16v2_blk {
  int16_t rng_flg;                        /* range or flag word */
  int16_t dummy;
  int16_t aux[P1DV2_AUX];                 /* aux channel data */
  uint16_t bins[BINS_16];                  /* data bins */
};
typedef struct P16v2_blk P16v2_blk;

/* Vme bus interface pms 1d 32 bin probe block structure */

struct P32v2_blk {
  int16_t rng_flg;                        /* range or flag word */
  int16_t dummy;
  int16_t aux[P1DV2_AUX];                 /* aux channel data */
  uint16_t bins[BINS_32];                  /* data bins */
};
typedef struct P32v2_blk P32v2_blk;

/* Serial port interface SPP-100 40 bin probe block structure */

struct DMT100_blk {
  uint16_t cabinChan[8];
  uint32_t rejDOF;
  uint32_t rejAvgTrans;
  uint16_t range;
  uint16_t AvgTransit;
  uint16_t FIFOfull;
  uint16_t resetFlag;
  uint32_t ADCoverflow;
  uint32_t OPCchan[OPCCHAN];	/* 40 channels max */
};
typedef struct DMT100_blk DMT100_blk;

struct Spp100_blk {
  struct DMT100_blk spp100[RATE_10];         /* block of 10 structs */
}; 
typedef struct Spp100_blk Spp100_blk;

/* Serial port interface SPP-200 PCASP probe block structure */

struct DMT200_blk {
  uint16_t cabinChan[8];
  uint16_t range;
  uint16_t fill;
  uint16_t AvgTransit;
  uint16_t FIFOfull;
  uint16_t resetFlag;
  uint16_t SyncErrA;
  uint16_t SyncErrB;
  uint16_t SyncErrC;
  uint32_t ADCoverflow;
  uint32_t OPCchan[OPCCHAN];       /* 40 channels max */
};
typedef struct DMT200_blk DMT200_blk;

struct Spp200_blk {
  struct DMT200_blk spp200[RATE_10];         /* block of 10 structs */
};
typedef struct Spp200_blk Spp200_blk;

/* Serial port interface SPP-300 30 bin probe block structure */

struct DMT300_blk {
  uint16_t cabinChan[8];
  uint32_t rejDOF;
  uint16_t range;
  uint16_t AvgTransit;
  uint16_t FIFOfull;
  uint16_t resetFlag;
  uint16_t SyncErrA;
  uint16_t SyncErrB;
  uint32_t ADCoverflow;
  uint32_t OPCchan[OPCCHAN];       /* 40 channels max */
};
typedef struct DMT300_blk DMT300_blk;

struct Spp300_blk {
  struct DMT300_blk spp300[RATE_10];         /* block of 10 structs */
};
typedef struct Spp300_blk Spp300_blk;

/* Vme bus interface pms 1d 64 bin probe block structure */

struct P64v2_blk {
  int16_t rng_flg;       			/* range or flag word */
  int16_t dummy;
  int16_t aux[P1DV2_AUX];                 /* aux channel data */
  uint16_t bins[BINS_64];                  /* data bins */
};
typedef struct P64v2_blk P64v2_blk;

/* Vme bus interface pms 1d particle spacing block structure. */

struct P1dSpc_blk {
  int16_t bins[BINS_64];
};
typedef struct P1dSpc_blk P1dSpc_blk;

/* Generic Serial blocks.  The block size and data type of the block elements
   is defined by the Serial header descriptor above.  This data block definition
   provides a union of various pointer types.  The correct pointer is
   selected based upon the data type defined in the Serial struct.
*/

union Ser_blk_ptr {
  char  *char_ptr;
  int16_t *short_ptr;
  int32_t *long_ptr;
  float *float_ptr;
};
typedef union Ser_blk_ptr Ser_blk_ptr;

/* UV hygrometer block.
   The uv hygrometer provides data at a 50 hz rate, with 9 parameters per
   sample interval.
*/

/* First define a sample structure. */

struct Uv_smp {
  int16_t hydr_smp;                       /* hydrogen sample */
  int16_t hydr_ref;                       /* hydrogen reference */
  int16_t kryp_smp;                       /* krypton sample */
  int16_t kryp_ref;                       /* krypton reference */
  int16_t chop_smp;                       /* chopper sample */
  int16_t chop_ref;                       /* chopper reference */
  int16_t temp;                           /* sample volume temperature */
  int16_t pressure;                       /* sample volume pressure */
  float humidity;                       /* calculated humidity */
};
typedef struct Uv_smp Uv_smp;

/* UV hygrometer block structure. */
 
struct Uv_blk {
  struct Uv_smp uvsmp[RATE_50];         /* block of 50 samples */
};   
typedef struct Uv_blk Uv_blk;

/* PPS GPS block.
   The PPS GPS provides data at a 1 hz rate, with 43 parameters occupying 
   64 16-bit words.
*/

struct Ppsgps_blk {
  int16_t flags;			/* flags */
  int16_t hdr_cksum;		/* header checksum */
  float gps_time;		/* GSP time of data */
  float utc_time;		/* UTC time of data */
  int16_t  resv;			/* reserved */
  int16_t time_count;		/* time mark count */
  float lat;			/* latitude */
  float lon;			/* longitude */
  float posx;			/* position X */
  float posy;			/* position Y */
  float posz;			/* position Z */
  float alt_msl;			/* altitude MSL */
  float alt_dtm;			/* altitude DTM */
  float vel_east;		/* velocity east */
  float vel_north;		/* velocity north */
  float vel_up;			/* velocity up */
  float acc_east;		/* acceleration east */
  float acc_north;		/* acceleration north */
  float acc_up;			/* accereration up */
  float pitch;			/* pitch */
  float roll;			/* roll */
  float heading;			/* true heading */
  float mag_var;			/* magnetic variation */
  int16_t  ch1_stat1;		/* channel 1 status 1 */
  int16_t  ch1_stat2;		/* channel 1 status 2 */
  int16_t  ch2_stat1;		/* channel 2 status 1 */
  int16_t  ch2_stat2;		/* channel 2 status 2 */
  int16_t  ch3_stat1;		/* channel 3 status 1 */
  int16_t  ch3_stat2;		/* channel 3 status 2 */
  int16_t  ch4_stat1;		/* channel 4 status 1 */
  int16_t  ch4_stat2;		/* channel 4 status 2 */
  int16_t  ch5_stat1;		/* channel 5 status 1 */
  int16_t  ch5_stat2;		/* channel 5 status 2 */
  int16_t  fom;			/* FOM word */
  int16_t  err_horiz;		/* expected horizontal error */
  int16_t  err_vert;		/* expected vertical error */
  int16_t  data_valid;		/* Aiding data valid word */
  int16_t  data_used;		/* Aiding data used word */
  int16_t  time_fom;		/* Time FOM */
  int16_t  day_of_year;		/* Day of year */
  int16_t  checksum;		/* Checksum */
};
  typedef struct Ppsgps_blk Ppsgps_blk; 
/*****************************************************************************/

/* Define the Mcr record structures. */
 
/* Mcr logical and physical records structures */
 
struct Mcr_rec {
  str8  id;				/* Mcr channel no. */
  int16_t hour;
  int16_t minute;
  int16_t second;
  int16_t data[MCR_CHAN_DATA];            /* high_rate buffer */
};
typedef struct Mcr_rec Mcr_rec;
 

/*****************************************************************************/
 
/* Define the asynchronous record structures. */
 
/* PMS 2d logical and physical records structures */
 
struct P2d_rec {
  int16_t id;                     	/* 'P1','C1','P2','C2', H1, H2 */
  int16_t hour;
  int16_t minute;
  int16_t second;
  int16_t spare1;
  int16_t spare2;
  int16_t spare3;
  int16_t tas;                    	/* true air speed */
  int16_t msec;                   	/* msec of this record */
  int16_t overld;                 	/* overload time, msec */
  unsigned char data[P2D_DATA]; 	/* image buffer */
};
typedef struct P2d_rec P2d_rec;

struct P2d_hdr {
  int16_t id;                     	/* 'P1','C1','P2','C2', H1, H2 */
  int16_t hour;
  int16_t minute;
  int16_t second;
  int16_t spare1;
  int16_t spare2;
  int16_t spare3;
  int16_t tas;                    	/* true air speed */
  int16_t msec;                   	/* msec of this record */
  int16_t overld;                 	/* overload time, msec */
};
typedef struct P2d_hdr P2d_hdr;

struct P2d_data {
  unsigned char data[P2D_DATA]; 	/* image buffer */
};
typedef struct P2d_data P2d_data;

struct P2d_house {
  int16_t hdata[P2D_HOUSE];
};
typedef struct P2d_house P2d_house;

struct P2d_Phys {
 struct P2d_rec rec[P2DLRPR];
};
typedef struct P2d_Phys P2d_Phys;

/* Pms 2d grey scale particle structures.  Each particle is recorded as a
   GreyParticle header, followed by the image data.  The image data
   consists of slice_count GreySlice structures.  A slice_cnt value of 
   GREY_END_OF_RECORD (-1) indicates the end of a logical record.

   Each logical record is MX_PHYS (32768) bytes in length.  The logical
   record is filled with as many complete particles from a single probe as will
   fit into the record.  Any unused space at the end of the record is
   undefined.  A physical tape record is also MX_PHYS (32768) bytes long and
   consists of 1 logical record.
*/

struct GreyParticle {
  str4 id;                              /* probe id 'G1', 'G2' */
  int16_t hour;                           /* particle time stamp, hour */
  int16_t minute;                         /* time stamp, minute */
  int16_t second;                         /* time stamp, second */
  int16_t msec;                           /* time stamp, millisecond */
  int32_t ena_sec;                         /* probe enabled second count */
  int32_t ena_usec;                        /* probe enable usec count */
  int32_t acq_sec;                         /* particle acquired second count */
  int32_t acq_usec;                        /* particle acquired usec count */
  int16_t tas;                            /* true air speed, m/sec */
  int16_t width;                          /* particle max width in pixels */
  int16_t length;                         /* particle max length in pixels */
  int16_t probe_ctl;                      /* probe control word */
  int16_t ovld_cnt;                       /* # particles while in overload */
  int16_t slice_cnt;                      /* # slices in the image data */
  int16_t chg_len;                        /* # 16-bit charge samples */
  int16_t spare1;
  int16_t spare2;
  int16_t spare3;
};
typedef struct GreyParticle GreyParticle;

/*** Original GreyParticle struct used only once.  Delete this if Darrel says
     ok.
struct GreyParticle {
  str4 id;
  int16_t hour;
  int16_t minute;
  int16_t second;
  int16_t msec;
  int16_t tas;
  int16_t min_cnt;
  int16_t mid_cnt;
  int16_t max_cnt;
  int16_t width;
  int16_t length;
  int16_t probe_ctl;
  int16_t slice_cnt;
  int16_t ovld_cnt;
  int16_t first_pix;
};
***/

/* An image slice is defined by an array of GREY_BYTES_PER_SLICE bytes. */

struct GreySlice {
  unsigned char segment [GREY_BYTES_PER_SLICE];
};
typedef struct GreySlice GreySlice;

struct Grey_house {
  int16_t hdata[GREY_HOUSE];
};
typedef struct Grey_house Grey_house;


/*****************************************************************************/

/* Define the NATS format data blocks */
/* Nats capability has been deleted with version 3.0 and above. */

/* LTN-51 INS NATS block structure */

struct Ins_blk_nats {
  int16_t second;       			/* sample second */
  int16_t t250;         			/* 250th of a second */
  int32_t lat;           			/* latitude */
  int32_t lon;           			/* longitude */
  int32_t vx1;           			/* x velocity #1 */
  int32_t vy1;           			/* y velocity #1 */
  int32_t alpha1;        			/* wander angle #1 */
  int32_t truehd1;       			/* true heading #1 */
  int32_t track;         			/* track angle */
  int32_t gndspd;        			/* ground speed */
};
 
/* Honeywell IRS NATS block structure */

struct Irs_blk_nats{

/* The 50 Hertz parameters */
  int32_t integ_vert_accel;   		/* Label 265 */
  int32_t pitch_angle;        		/* Label 324 */
  int32_t roll_angle;         		/* Label 325 */
  int32_t pitch_rate;         		/* Label 326 */
  int32_t roll_rate;          		/* Label 327 */
  int32_t yaw_rate;           		/* Label 330 */
  int32_t long_accel;         		/* Label 331 */
  int32_t lat_accel;          		/* Label 332 */
  int32_t normal_accel;       		/* Label 333 */
  int32_t pitch_att_rate;     		/* Label 336 */
  int32_t roll_att_rate;      		/* Label 337 */
  int32_t pot_vert_speed;     		/* Label 360 */
  int32_t vertical_accel;     		/* Label 364 */
 
/* The 25 Hertz parameters */
  int32_t true_heading;       		/* Label 314 */
  int32_t inertial_alt;       		/* Label 361 */
  int32_t inrt_vert_speed;    		/* Label 365 */

/* The 10 Hertz parameters */                                     
  int32_t ground_speed;       		/* Label 312 */
  int32_t wind_speed;         		/* Label 315 */
  int32_t wind_dir_true;      		/* Label 316 */
  int32_t platform_hdg;       		/* Label 334 */
  int32_t velocity_ns;        		/* Label 366 */
  int32_t velocity_ew;        		/* Label 367 */
 
/* The 5 Hertz parameters */
  int32_t present_lat;         		/* Label 310 */
  int32_t present_lon;         		/* Label 311 */
  int32_t norm_accel;          		/* Label 370 */
 
/* The 2 Hertz parameters */                                      
  int32_t irs_discretes;       		/* Label 270 */
  int32_t irs_maint_discretes; 		/* Label 350 */
  int32_t time_to_nav_ready;   		/* Label 351 */
  int32_t equipment_id;        		/* Label 371 */
 
/* The time lags to parameters (ms ADS time when first sample was received) */
  int32_t lag_50hz_frame;    		/* Time first 50 hz frame was rx */
  int32_t lag_25hz_frame;    		/* Time first 25 hz frame was rx */
  int32_t lag_10hz_frame;    		/* Time first 10 hz frame was rx */
  int32_t lag_5hz_frame;     		/* Time first 5 hz frame was rx */
}; 
 
#endif
