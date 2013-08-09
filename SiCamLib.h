///////////////////////////////////////////////////////////////////////////
// Camera Object Interface Library.                                      //
// Copyright (C) Spectral Instruments, Inc., 2003, 2004, 2005, 2006, 2007//
///////////////////////////////////////////////////////////////////////////
// Camera.h: interface for the CCamera class.
// Copyright (C) Spectral Instruments, Inc., 2003, 2004, 2005, 2007
#ifndef _CAMERA_H_
#define _CAMERA_H_

#define	STANDARD_DOWNLOAD_BREAK_TIME	200		// "breaktime" to use for SendBin() and SendDSPCode().

// Error definitions
#define	E_NO_ERROR						0		// Call was successful (no error).
#define E_TIMEOUT						1
#define E_UNUSABLE_POINTER				2
#define E_COM_ERROR						3
#define E_DMA_ACTIVE					4
#define E_DMA_NOT_ACTIVE				5
#define	E_RETRIES_EXHAUSTED				6
#define	E_FILE_NOT_FOUND				7
#define	E_NO_DRIVERS_INSTALLED			8
#define	E_DMA_THREAD_FAILED				9
#define	E_DEINTERLACE_THREAD_FAILED		10
#define	E_MEMORY_ALLOCATION_FAILED		11
#define	E_INVALID_DEINTERLACE_MODE		12
#define	E_INVALID_PARAMETER_OFFSET		13
#define	E_INVALID_CAMERA_DB_INDEX		14
#define	E_MEDIA_TYPE_NOT_SELECTABLE		15
#define	E_MEDIA_TYPE_INVALID			16
#define	E_STRING_TOO_LONG				17
#define E_NEGATIVE_RESPONSE				18
#define E_FUNCTION_UNSUPPORTED			19
#define E_UNSUPPORTED_RECORD_TYPE		20
#define E_FRAMING_OVERRUN				21
#define E_FILE_OPEN_ERROR				22

#define CAM_ALREADY_OPEN	8001
#define CAM_NOT_OPEN		8002
#define OPEN_DEV_FAILED		8003

// Return values for ExternalTriggerStatus()
#define	EXTERNAL_TRIGGER_STAT_LOST	0
#define	EXTERNAL_TRIGGER_PENDING	1
#define	EXTERNAL_TRIGGER_OCCURRED	2

// bit values for GetIFStatus() only supported when pci_interface is PCI_INT_PLX or PC104_INT_PLX
#define	GETIF_STATUS_F_EF		0x01	// image fifo empty flag (low active)
#define	GETIF_STATUS_F_HF		0x02	// image fifo half full flag (low active)
#define	GETIF_STATUS_F_FF		0x04	// image fifo full flag (low active)
#define	GETIF_STATUS_HL_SD		0x08	// Gigabit transceiver signal detect
#define	GETIF_STATUS_HL_LFI		0x10	// Gigabit line fault indicator
#define	GETIF_STATUS_IS100MHZ	0x20	// indicates interface runs with 100MBytes/sec speed (high active)
#define	GETIF_STATUS_POR		0x40	// indicates camera received a Power-on reset (high active) This bit clears after read
//#define	GETIF_STATUS_UNUSED	0x80	// Unused bit (returns zero)
// additional bit values only defined when pci_interface is PC104_INT_PLX
#define	GETIF_STATUS_HK_TX_DONE		0x100	// indicates that HK Uart is ready to transmit (high active)
#define	GETIF_STATUS_HK_EF			0x200	// indicates that HK Uart RX fifo is empty (high active)
#define	GETIF_STATUS_HK_FF			0x400	// indicates that HK Uart RX fifo is full (high active)

//////////////////////////////////////////////////////////
// u32: defined type to represent an unsigned 32-bit integer within the SiCamLib library.
// @see u32_ptr
//////////////////////////////////////////////////////////
typedef	unsigned long u32;
//////////////////////////////////////////////////////////
// u32_ptr: defined type to point to a u32.
// @see u32
//////////////////////////////////////////////////////////
typedef	unsigned long *u32_ptr;
//////////////////////////////////////////////////////////
// i32: defined type to represent a signed 32-bit integer within the SiCamLib library.
// @see i32_ptr
//////////////////////////////////////////////////////////
typedef	long i32;
//////////////////////////////////////////////////////////
// i32_ptr: defined type to point to an i32.
// @see i32
//////////////////////////////////////////////////////////
typedef	long *i32_ptr;
//////////////////////////////////////////////////////////
// u16: defined type to represent an unsigned 16-bit integer within the SiCamLib library.
// @see u16_ptr
//////////////////////////////////////////////////////////
typedef	unsigned short u16;
//////////////////////////////////////////////////////////
// u16_ptr: defined type to point to a u16.
// @see u16
//////////////////////////////////////////////////////////
typedef	unsigned short *u16_ptr;
//////////////////////////////////////////////////////////
// u8: defined type to represent an unsigned 8-bit integer within the SiCamLib library.<br>
//	This is our representation of a byte value.
// @see u8_ptr
//////////////////////////////////////////////////////////
typedef	unsigned char u8;
//////////////////////////////////////////////////////////
// u8_ptr: defined type to point to a u8.
// @see u8
//////////////////////////////////////////////////////////
typedef	unsigned char *u8_ptr;
//////////////////////////////////////////////////////////
// si_handle: defined type to represent a handle within the SiCamLib library.
//////////////////////////////////////////////////////////
typedef void *si_handle;

//////////////////////////////////////////////////////////
// enum read_mode:
//	This enum contains the values that describe the possible read modes for the DMA interface.<br>
//	READ_NORMAL signifies a single image acquisition.<br>
//	READ_VIDEO signifies continuous image acquisition until aborted.<br>
// @author Roger W. Cover
//////////////////////////////////////////////////////////
enum read_mode { READ_NORMAL=0,READ_VIDEO };
//////////////////////////////////////////////////////////
// enum di_mode:
//	This enum contains the values that describe the available de-interlace modes.<br>
//	DI_NONE signifies no de-interlacing.<br>
//	DI_4QUAD signifies 4 quadrant readout from all 4 corners.<br>
//	DI_S_SPLIT signifies a split serial register readout.<br>
//	DI_P_SPLIT_SAME_SIDE signifies a split parallel register with the serial registers clocking the same direction.<br>
//	DI_P_SPLIT_OPPOSITE_CORNERS signifies a split parallel register with the serial registers clocking in opposite directions.<br>
//	DI_9_CCD_PORT_A signifies a 9 CCD mosaic with only port A being used.<br>
//	DI_9_CCD_PORT_D signifies a 9 CCD mosaic with only port D being used.<br>
//	DI_9_CCD_A_AND_D signifies a 9 CCD mosaic with both ports A and D being used.<br>
//	DI_16_CCD_PORT_A signifies a 16 CCD mosaic with only port A being used.<br>
//	DI_16_CCD_PORT_D signifies a 16 CCD mosaic with only port D being used.<br>
//	DI_16_CCD_A_AND_D signifies a 16 CCD mosaic with both ports A and D being used.<br>
//	DI_COUNT is a count of the number of supported modes.<br>
// @author Roger W. Cover
//////////////////////////////////////////////////////////
enum di_mode {	DI_NONE=0,DI_4QUAD,DI_S_SPLIT,DI_P_SPLIT_SAME_SIDE,DI_P_SPLIT_OPPOSITE_CORNERS,
	DI_9_CCD_PORT_A,DI_9_CCD_PORT_D,DI_9_CCD_A_AND_D,DI_16_CCD_PORT_A,DI_16_CCD_PORT_D,
	DI_16_CCD_A_AND_D,DI_9_CCD_PORT_A_90,DI_9_CCD_PORT_B_90,DI_9_CCD_A_AND_B_90,
	DI_16_CCD_PORT_A_90,DI_16_CCD_PORT_B_90,DI_16_CCD_A_AND_B_90,DI_COUNT };

#define	MAX_DI_PORT_COUNT	32	// Largest allowed value for the # of ports to deinterlace.

#define	MAX_CAMERA_DRIVER_NAME	12	// Longest device driver name used by our device drivers.

//////////////////////////////////////////////////////////
// enum param_type_enum:
//	This enum contains the values that describe the possible prarmeter types for the parameter_description_type structure.<br>
// PT_UNITS: signifies a parameter with min, max, and units. The user must enter a number in [min..max].<br>
// PT_PULL_DOWN: signifies a parameter with min, max, and a list of text descriptions that will be presented in a pull-down.<br>
// PT_NOT_USED: signifies that the parameter is not in use.<br>
// PT_G4 signifies that the parameter has a G4 entry that will contain a g4_description_type structure.
// @author Roger W. Cover
//////////////////////////////////////////////////////////
enum param_type_enum {	PT_UNITS=0,PT_PULL_DOWN,PT_NOT_USED,PT_G4 };

#define	MAX_PULLDOWNS		6	// Largest number of pull-down descriptions supported.
#define	MAX_BITS_IN_FIELD	24	// Largest number of bits in a bitfield supported.

//////////////////////////////////////////////////////////
// enum g4_subtype_enum:
//	This enum contains the values that describe the possible subtypes for the G4 parameter descriptions.<br>
//	ST_NOT_USED signifies an unused entry in a table.<br>
//	ST_S800_PRESSURE signifies a pressure entry in the S800 look-up-table format.<br>
//	ST_MILLITORR signifies a pressure entry in milliTorr.<br>
//	ST_TEMPERATURE signifies a temperature in degrees K * 10.<br>
//	ST_MILLIVOLTS signifies a voltage in millivolts.<br>
//	ST_VOLTS signifies a voltage in volts.<br>
//	ST_MILLIAMPS signifies a current in milliamps.<br>
//	ST_MILLISECONDS signifies a time in milliseconds.<br>
//	ST_INDEX signifies a list index entry with a string describing each possible value.<br>
//	ST_SPARSE_INDEX signifies a list index entry with a string describing each possible value and the value.<br>
//	ST_BITFIELD signifies a bitfield where each bit has a name.<br>
//	ST_NUMBER signifies a number with no units but with a minimum and a maximum.<br>
//	ST_NUMBER_WITH_UNITS signifies a number with a units name, a minimum, and a maximum.<br>
//	ST_IP_ADDR_ENTRY signifies an IP address stored as a 32-bit value.<br>
//	ST_REL_HUMID_ENTRY signifies a relative humidity in % * 100.<br>
// @author Roger W. Cover
//////////////////////////////////////////////////////////
enum g4_subtype_enum {	ST_NOT_USED=0,ST_S800_PRESSURE,ST_MILLITORR,ST_TEMPERATURE,ST_MILLIVOLTS,ST_VOLTS,ST_MILLIAMPS,ST_MILLISECONDS,
	ST_INDEX,ST_SPARSE_INDEX,ST_BITFIELD,ST_NUMBER,ST_NUMBER_WITH_UNITS,ST_IP_ADDR_ENTRY,ST_REL_HUMID_ENTRY };
//////////////////////////////////////////////////////////
// struct g4_description_type:
//	This structure contains a description of the type for G4 status and parameter subtypes.
// @author Roger W. Cover
//////////////////////////////////////////////////////////
struct g4_description_type {
	//////////////////////////////////////////////////////////
	// subtype: set to a member of g4_subtype_enum
	//////////////////////////////////////////////////////////
	unsigned char subtype;
	union {
		char *unit_string;
		struct {
			u32 index_count;
			char *name[MAX_PULLDOWNS];
			} index;
		struct {
			u32 index_count;
			struct {
				char *name;
				i32 value;
				} pd[MAX_PULLDOWNS];
			} sparse;
		struct {
			u32 bits;	// # of bits defined
			struct {
				char *name;
				u32 mask;
				} bitdata[MAX_BITS_IN_FIELD];
			} bitfield;
		} u;
	};

//////////////////////////////////////////////////////////
// struct parameter_description_type:
//	This structure contains the fields required to describe a parameter to the auto-entry formating routines.
// @author Roger W. Cover
//////////////////////////////////////////////////////////

struct parameter_description_type {
	//////////////////////////////////////////////////////////
	// pt: set to a member of param_type_enum.<br>
	//	The type of the parameter described by this structure.
	//////////////////////////////////////////////////////////
	param_type_enum pt;
	//////////////////////////////////////////////////////////
	// description: A pointer to the user-readable text that identifies this parameter.
	//////////////////////////////////////////////////////////
	char *description;
	//////////////////////////////////////////////////////////
	// min: The minimum value this parameter can have.
	//////////////////////////////////////////////////////////
	u32 min;
	//////////////////////////////////////////////////////////
	// max: The maximum value this parameter can have.
	//////////////////////////////////////////////////////////
	u32 max;
	//////////////////////////////////////////////////////////
	// t: A union containing one of the necessary user-readable display items identified by the pt field.
	//////////////////////////////////////////////////////////
	union {
		//////////////////////////////////////////////////////////
		// units: A pointer to user-readable text identifying the units for this parameter.
		//////////////////////////////////////////////////////////
		char *units;
		//////////////////////////////////////////////////////////
		// pull_down: A list of pointers to user-readable text identifying the possible values for this parameter.
		//////////////////////////////////////////////////////////
		char *pull_down[MAX_PULLDOWNS];
		//////////////////////////////////////////////////////////
		// g4: A G4 parameter description structure.
		//////////////////////////////////////////////////////////
		g4_description_type g4;
		} t;
	};

/* CPPDOC_BEGIN_EXCLUDE */
//////////////////////////////////////////////////////////
// dma_data_type:
//	This structure contains the information used during the DMA transfer of an image from the camera.
// @author Roger W. Cover
//////////////////////////////////////////////////////////
struct dma_data_type {
	char	device_name[MAX_CAMERA_DRIVER_NAME];// The name of the device driver associated with this camera object.
	u16		ParLen;								// number of rows in the data (parallel length).
	u16		SerLen;								// number of columns in the data (serial length).
	u32		DrvBytes;							// number of bytes in the DMA buffer array.
	u16_ptr dma_buf;							// Pointer to the DMA buffer. If no de-interlace this will = lpImage.
	si_handle hTempImage;						// GlobalAlloc handle for the DMA buffer.
	u32		UsrBytes;							// number of bytes in the user buffer
	u16_ptr lpImage;							// Pointer to the user buffer where the final image will appear.
	u32		FramesError;						// error flag when Frames are out of sync
	u32		NumFrames;							// Total number of frames to be read.
	u32		frame_count;						// Number of frames acquired so far.
	u32		n_ptr_pos;							// Position of the de-interlace or file write completion during readout.
	u32		NumPorts;							// number of ports in this de-interlace mode
	u32		NumParSect;							// number of parallel sections
	u32		StartIndex[MAX_DI_PORT_COUNT];		// start address indices per port
	i32		PixInc[MAX_DI_PORT_COUNT];			// pixel increment per port
	i32		RowInc[MAX_DI_PORT_COUNT];			// row increment per port
	read_mode dma_mode;							// One of the modes from enum read_mode.
	u8		dma_thread_complete;				// Set to TRUE when the readout thread has terminated.
	u8		deint_thread_complete;				// Set to TRUE when de-interlace thread has completed.
	u8		dma_abort;							// Used to communicate between threads.<br>Set to 1 to terminate acquisition.
	u8		frame_complete;						// Set to 1 when the first frame of an acquisition has been read.
	bool	wSave;								// Set to TRUE to write raw data to a file during readout.
	char	*file_name;							// Name of the file to write if wSave is TRUE.
	char	*fits_header;						// Pointer to the FITS header in RAM to write into the file if wSave is TRUE.
	si_handle hFile;							// Handle for save file
	si_handle hSaveArray;						// GlobalAlloc handle for array
	u16_ptr save_array;							// 1-frame buffer for fast file save
	OVERLAPPED ovl;								// structure for overlapped file write
	//bool	ovl;
	};
/* CPPDOC_END_EXCLUDE */

//////////////////////////////////////////////////////////
// enum pci_interface_type:
//	This enum contains the values that describe PCI interface boards.<br>
//	PCI_INT_UNKNOWN signifies an unknown PCI interface.<br>
//	PCI_INT_PLX signifies a PLX PCI interface.<br>
//	PC104_INT_PLX signifies a PC-104+ interface that has an on-board HKS.<br>
// @author Roger W. Cover
// @see media_type
// @see CCamera#SetMediaType
// @see CCamera#GetMediaType
//////////////////////////////////////////////////////////
enum pci_interface_type { PCI_INT_UNKNOWN,PCI_INT_PLX,PC104_INT_PLX };
//////////////////////////////////////////////////////////
// enum media_type:
//	This enum contains the values that describe the media type used to connect to the camera.<br>
//	MEDIA_UNKNOWN signifies an unknown media type (not checked yet).<br>
//	MEDIA_FIXED signifies a media type that can't be changed (all PCI_INT_UNKNOWN interfaces).<br>
//	MEDIA_AIA signifies that the AIA copper connection to the camera is selected.<br>
//	MEDIA_FIBER signifies that the fiber optic connection to the camera is selected.<br>
// @author Roger W. Cover
//////////////////////////////////////////////////////////
enum media_type { MEDIA_UNKNOWN,MEDIA_FIXED,MEDIA_AIA,MEDIA_FIBER };

//////////////////////////////////////////////////////////
// enum si_std_baud_rates:
//	This enum contains the values for the Baud rates used by Spectral Instruments' cameras.<br>
//	DEFAULT_BAUD is the Default Baud rate (the only one for 800 series cameras).<br>
//	ALTERNATE_BAUD1	an alternate Baud rate.<br>
//	ALTERNATE_BAUD2 an alternate Baud rate.<br>
//	ALTERNATE_BAUD3 an alternate Baud rate.<br>
//	ALTERNATE_BAUD4 an alternate Baud rate.<br>
//	ALTERNATE_BAUD5 an alternate Baud rate.<br>
// @author Roger W. Cover
//////////////////////////////////////////////////////////
enum si_std_baud_rates { DEFAULT_BAUD=19200,ALTERNATE_BAUD1=57600,ALTERNATE_BAUD2=38400,ALTERNATE_BAUD3=9600,
	ALTERNATE_BAUD4=115200,ALTERNATE_BAUD5=250000 };

//////////////////////////////////////////////////////////
// class CCamera:
//	This is the genereic Spectral Instruments camera interface class.<br>
//	This class implements all genereic methods and fields used to communicate with the device driver and the camera.
//	The protected methods of this class provide all the necessary functionality to communicate with the camera using
//	any of our defined protocols.
// @author Roger W. Cover
// @author Hans J. Meyer
//////////////////////////////////////////////////////////
class CCamera
	{
	public:	// High Level Functions the user can call.
		CCamera();
		virtual ~CCamera();

		int OpenCamera(char *device_name);		// This must be called first.
		int SetBaudRate(int baud);				// OpenCamera selects the default Baud rate.
		int GetBaudRate();
		int SetMediaType(media_type mt);
		media_type GetMediaType();

		int SendBin(u32 breaktime, char *filename);
		int SendDSPCode(u32 breaktime,const u8 program[],u16 size);
		int SaveDSPCode();
		int GetDSPCodeInfo(u32_ptr part_num,u32_ptr revision,u32_ptr checksum);
		int RestartDSPCode();
		int HardwareReset();
		int TestCommand();

		int CoolerOff();
		int CoolerOn();

		int ClearFrame();
		int CloseShutter();
		int OpenShutter();

		int PrepareAcquisition(u16 row,u16 col,u16_ptr i_buff,read_mode mode,di_mode di,char *file_name,char *fits_hdr);
		int PrepareFramesAcquisition(u16 row,u16 col,u16_ptr i_buff,read_mode mode,di_mode di,int frames,char *file_name,char *fits_hdr);

		int ImageExpose();
		int ImageRead();
		int ImageTdi();
		int ImageExternallyPacedTdi();
		int ImageLinearityTdi();
		int ImageTest();
		int ImageTriggered();

		int DmaFramesStatus(u32 *CurrentDMAFrame,u32 *CurrentDMAPixel,u32 *CurrentDIFrame,u32 *CurrentDIPixel,u8 *DMAPercent,u8 *DIPercent,u8 *TotalPercent);
		int DmaStatus(bool *dma_complete,bool *frame_complete,u32 *frame_count);
		int ReadoutStatus(u32 *pixels_read,u32 *pixels_deinterlaced,u8 *percent_done);
		int ExternalTriggerStatus(u32_ptr value);
		int RemainingExposureTime(u32_ptr ms);
		int AbortExposure();

		int EndAcquisition(bool force_abort);

		int GetDriverVersion(int *version);
		int GetIFStatus(int *status);
		int GetHKSStr(char *dest,int size,int timeout);
		int SendHKSStr(char *str);

		int FITSboolRecord(char *record,const char *keyword,bool data,const char *comment);
		int FITScharRecord(char *record,const char *keyword,const char *data);
		int FITSCommentRecord(char *record,const char *data);
		int FITSi32Record(char *record,const char *keyword,i32 data,const char *comment);
		int FITSu32Record(char *record,const char *keyword,u32 data,const char *comment);

	protected:	// Medium Level Functions derived classes can call.
		int CharsAvail(u32_ptr value);
		int CharsToSend(u32_ptr value);
		int ExpectYN();
		int Flush();
		int GetString(char *str,u16 size,int timeout);
		int ReceiveCharacter(char *c,int timeout);
		int ReceiveDWord(u32_ptr value);
		int SendBreak(u32 ms);
		int SendCharacter(char c);
		int SendCommand(char command);
		int SendDWord(u32 value);
		int SendString(char *str,u16 size);

	private:	// Low Level data for use by this class.
		//////////////////////////////////////////////////////////
		// m_DeviceHandle: Handle to the open device driver. NULL when the camera is not open.
		//////////////////////////////////////////////////////////
		si_handle m_DeviceHandle;
		//////////////////////////////////////////////////////////
		// h_dma_thread: Handle to the DMA reader thread for this camera if DMA is active.
		//////////////////////////////////////////////////////////
		si_handle h_dma_thread;
		//////////////////////////////////////////////////////////
		// h_deinterlace_thread: Handle to the deinterlacing thread for this camera if de-interlace is acvive.
		//////////////////////////////////////////////////////////
		si_handle h_deinterlace_thread;
	public:
		//////////////////////////////////////////////////////////
		// m_DmaActive: Flag for active DMA.
		//////////////////////////////////////////////////////////
		int m_DmaActive;
	private:
		/* CPPDOC_BEGIN_EXCLUDE */
		//////////////////////////////////////////////////////////
		// dma_data: Data structure used by the DMA and de-interlace threads.
		//////////////////////////////////////////////////////////
		volatile dma_data_type dma_data;
		/* CPPDOC_END_EXCLUDE */
		//////////////////////////////////////////////////////////
		// pci_interface: Id type for the PCI interface card.<br>
		//	This is used to determine if the media type is selectable or fixed.
		//////////////////////////////////////////////////////////
		pci_interface_type pci_interface;
		//////////////////////////////////////////////////////////
		// media: The type of media used to connect to the camera.<br>
		// For the Gigabit interface, this is selectable.
		//////////////////////////////////////////////////////////
		media_type media;
		/* CPPDOC_BEGIN_EXCLUDE */
		//////////////////////////////////////////////////////////
		// struct SERIAL_PARAM_STRUCT:
		//	This structure is used to communicate serial communications setup values to the device driver.
		// @author Hans J. Meyer
		//////////////////////////////////////////////////////////
		struct SERIAL_PARAM_STRUCT {
			int flags;
			int baud;
			int bits;
			int parity;
			int stopbits;
			int buffersize;
			int fifotrigger;
			};
		//////////////////////////////////////////////////////////
		// sp: The instance of SERIAL_PARAM_STRUCT used to select the Baud rate, etc. for the camera interface.
		//////////////////////////////////////////////////////////
		SERIAL_PARAM_STRUCT	sp;
		/* CPPDOC_END_EXCLUDE */
	};

#endif
// G2Camera.h: interface for the CG2Camera class.
// Copyright (C) Spectral Instruments, Inc., 2003, 2004, 2005
#ifndef _G2CAMERA_H_
#define _G2CAMERA_H_

//////////////////////////////////////////////////////////
// enum G2StatusIndex:
//	This enum contains names for each index into the G2 system status description array
//	SysStatusStr and the system status array SysStatus.
// @author Roger W. Cover
// @see CG2Camera#SysStatus
// @see CG2Camera#SysStatusStr
//////////////////////////////////////////////////////////
enum G2SysStatusIndex {	CCD_TEMPERATURE=0,BACKPLATE_TEMPERATURE,HEAD_PRESSURE,ADC_SPARE4,
	ADC_SPARE5,ADC_SPARE6,ADC_SPARE7,ADC_SPARE8,
	SHUTTER_STATUS,XIRQ_STATUS,SPARE_STATUS1,SPARE_STATUS2,
	SPARE_STATUS3,SPARE_STATUS4,SPARE_STATUS5,SPARE_STATUS6,
	STATUS_COUNT };

//////////////////////////////////////////////////////////
// enum G2SysReadoutIndex:
//	This enum contains names for each index into the G2 system readout parameter description array
//	SysReadoutDes and the system readout parameter array SysReadout.
// @author Roger W. Cover
// @see CG2Camera#SysReadout
// @see CG2Camera#SysReadoutDes
//////////////////////////////////////////////////////////

//Dave Mills - Copy over TDIShutter paramater from SI800Camera class. Replacing SPARE_PARAM7
//Dave Mills - Copy over TDI_COMMAND paramaters from SI800Camera class, replacing SPARE_PARAM9
//enum G2SysReadoutIndex {SERIAL_ORIGIN=0,SERIAL_READ_LENGTH,SERIAL_BINNING,SERIAL_POSTSCAN,
//	PARALLEL_ORIGIN,PARALLEL_READ_LENGTH,PARALLEL_BINNING,PARALLEL_POSTSCAN,
//	EXPOSURE_TIME,CONTINUOUS_CLEAR_DISABLE,DSI_SAMPLE_TIME,ANALOG_ATTENUATION,
//	PORT1_OFFSET,PORT2_OFFSET,PORT3_OFFSET,PORT4_OFFSET,
//	TDI_DELAY_TIME,SKIP_LINES,BLANK_LINES,SHIFT_LINES,
//	ADC_SAMPLES,CCD_ATTENUATION,SPARE_PARAM10,TDI_COMMAND,
//	SPARE_PARAM8,TDISHUTTER,SPARE_PARAM6,SPARE_PARAM5,
//	SPARE_PARAM4,SPARE_PARAM3,SPARE_PARAM2,SPARE_PARAM1,
//	PARAMETER_COUNT };

enum G2SysReadoutIndex {SERIAL_ORIGIN=0,SERIAL_READ_LENGTH,SERIAL_BINNING,SERIAL_POSTSCAN,
	PARALLEL_ORIGIN,PARALLEL_READ_LENGTH,PARALLEL_BINNING,PARALLEL_POSTSCAN,
	EXPOSURE_TIME,CONTINUOUS_CLEAR_DISABLE,DSI_SAMPLE_TIME,ANALOG_ATTENUATION,
	PORT1_OFFSET,PORT2_OFFSET,PORT3_OFFSET,PORT4_OFFSET,
	TDI_DELAY_TIME,SKIP_LINES,BLANK_LINES,SHIFT_LINES,
	ADC_SAMPLES,CCD_ATTENUATION,TDI_SHUTTER,TDI_COMMAND,
	PORT1ADC_OFFSET,PORT2ADC_OFFSET,PORT3ADC_OFFSET,PORT4ADC_OFFSET,
	SPARE_PARAM4,SPARE_PARAM3,SPARE_PARAM2,SPARE_PARAM1,
	PARAMETER_COUNT };

//////////////////////////////////////////////////////////
// enum G2SysConfigIndex:
//	This enum contains names for each index into the G2 system configuration parameter description array
//	SysConfigDes and the system configuration parameter array SysConfig.
// @author Roger W. Cover
// @see CG2Camera#SysConfig
// @see CG2Camera#SysConfigDes
//////////////////////////////////////////////////////////
enum G2SysConfigIndex {INSTRUMENT_MODEL=0,INSTRUMENT_SERIAL_NUMBER,HARDWARE_REVISION,SERIAL_PHASING,
	SERIAL_SPLIT,SERIAL_SIZE,PARALLEL_PHASING,PARALLEL_SPLIT,
	PARALLEL_SIZE,PARALLEL_SHIFT_DELAY,NUMBER_OF_PORTS,SHUTTER_CLOSE_DELAY,
	CCD_TEMPERATURE_OFFSET,BACKPLATE_TEMPERATURE_OFFSET,CCD_TEMPERATURE_SETPOINT,DATA_WORD_SIZE,
	MPP_MODE_DISABLE,PORT_SELECT,SPARE_CONFIG14,SPARE_CONFIG13,
	SPARE_CONFIG12,SPARE_CONFIG11,SPARE_CONFIG10,SPARE_CONFIG9,
	SPARE_CONFIG8,SPARE_CONFIG7,SPARE_CONFIG6,SPARE_CONFIG5,
	UART_100_BYTE_HANDSHAKE,SPARE_CONFIG3,SPARE_CONFIG2,SPARE_CONFIG1,
	CONFIGURATION_COUNT };
//////////////////////////////////////////////////////////
// class CG2Camera:
//	This class implements the fields and methods for the G2 communications protocol.<br>
//	It inherits the low-level communications with the device driver from the CCamera class.
//
// @author Roger W. Cover
// @author Hans J. Meyer
//////////////////////////////////////////////////////////
class CG2Camera : public CCamera
	{
	public:
		CG2Camera();
		virtual ~CG2Camera();

		int GetStatus();
		int LoadConfigurationParameters();
		int LoadReadoutParameters();
		int MakeCurrentTheDefault();
		int SelectDefaultParameters();
		int SendConfigurationParameters();
		int SendReadoutParameters();
		int SendOneReadoutParameter(int offset,u32 value);
		int SendOneConfigurationParameter(int offset,u32 value);
		int FITSHeaderSize();
		int BuildFITSHeader(char *header,const char *date_str,const char *time_str,const char *comment);

		//////////////////////////////////////////////////////////
		// SysStatus: The system status parameters read from the camera.
		// @see CG2Camera#GetStatus
		// @see G2SysStatusIndex
		//////////////////////////////////////////////////////////
		u32 SysStatus[STATUS_COUNT];
		//////////////////////////////////////////////////////////
		// SysStatusStr: Pointers to text descriptions of the elements in SysStatus.
		// @see G2SysStatusIndex
		//////////////////////////////////////////////////////////
		char *SysStatusStr[STATUS_COUNT];
		//////////////////////////////////////////////////////////
		// SysReadout: The system readout parameters read from or written to the camera.
		// @see CG2Camera#LoadReadoutParameters
		// @see CG2Camera#SendReadoutParameters
		// @see CG2Camera#SendOneReadoutParameter
		// @see G2SysReadoutIndex
		//////////////////////////////////////////////////////////
		u32 SysReadout[PARAMETER_COUNT];
		//////////////////////////////////////////////////////////
		// SysReadoutDes: Structures containing descriptions of the elements in SysReadout.
		// @see G2SysReadoutIndex
		//////////////////////////////////////////////////////////
		parameter_description_type SysReadoutDes[PARAMETER_COUNT];
		//////////////////////////////////////////////////////////
		// SysConfig: The system configuration parameters read from or written to the camera.
		// @see CG2Camera#LoadConfigurationParameters
		// @see CG2Camera#SendConfigurationParameters
		// @see CG2Camera#SendOneConfigurationParameter
		// @see G2SysConfigIndex
		//////////////////////////////////////////////////////////
		u32 SysConfig[CONFIGURATION_COUNT];
		//////////////////////////////////////////////////////////
		// SysConfigDes: Structures containing descriptions of the elements in SysConfig.
		// @see G2SysConfigIndex
		//////////////////////////////////////////////////////////
		parameter_description_type SysConfigDes[CONFIGURATION_COUNT];
	private:
		int FITSParameters();
	};

#endif
// G3Camera.h: interface for the CG3Camera class.
// Copyright (C) Spectral Instruments, Inc., 2003
#ifndef _G3CAMERA_H_
#define _G3CAMERA_H_

//////////////////////////////////////////////////////////
// enum G3SysStatusIndex:
//	This enum contains names for each index into the G3 system status description array
//	SysStatusStr and the system status array SysStatus.
// @author Roger W. Cover
// @see CG3Camera#SysStatus
// @see CG3Camera#SysStatusStr
//////////////////////////////////////////////////////////
enum G3SysStatusIndex { 	G3_CCD_CHAMBER_PRESSURE=0,G3_LINE_PRESSURE,G3_SHUTTER_STATUS,G3_XIRQ_STATUS,
	G3_POWER_SUPPLY_TEMP,G3_CASE_TEMPERATURE,G3_CRYO1_TEMPERATURE,G3_CRYO2_TEMPERATURE,
	G3_COOL_PLATE_TEMP1,G3_COOL_PLATE_TEMP2,G3_VACUUM_PUMP_CURRENT,G3_VACUUM_PUMP_VOLTAGE,
	G3_VACUUM_PUMP_SPEED,G3_VACUUM_PUMP_TEMPERATURE,G3_VACUUM_PUMP_LIFE,G3_HKS_SW_VERSION,
	G3_HKS_FLAGS,G3_SPARE_SYS_STATUS5,G3_SPARE_SYS_STATUS4,G3_SPARE_SYS_STATUS3,
	G3_SPARE_SYS_STATUS2,G3_SPARE_SYS_STATUS1,
	G3_SYS_STATUS_COUNT };

//////////////////////////////////////////////////////////
// enum G3CCDStatusIndex:
//	This enum contains names for each index into the G3 CCD status description array CCDStatusStr.
// @author Roger W. Cover
// @see CG3Camera#CCDStatus
// @see CG3Camera#CCDStatusStr
//////////////////////////////////////////////////////////
enum G3CCDStatusIndex { 	G3_CCD_TEMPERATURE=0,G3_CCD_SPARE_STATUS1,
	G3_CCD_STATUS_COUNT };

//////////////////////////////////////////////////////////
// enum G3SysReadoutIndex:
//	This enum contains names for each index into the G3 system readout parameter description array
//	SysReadoutDes and the system readout parameter array SysReadout.
// @author Roger W. Cover
// @see CG3Camera#SysReadout
// @see CG3Camera#SysReadoutDes
//////////////////////////////////////////////////////////
enum G3SysReadoutIndex {	G3_FORMAT_MODE,G3_SERIAL_ORIGIN,G3_SERIAL_LENGTH,G3_SERIAL_POSTSCAN,
	G3_PARALLEL_ORIGIN,G3_PARALLEL_LENGTH,G3_PARALLEL_POSTSCAN,G3_EXPOSURE_TIME,
	G3_CONTINUOUS_CLEAR_DISABLE,G3_TDI_DELAY,G3_LINES_TO_SHIFT_ON_TRIGGER,G3_BLANK_LINES_PRESCAN,
	G3_COMMAND_ON_TRIGGER,G3_SYS_SPARE_FORMAT20,G3_SYS_SPARE_FORMAT19,G3_SYS_SPARE_FORMAT18,
	G3_SYS_SPARE_FORMAT17,G3_SYS_SPARE_FORMAT16,G3_SYS_SPARE_FORMAT15,G3_SYS_SPARE_FORMAT14,
	G3_SYS_SPARE_FORMAT13,G3_SYS_SPARE_FORMAT12,G3_SYS_SPARE_FORMAT11,G3_SYS_SPARE_FORMAT10,
	G3_SYS_SPARE_FORMAT9,G3_SYS_SPARE_FORMAT8,G3_SYS_SPARE_FORMAT7,G3_SYS_SPARE_FORMAT6,
	G3_SYS_SPARE_FORMAT5,G3_SYS_SPARE_FORMAT4,G3_SYS_SPARE_FORMAT3,G3_SYS_SPARE_FORMAT2,
	G3_SYS_SPARE_FORMAT1,
	G3_READOUT_MODE=0,G3_READOUT_SPEED,G3_SERIAL_BINNING,G3_PARALLEL_BINNING,
	G3_DSI_SAMPLE_TIME,G3_ANALOG_ATTENUATION,G3_CCD_ATTENUATION,G3_READOUT_SPARE5,
	G3_READOUT_SPARE4,G3_READOUT_SPARE3,G3_READOUT_SPARE2,G3_READOUT_SPARE1,
	G3_SYS_READOUT_COUNT };

//////////////////////////////////////////////////////////
// enum G3CCDReadoutIndex:
//	This enum contains names for each index into the G3 CCD readout parameter description array CCDReadoutDes.
// @author Roger W. Cover
// @see CG3Camera#CCDReadout
// @see CG3Camera#CCDReadoutDes
//////////////////////////////////////////////////////////
enum G3CCDReadoutIndex {	G3_CCD_PORT1_IMAGE_OFFSET=0,G3_CCD_PORT1_ADC_OFFSET,G3_CCD_PORT1_SENSITIVITY,G3_CCD_PORT1_SPARE,
	G3_CCD_PORT2_IMAGE_OFFSET,G3_CCD_PORT2_ADC_OFFSET,G3_CCD_PORT2_SENSITIVITY,G3_CCD_PORT2_SPARE,
	G3_CCD_PORT3_IMAGE_OFFSET,G3_CCD_PORT3_ADC_OFFSET,G3_CCD_PORT3_SENSITIVITY,G3_CCD_PORT3_SPARE,
	G3_CCD_PORT4_IMAGE_OFFSET,G3_CCD_PORT4_ADC_OFFSET,G3_CCD_PORT4_SENSITIVITY,G3_CCD_PORT4_SPARE,
	G3_CCD_READOUT_COUNT };

//////////////////////////////////////////////////////////
// enum G3SysConfigIndex:
//	This enum contains names for each index into the G3 system configuration description array
//	SysConfigDes and the configuration parameter array SysConfig.
// @author Roger W. Cover
// @see CG3Camera#SysConfig
// @see CG3Camera#SysConfigDes
//////////////////////////////////////////////////////////
enum G3SysConfigIndex {		G3_SYS_INSTRUMENT_MODEL=0,G3_SYS_INSTRUMENT_SERIAL_NUMBER,G3_SYS_HARDWARE_PART_NUM,
	G3_SYS_HARDWARE_REVISION,G3_SYS_SERIAL_PHASING,G3_SYS_SERIAL_SPLIT,
	G3_SYS_SERIAL_SIZE,G3_SYS_PARALLEL_PHASING,G3_SYS_PARALLEL_SPLIT,
	G3_SYS_PARALLEL_SIZE,G3_SYS_PARALLEL_SHIFT_DELAY,G3_SYS_PORTS_PER_CCD,
	G3_SYS_SHUTTER_CLOSE_DELAY,G3_SYS_MPP_MODE_DISABLE,G3_SYS_PORT_SELECT,
	G3_SYS_ROWS_OF_CCDS,G3_SYS_COLUMNS_OF_CCDS,G3_SYS_COOLER_ON,G3_SYS_CRYO1_TEMP_OFFSET,
	G3_SYS_CRYO1_TEMP_SETPOINT,G3_SYS_CRYO2_TEMP_OFFSET,G3_SYS_CRYO2_TEMP_SETPOINT,
	G3_SYS_CCD_ROW1_ENABLE,G3_SYS_CCD_ROW2_ENABLE,G3_SYS_CCD_ROW3_ENABLE,
	G3_SYS_CCD_ROW4_ENABLE,G3_SYS_ACQUISITION_MODE,G3_SYS_VAC_VALVE_ENABLE,
	G3_SYS_COMM_ACK_ENABLE,G3_SYS_CONFIG_SPARE17,G3_SYS_CONFIG_SPARE16,
	G3_SYS_CONFIG_SPARE15,G3_SYS_CONFIG_SPARE14,G3_SYS_CONFIG_SPARE13,
	G3_SYS_CONFIG_SPARE12,G3_SYS_CONFIG_SPARE11,G3_SYS_CONFIG_SPARE10,
	G3_SYS_CONFIG_SPARE9,G3_SYS_CONFIG_SPARE8,G3_SYS_CONFIG_SPARE7,
	G3_SYS_CONFIG_SPARE6,G3_SYS_CONFIG_SPARE5,G3_SYS_CONFIG_SPARE4,
	G3_SYS_CONFIG_SPARE3,G3_SYS_CONFIG_SPARE2,G3_SYS_CONFIG_SPARE1,
	G3_SYS_CONFIG_COUNT };

//////////////////////////////////////////////////////////
// enum G3CCDConfigIndex:
//	This enum contains names for each index into the G3 CCD configuration description array CCDConfigDes.
// @author Roger W. Cover
// @see CG3Camera#CCDConfig
// @see CG3Camera#CCDConfigDes
//////////////////////////////////////////////////////////
enum G3CCDConfigIndex { 	G3_CCD_TEMP_SETPOINT_OFFSET=0,G3_CCD_TEMP_SETPOINT,G3_CCD_CONFIG_SPARE1,
	G3_CCD_CONFIG_COUNT };

//////////////////////////////////////////////////////////
// class CG3Camera:
//	This class implements the fields and methods for the G3 communications protocol.<br>
//	It inherits the low-level communications with the device driver from the CCamera class.
// @author Roger W. Cover
//////////////////////////////////////////////////////////
class CG3Camera : public CCamera  
	{
	public:
		CG3Camera();
		virtual ~CG3Camera();

		int LoadConfigurationParameters();		// Call this first: it allocates RAM for the CCD structures.
		int SendConfigurationParameters();
		int LoadReadoutParameters();
		int SendReadoutParameters();
		int GetStatus();
		int MakeCurrentTheDefault();
		int SelectDefaultParameters();
		int MakeCurrentReadoutSelectableAs(int mode);
		int SelectReadoutMode(int mode);
		int DeleteReadoutMode(int mode);
		int MakeCurrentFormatSelectableAs(int mode);
		int SelectFormatMode(int mode);
		int DeleteFormatMode(int mode);
		int StoreTextString(char *text,u16 size);
		int RetrieveTextStringSize(int *size);
		int RetrieveTextString(char *text);
		int FITSHeaderSize();
		int BuildFITSHeader(char *header,const char *date_str,const char *time_str,const char *comment);

		//////////////////////////////////////////////////////////
		// total_ccds: Calculated total number of CCDs in this camera.<br>
		//	Set by LoadConfigurationParameters().
		// @see CG3Camera#LoadConfigurationParameters
		//////////////////////////////////////////////////////////
		int total_ccds;
		//////////////////////////////////////////////////////////
		// SysConfig: The system configuration parameters read from or written to the camera.
		// @see CG3Camera#LoadConfigurationParameters
		// @see CG3Camera#SendConfigurationParameters
		// @see G3SysConfigIndex
		//////////////////////////////////////////////////////////
		u32 SysConfig[G3_SYS_CONFIG_COUNT];
		//////////////////////////////////////////////////////////
		// SysConfigDes: Structures containing descriptions of the elements in SysConfig.
		// @see G3SysConfigIndex
		//////////////////////////////////////////////////////////
		parameter_description_type SysConfigDes[G3_SYS_CONFIG_COUNT];
		//////////////////////////////////////////////////////////
		// CCDConfig: A dynamically allocated array of the CCD configuration parameters read from or written to the camera.<br>
		//	There will be (G3_CCD_CONFIG_COUNT * total_ccds) elements in this array after calling LoadConfigurationParameters().
		// @see CG3Camera#LoadConfigurationParameters
		// @see CG3Camera#SendConfigurationParameters
		// @see G3CCDConfigIndex
		//////////////////////////////////////////////////////////
		u32_ptr CCDConfig;
		//////////////////////////////////////////////////////////
		// CCDConfigDes: Structures containing descriptions of the elements in CCDConfig.
		//	There are only G3_CCD_CONFIG_COUNT elements in this array.
		// @see G3CCDConfigIndex
		//////////////////////////////////////////////////////////
		parameter_description_type CCDConfigDes[G3_CCD_CONFIG_COUNT];
		//////////////////////////////////////////////////////////
		// SysReadout: The system readout parameters read from or written to the camera.
		// @see CG3Camera#LoadReadoutParameters
		// @see CG3Camera#SendReadoutParameters
		// @see G3SysReadoutIndex
		//////////////////////////////////////////////////////////
		u32 SysReadout[G3_SYS_READOUT_COUNT];
		//////////////////////////////////////////////////////////
		// SysReadoutDes: Structures containing descriptions of the elements in SysReadout.
		// @see G3SysReadoutIndex
		//////////////////////////////////////////////////////////
		parameter_description_type SysReadoutDes[G3_SYS_READOUT_COUNT];
		//////////////////////////////////////////////////////////
		// CCDReadout: A dynamically allocated array of the CCD readout parameters read from or written to the camera.<br>
		// There will be (G3_CCD_READOUT_COUNT * total_ccds) elements in this array after calling LoadConfigurationParameters().
		// @see CG3Camera#LoadConfigurationParameters
		// @see CG3Camera#LoadReadoutParameters
		// @see CG3Camera#SendReadoutParameters
		// @see G3CCDReadoutIndex
		//////////////////////////////////////////////////////////
		u32_ptr CCDReadout;
		//////////////////////////////////////////////////////////
		// CCDReadoutDes: Structures containing descriptions of the elements in CCDReadout.<br>
		//	There are only G3_CCD_READOUT_COUNT elements in this array.
		// @see G3CCDReadoutIndex
		//////////////////////////////////////////////////////////
		parameter_description_type CCDReadoutDes[G3_CCD_READOUT_COUNT];
		//////////////////////////////////////////////////////////
		// SysStatus: The system status parameters read from the camera.
		// @see CG3Camera#GetStatus
		// @see G3SysStatusIndex
		//////////////////////////////////////////////////////////
		u32 SysStatus[G3_SYS_STATUS_COUNT];
		//////////////////////////////////////////////////////////
		// SysStatusStr: Pointers to text descriptions of the elements in SysStatus.
		// @see G3SysStatusIndex
		//////////////////////////////////////////////////////////
		char *SysStatusStr[G3_SYS_STATUS_COUNT];
		//////////////////////////////////////////////////////////
		// CCDStatus: A dynamically allocated array of the CCD status parameters read from the camera.<br>
		// There will be (G3_CCD_STATUS_COUNT * total_ccds) elements in this array after calling LoadConfigurationParameters().
		// @see CG3Camera#LoadConfigurationParameters
		// @see CG3Camera#GetStatus
		// @see G3CCDStatusIndex
		//////////////////////////////////////////////////////////
		u32_ptr CCDStatus;
		//////////////////////////////////////////////////////////
		// CCDStatusStr: Pointers to the text descriptions of the elements in CCDStatus.<br>
		//	There are only G3_CCD_STATUS_COUNT elements in this array.
		// @see G3CCDStatusIndex
		//////////////////////////////////////////////////////////
		char *CCDStatusStr[G3_CCD_STATUS_COUNT];
	private:
		void free_ccd_data_ram();
		int FITSParameters();
		int ReadEEPROMBlock(int block,char *data);
		int WriteEEPROMBlock(int block,char *data);
	};

#endif
// G4Camera.h: interface for the CG4Camera class.
// Copyright (C) Spectral Instruments, Inc., 2005
#ifndef _G4CAMERA_H_
#define _G4CAMERA_H_

//////////////////////////////////////////////////////////
// enum G4CapabilityMasks:
//	Describes the bit masks for CapabilityMask.<br>
//	G4_TRIG_SIGNALLED: the external trigger received signal will be sent to the host computer.<br>
//	G4_FRAMING: the multi-frame readout functions are supported.
// @see CG4Camera#CapabilityMask
// @author Roger W. Cover
//////////////////////////////////////////////////////////
enum G4CapabilityMasks { G4_TRIG_SIGNALLED=1,G4_FRAMING=2 };

//////////////////////////////////////////////////////////
// struct ExtStatDesType:
//	This structure describes extended status parameters for the user application.
// @author Roger W. Cover
//////////////////////////////////////////////////////////
struct ExtStatDesType {
	//////////////////////////////////////////////////////////
	// description: The user-readable text that identifies this parameter
	//////////////////////////////////////////////////////////
	char *description;
	//////////////////////////////////////////////////////////
	// des: The machine-readable description of the parameter
	// @see g4_description_type
	//////////////////////////////////////////////////////////
	g4_description_type des;
	};

//////////////////////////////////////////////////////////
// class CG4Camera:
//	This class implements the fields and methods for the G4 communications protocol.<br>
//	It inherits fields and methods from the CG2Camera class.
//
// @author Roger W. Cover
//////////////////////////////////////////////////////////
class CG4Camera : public CG2Camera
	{
	public:
		CG4Camera();
		virtual ~CG4Camera();

		int GetCapabilities();
		int GetExtendedStatus();
		int SaveCurrentToMode(int mode,u32 ReadoutRestoreMask,u32 ConfigurationRestoreMask);
		int RestoreMode(int mode,u32 ReadoutRestoreMask,u32 ConfigurationRestoreMask);
		int CheckMode(int mode,u32 *ReadoutRestoreMask,u32 *ConfigurationRestoreMask,bool *HasData);
		int FITSHeaderSize();
		int BuildFITSHeader(char *header,const char *date_str,const char *time_str,const char *comment);

		//////////////////////////////////////////////////////////
		// MinCommandByte: The lowest ASCII code recognized as a command by the camera.
		//////////////////////////////////////////////////////////
		char MinCommandByte;
		//////////////////////////////////////////////////////////
		// MaxCommandByte: The highest ASCII code recognized as a command by the camera.
		//////////////////////////////////////////////////////////
		char MaxCommandByte;
		//////////////////////////////////////////////////////////
		// AvailableCommandMask: A mask with a bit set for each command the camera can process.<br>
		//	Bit 0 (the LSB) corresponds to the command contained in MinCommandByte. Bit 31 (the MSB)
		//	corresponds to the command contained in MaxCommandByte.
		//	@see CG4Camera#MinCommandByte
		//	@see CG4Camera#MaxCommandByte
		//////////////////////////////////////////////////////////
		u32 AvailableCommandMask;
		//////////////////////////////////////////////////////////
		// CapabilityMask: A mask indicating additional capabilities of the camera.
		//	@see G4CapabilityMasks
		//////////////////////////////////////////////////////////
		u32 CapabilityMask;
		//////////////////////////////////////////////////////////
		// SupportedModes: The number of modes supported by the mode maintenance functions.
		// @see CG4Camera#SaveCurrentToMode
		// @see CG4Camera#RestoreMode
		// @see CG4Camera#CheckMode
		//////////////////////////////////////////////////////////
		u32 SupportedModes;
		//////////////////////////////////////////////////////////
		// DefaultReadoutRestoreMask: This is a suggested restore mask for the Readout Parameter table.
		//////////////////////////////////////////////////////////
		u32 DefaultReadoutRestoreMask;
		//////////////////////////////////////////////////////////
		// DefaultConfigurationRestoreMask: This is a suggested restore mask for the Configuration Parameter table.
		//////////////////////////////////////////////////////////
		u32 DefaultConfigurationRestoreMask;
		//////////////////////////////////////////////////////////
		// SysStatusDes: Structures containing descriptions of the parameters in SysStatus.
		//////////////////////////////////////////////////////////
		g4_description_type SysStatusDes[STATUS_COUNT];
		//////////////////////////////////////////////////////////
		// ExtStatusEntries: The number of entries in the extended status data words array.<br>
		//	Zero until GetCapabilities() or GetExtendedStatus are called.
		//	@see CG4Camera#GetCapabilities
		//	@see CG4Camera#GetExtendedStatus
		//////////////////////////////////////////////////////////
		u32 ExtStatusEntries;
		//////////////////////////////////////////////////////////
		// ExtStatus: Points to the extended status data words.<br>
		//	NULL until GetCapabilities() or GetExtendedStatus are called.
		//	@see CG4Camera#GetCapabilities
		//	@see CG4Camera#GetExtendedStatus
		//////////////////////////////////////////////////////////
		i32_ptr ExtStatus;
		//////////////////////////////////////////////////////////
		// ExtStatusDes: Points to structures containing descriptions of the parameters in ExtStatus.<br>
		//	NULL until GetCapabilities() or GetExtendedStatus are called.
		//	This structure is only filled in by GetCapabilities().
		//	@see CG4Camera#GetCapabilities
		//	@see CG4Camera#GetExtendedStatus
		//////////////////////////////////////////////////////////
		ExtStatDesType *ExtStatusDes;
	private:
		int FITSParameters();
	};

#endif
// CameraInfo.h: interface for the CCameraInfo class.
// Copyright (C) Spectral Instruments, Inc., 2003
#ifndef _CAMERAINFO_H_
#define _CAMERAINFO_H_

#define NO_CAMERA_DETECTED		0	// Used in the model field to indicate no camera.
#define CAMERA_UNRESPONSIVE		1	// Used in the model field to indicate the camera did not send parameters.
#define CAMERA_EEPROM_UNCONFIG	2	// Used in the model field to indicate a who's EEPROM is not configured.

//////////////////////////////////////////////////////////
// class CCameraInfo:
//	This class implements the members required by the camera data base.<br>
//	An array of these objects makes up the datbase used by the CCameraDB class to report
//	on the existence of active camera interfaces in the computer.
// @author Roger W. Cover
// @see CCameraDB
//////////////////////////////////////////////////////////
class CCameraInfo  
	{
	public:
		CCameraInfo();
		virtual ~CCameraInfo();

		//////////////////////////////////////////////////////////
		// device_name: string representing the device name.<br>
		//	This string is sent to the OpenCamera method of the relevant camera class object.
		//	@see CG2Camera
		//	@see CG3Camera
		//////////////////////////////////////////////////////////
		char device_name[MAX_CAMERA_DRIVER_NAME];
		//////////////////////////////////////////////////////////
		// driver_present: flag for device driver present.<br>
		//	Any Spectral Instruments PCI interface card device driver that opens will cause this value to be TRUE.
		//	This only requires that the device driver is installed and the PCI card is in the bus.
		//	If the camera is off or disconnected, this will still be TRUE.
		//////////////////////////////////////////////////////////
		bool driver_present;
		//////////////////////////////////////////////////////////
		// model: the model number of the detected camera: 500, 600, 800, 900, 1000, etc.<br>
		//	If a camera is not detected, model will be set to NO_CAMERA_DETECTED.<br>
		//	If a camera is detected but communication fails, model will be CAMERA_UNRESPONSIVE.<br>
		//	If a camera is detected and communication returns a model number of zero, model will be CAMERA_EEPROM_UNCONFIG.
		//////////////////////////////////////////////////////////
		int model;	
		//////////////////////////////////////////////////////////
		// serial_number: serial number of the detected camera.<br>
		//	If no camera is detected, ignore this field.
		//////////////////////////////////////////////////////////
		int serial_number;
		//////////////////////////////////////////////////////////
		// baud_rate: Baud rate used to communicate with the camera.
		//////////////////////////////////////////////////////////
		int baud_rate;
		//////////////////////////////////////////////////////////
		// mt: The media type for the connection. One of enum media_type's entries.
		//////////////////////////////////////////////////////////
		media_type mt;
	};

#endif
// CameraDB.h: interface for the CCameraDB class.
// Copyright (C) Spectral Instruments, Inc., 2003
#ifndef _CAMERADB_H_
#define _CAMERADB_H_

//////////////////////////////////////////////////////////
// class CCameraDB:
//	This class implements the methods needed to search for Spectral Instruments camera interfaces.<br>
//	The result of this search is a table of CCameraInfo objects that contain the information needed to use all
//	the cameras connected to the computer (up to 10 cameras).
// @author Roger W. Cover
// @author Hans J. Meyer
// @see CCameraInfo
//////////////////////////////////////////////////////////
class CCameraDB  
	{
	public:
#define MAX_CAM				10		// Maximum # of camera interfaces supported.

		//////////////////////////////////////////////////////////
		// camera: This array contains the information required to open and connect to the cameras.<br>
		//	If a camera was off or disconnected when ScanInterfaces() was run, you can call ScanOneInterface()
		//	to determine the required Baud rate and media type at a later time.
		// @see CCameraDB#ScanInterfaces
		// @see CCameraDB#ScanOneInterface
		//////////////////////////////////////////////////////////
		CCameraInfo camera[MAX_CAM];
		//////////////////////////////////////////////////////////
		// interfaces_found: This field contains the number of PCI interface card device drivers that were found by ScanInterfaces().<br>
		// @see CCameraDB#ScanInterfaces
		//////////////////////////////////////////////////////////
		int	interfaces_found;

	public:
		CCameraDB();
		virtual ~CCameraDB();

		int ScanInterfaces(bool Skip600SeriesTest);
		int ScanOneInterface(int index,bool Skip600SeriesTest);
	private:
		int CheckConfig(int index,CG2Camera *camera);
	};

#endif
