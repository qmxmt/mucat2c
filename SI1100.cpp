#include "StdAfx.h"
#include "SI1100.h"
#include "843.h"
#include <valarray>
#include <fstream>
//#include  "MuCatView.h"
using namespace std;
SI1100 Camera;

SI1100::SI1100(void)
	:m_NumRows(0), m_NumColumns(0)
	{
	CoolerOn();
	}


SI1100::~SI1100(void)
	{
	}


// Allocates space for a camera frame
int SI1100::NewRaw(unsigned short Xsize, unsigned short Ysize)
	{
	if (m_pRawImage)	DeleteRaw();			// delete old array
	m_ImgHandle = GlobalAlloc(GPTR, Xsize*Ysize * sizeof(unsigned short));
	m_pRawImage = (unsigned short *)GlobalLock(m_ImgHandle);

	return (m_pRawImage?0:1);
	}


// Delete the pointer to old image and free up RAM
void SI1100::DeleteRaw(void)
	{
	if (m_pRawImage)
		{
		GlobalUnlock(m_ImgHandle);			// release image buffer
		GlobalFree(m_ImgHandle);
		m_pRawImage = NULL;
		}
	}

//int SI1100::TDIReadout(unsigned long ** image, unsigned short *TDIBuffer, unsigned short **TDIArray, int XSize, int YSize, int Bin, double StepSize, bool IsDark, bool ShutterOpen, int FrameNum, int NumFrames,unsigned int TDIDelay, int direction,vector<float>  pixel_mag_lookup)
int SI1100::TDIReadout(unsigned long ** image, int XSize, int YSize, int Bin, double StepSize, bool IsDark, bool ShutterOpen, int FrameNum, int NumFrames,unsigned int TDIDelay, int direction,vector<float>  pixel_mag_lookup)
	{
//	BOOL *PepperFlag;
	int Error = 0;
//	unsigned short *pPercentDone;
//	unsigned long *pPointerPos;
	unsigned short *TDIBuffer, **TDIArray; //*CCDLine,*TDILine,,  *Buffer
	INT32 a,b,temp, NumRows;//,y0,y1,y2,yend,ystart,BytesRead,Status,xend,xstart;
	INT32 xi,yi;
	//int CCDXSize, CCDYSize;
	int Bin2;
	float x,y;//yf,frac;
	//unsigned short *TDIBuffer,**TDIArray, *CCDLine, *TDILine;
	//unsigned short *CCDLine, *TDILine;
//	double Position;
	int ports, ser;
	di_mode	di_md;
	u32 test,pixels_read,pixels_deinterlaced;
	u8 percent_readout;
	bool dma_complete, frame_complete;
	float scale;
	CTime TimeOut;
	CTime TimeNow;

	NumRows = XSize * Bin + 4;
	Bin2 = Bin*Bin;

	TDIBuffer = new unsigned short[NumRows * YSize * Bin *2];
	TDIArray  = new unsigned short*[NumRows];
	for (a=0; a<NumRows; ++a)
		TDIArray[a] = &TDIBuffer[a*YSize*Bin];



	LoadConfigurationParameters();
	LoadReadoutParameters();

	if (m_NumColumns == 0)
		{m_NumColumns = 2000;}

	bool pPhase = 1;
	bool sPhase = 0;
	int ports_used = 6;		//4 SEEMS to be ports A and B. 6 SEEMS to be ports C and D. Emperical evidence only!

	ports = SysConfig[NUMBER_OF_PORTS];	
	SysConfig[PORT_SELECT] = ports_used;
	SysReadout[SERIAL_ORIGIN]= 2063 - m_NumColumns; 
	SysReadout[SERIAL_READ_LENGTH] = m_NumColumns/2;
	SysReadout[SERIAL_BINNING] = 2;
	SysReadout[SERIAL_POSTSCAN] = 0;//2048 - m_NumColumns;
	SysReadout[PARALLEL_ORIGIN] = 2060;// m_NumRows + 9;
	SysReadout[PARALLEL_READ_LENGTH] = NumRows;
	SysReadout[PARALLEL_BINNING] = 2;
	SysReadout[PARALLEL_POSTSCAN] = 0;
	SysReadout[DSI_SAMPLE_TIME] = 2;//2;// 2 is from SI specs and email. 10;  
	SysReadout[TDI_DELAY_TIME] = TDIDelay;

	SysReadout[PORT1_OFFSET] = 3291;//PORT1OFFSET;
	SysReadout[PORT2_OFFSET] = 3681;//PORT2OFFSET;
	SysReadout[PORT3_OFFSET] = 3500;//PORT3OFFSET;
	SysReadout[PORT4_OFFSET] = 3860;//PORT4OFFSET
	SysReadout[PORT1ADC_OFFSET] = 35241;
	SysReadout[PORT2ADC_OFFSET] = 36406; 
	SysReadout[PORT3ADC_OFFSET] = 36787;
	SysReadout[PORT4ADC_OFFSET] = 36215;
	SysReadout[CCD_ATTENUATION] = 1; //1 = high dynamic range, 0 is high sensitivity accoring to SI Image software

	//SysReadout[EXPOSURE_TIME] = (DWORD) 6000; //check
	/*	Serial_Phase	Serial_Split	Action
	0				0			Shift to A and C outputs
	1				0			Shift to B and D outputs
	0				1			Split both serial registers and shift to all 4 outputs
	1				1			Invalid!
	*/

	SysConfig[SERIAL_SPLIT] = 1; //1
	SysConfig[SERIAL_PHASING] = sPhase; //0
	SysConfig[PARALLEL_SPLIT] = 0;


	if(direction == 1)	//direction == 1 we are going CW, so read out to ports C and D
		{
		SysConfig[PARALLEL_PHASING] = 1;
		SysConfig[PORT_SELECT] = 6;
		}
	else
		{
		SysConfig[PARALLEL_PHASING] = 0;
		//			SysConfig[SERIAL_PHASING] = 0;
		SysConfig[PORT_SELECT] = 4;
		}
	//SysConfig[PARALLEL_PHASING] = pPhase;
	//else
	//	if (direction = READ_ANTICLOCKWISE)
	//SysConfig[PARALLEL_PHASING] = 1;
	//	else
	//		Texit("Error setting parallel phasing");

	if (IsDark)
		{
		//TRACE("Setting #22 to 1");
		SysReadout[TDI_SHUTTER] = 1;
		}
	else
		{
		//TRACE("Setting #22 to 2");
		SysReadout[TDI_SHUTTER] = 0;
		}
	if (ShutterOpen)
		//SysReadout[TDISHUTTER] = 2;
		SysReadout[TDI_SHUTTER] = 2;

	SysReadout[TDI_COMMAND] = 26;
	SendReadoutParameters();

	SendConfigurationParameters();


	LoadConfigurationParameters();							
	ports = SysConfig[NUMBER_OF_PORTS];

	di_md = DI_S_SPLIT;								// 2 ports on same side, Serial Split
	ser = SysReadout[SERIAL_READ_LENGTH] * 2;		// if Serial is split, double the Serial length


	Error = DmaStatus(&dma_complete,&frame_complete,&test);
	if( !dma_complete && frame_complete ) {	// A DMA transaction is still open but the frame is done.

		Error = EndAcquisition(true);
		Error = DmaStatus(&dma_complete,&frame_complete,&test);
		}


	Error = PrepareAcquisition(m_NumColumns, NumRows, TDIBuffer, READ_NORMAL, di_md, NULL, NULL);
	LoadReadoutParameters();							//Get back the ReadOut Params incorperating the changes we just uploaded to the camera.
	ports = SysConfig[NUMBER_OF_PORTS];
	if( Error != E_NO_ERROR ) {
		EndAcquisition(false);
		ErrorSwitch(Error);
		//Texit("Error preparing camera acquisition");
		}



	
	SendCommand('U'); //Triggered external clocked TDI - Z is non-triggered external clocked TDI

	C843::DoProfile();
	double DXCentre, DYCentre, TXCentre, TYCentre, MXCentre; // centres for display and TDI arrays
	DXCentre = (XSize - 1.0)/2.0;
	DYCentre = (YSize - 1.0)/2.0;
	TXCentre = (XSize*Bin - 1.0)/2.0;
	TYCentre = (YSize*Bin - 1.0)/2.0;
	//MXCentre = (XSize - 1.0)/2.0;
	MXCentre = 3999.5;///(pixel_mag_lookup.size()-1)/2;

	TimeNow = CTime::GetCurrentTime();// + CTimeSpan(0,0,45,0);
					
	//fstream file ("scale_look_up.bin",ios::in|ios::out|ios::binary|ios::trunc);
	do 
		{
		TimeOut = CTime::GetCurrentTime();
		Error = ReadoutStatus(&pixels_read,&pixels_deinterlaced,&percent_readout);
		Error = DmaStatus(&dma_complete,&frame_complete,&test);
		if (TimeOut > TimeNow + CTimeSpan(0,0,1,30))
			{
			EndAcquisition(true);	//don't leave camera hanging for a trigger if we fail to readout because stages freeze
			break;
			}
		} while (percent_readout < 100);			
		
		Error = EndAcquisition(false);
		if (Error)
			ErrorSwitch(Error);
		//Texit("Camera data acquisition error");

		if (Error == E_NO_ERROR)
			{
			TRACE("\nDirection = %d\n",direction);
			TRACE("Mag Lookup size = %d",pixel_mag_lookup.size());
			for (a=0; a<XSize; ++a)
				{
				for (b=0; b<YSize; ++b)
					{
					if (FrameNum == 0)
						image[b][a] = 0;


					int PMindex = b*8000+MXCentre-DXCentre+a;

					scale = pixel_mag_lookup[PMindex]*2.0;
					//if (file.is_open())
					//	file << scale;
					//scale = 2.0;
					x = (direction) * scale*(a - DXCentre) + TXCentre;		//TODO put real scale factors in here
					y = scale*(b - DYCentre) + TYCentre;
					xi = INT32(x);
					yi = INT32(y);
					temp = (yi+1-y)*((xi+1-x)*TDIArray[xi][yi] + TDIArray[xi+1][yi] + (x-xi)*TDIArray[xi+2][yi]);
					temp +=(xi+1-x)*TDIArray[xi][yi+1] + TDIArray[xi+1][yi+1] + (x-xi)*TDIArray[xi+2][yi+1];
					temp +=(y-yi)*((xi+1-x)*TDIArray[xi][yi+2] + TDIArray[xi+1][yi+2] + (x-xi)*TDIArray[xi+2][yi+2]);

					image[b][a] += (unsigned short)(temp/4);
					//if (FrameNum == (NumFrames-1))
					//	image[b][a] /= NumFrames;
					}
				}

			//file.close();
			delete  TDIArray;
			delete  TDIBuffer;

			}

		return Error;
	}


void SI1100::Texit(char* Message)
	{
	::MessageBox(NULL,Message,"Error",MB_OK|MB_ICONHAND);
	exit(1);
	}


int SI1100::ImageInternalTDI(void)
	{
	if (!m_DmaActive)									// error if no DMA active
		return E_DMA_NOT_ACTIVE;
	return SendCommand('U');
	}


void SI1100::ExposeandReadoutRegion(float exposure, unsigned long ** image, int X, int Y, int XSize, int YSize, int Bin)
	{
	int SerialReadLength = YSize/2*Bin;
	INT32 a,b,x,y,xstart,xend,ystart,yend,temp, NumRows;//Status,BytesRead,
	int ParallelReadLength = NumRows = XSize*Bin;
	int Bin2;
	unsigned short *TDIBuffer, **TDIArray;//, *CCDLine, *TDILine;
//	double Position;
	Bin2 = Bin*Bin;
	TDIBuffer = new unsigned short[NumRows * YSize * Bin];
	TDIArray  = new unsigned short*[NumRows];
	for (a=0; a<NumRows; ++a)
		TDIArray[a] = &TDIBuffer[a*YSize*Bin];

	SysReadout[EXPOSURE_TIME]= (DWORD) exposure; 
	SysReadout[SERIAL_ORIGIN]= 8 + Y*2;
	SysReadout[SERIAL_READ_LENGTH] = SerialReadLength;
	SysReadout[SERIAL_BINNING] = 2;
	SysReadout[SERIAL_POSTSCAN] = 2048 - SerialReadLength*2 - SysReadout[SERIAL_ORIGIN];
	SysReadout[PARALLEL_ORIGIN] = 8 + X*2;
	SysReadout[PARALLEL_READ_LENGTH] = ParallelReadLength;
	SysReadout[PARALLEL_BINNING] = 2;
	SysReadout[PARALLEL_POSTSCAN] = 4097 - ParallelReadLength*2 - SysReadout[PARALLEL_ORIGIN];

	SendReadoutParameters();
	PrepareAcquisition(SerialReadLength*2,ParallelReadLength,TDIBuffer,READ_NORMAL,DI_S_SPLIT,0,NULL);

	ImageExpose();

	while(DMAStatus())
		if (EndAcquisition(TRUE))     
			Texit("Camera DAQ Error");

	for (a=0; a<XSize; ++a)
		{
		xstart = (XSize-1-a)*Bin;
		xend = xstart + Bin;
		for (b=0; b<YSize; ++b)
			{
			temp = 0;
			ystart = b*Bin;
			yend = ystart + Bin;
			for (x=xstart; x<xend; ++x)
				{
				for (y = ystart; y < yend; ++y)
					temp += TDIArray[x][y];
				}
			image[b][a] = (unsigned short)(temp/Bin2);
			}
		}
	delete TDIBuffer;
	delete TDIArray;
	}


int SI1100::DMAStatus(void)
	{
	bool	dma_complete,frame_complete;
	DWORD framecount;
	return DmaStatus(&dma_complete,&frame_complete, &framecount);
	}


void SI1100::StillImage(float exposure, unsigned long ** image , int XSize, int YSize, int Bin, vector<float>  pixel_mag_lookup, bool IsDark)
	{

	int SerialReadLength = YSize*Bin/2;
	INT32 a,b,xi,yi,xstart,xend,ystart,yend,temp, NumRows;//Status,BytesRead,
	float y,x;
	int Bin2;
	unsigned short *TDIBuffer, **TDIArray;// *CCDLine, *TDILine, *Buffer;
//	double Position;
	int error;
	u32 test,pixels_read,pixels_deinterlaced;
	u8 percent_readout;
	bool dma_complete, frame_complete;
	NumRows = XSize * Bin;
	Bin2 = Bin*Bin;
	float scale;

	//valarray<float> scale(XSize,0.9);	//fixed scalefactor for test

	/*NewRaw(XSize,YSize);
	Buffer = GetImagePtr();*/

	TDIBuffer = new unsigned short[NumRows * YSize * Bin *2];
	TDIArray  = new unsigned short*[NumRows];
	for (a=0; a<NumRows; ++a)
		TDIArray[a] = &TDIBuffer[a*YSize*Bin];

	LoadConfigurationParameters();
	LoadReadoutParameters();

	/*ports = SysConfig[NUMBER_OF_PORTS];
	SendOneReadoutParameter(EXPOSURE_TIME,exposure);*/

	bool pPhase = 0;
	bool sPhase = 0;	

	if(IsDark) {exposure = 0;}

	SysReadout[EXPOSURE_TIME] = (DWORD) exposure;
	SysReadout[SERIAL_ORIGIN] = 64;// 2063 - SerialReadLength*2; //2098
	SysReadout[SERIAL_READ_LENGTH] = 1000;//SerialReadLength;
	SysReadout[SERIAL_BINNING] = 2;
	SysReadout[SERIAL_POSTSCAN] = 0;//2048 - SerialReadLength*2;
	SysReadout[PARALLEL_ORIGIN] = 40;//57;//57;
	SysReadout[PARALLEL_READ_LENGTH] = 1020;//NumRows;
	SysReadout[PARALLEL_BINNING] = 2;
	SysReadout[PARALLEL_POSTSCAN] = 0;//2080;//4089 - NumRows*2+57;
	SysReadout[PORT1_OFFSET] = 2122;//PORT1OFFSET;
	SysReadout[PORT2_OFFSET] = 2329;//PORT2OFFSET;
	SysReadout[PORT3_OFFSET] = 2012;//PORT2OFFSET;
	SysReadout[PORT4_OFFSET] = 2134;
	SysConfig[NUMBER_OF_PORTS] = 3;
	SysConfig[PORT_SELECT] = 10;
	SysReadout[DSI_SAMPLE_TIME] = 20;//20;  

	/*	Serial_Phase	Serial_Split	Action
	0				0			Shift to A and C outputs
	1				0			Shift to B and D outputs
	0				1			Split both serial registers and shift to all 4 outputs
	1				1			Invalid!
	*/

	SysConfig[SERIAL_SPLIT] = 1;
	SysConfig[SERIAL_PHASING] = sPhase;
	SysConfig[PARALLEL_SPLIT] = 1;
	SysConfig[PARALLEL_PHASING] = pPhase;
	error = SendConfigurationParameters();
	error = SendReadoutParameters();

	error = DmaStatus(&dma_complete,&frame_complete,&test);
	if( !dma_complete && frame_complete ) {	// A DMA transaction is still open but the frame is done.

		error = EndAcquisition(true);
		error = DmaStatus(&dma_complete,&frame_complete,&test);
		}

	
	error = PrepareAcquisition(SerialReadLength*2,NumRows,TDIBuffer,READ_NORMAL,DI_4QUAD,0,NULL);
	dma_complete = true;
	error = ImageExpose();
	CTime TimeOut;
	CTime TimeNow;
	TimeNow = CTime::GetCurrentTime();
	
	do 
		{
		TimeOut = CTime::GetCurrentTime();
		
		error = ReadoutStatus(&pixels_read,&pixels_deinterlaced,&percent_readout);
		if (TimeOut > TimeNow + CTimeSpan(0,0,1,30))
			{
			EndAcquisition(true);	//don't leave camera hanging for a trigger if we fail to readout because stages freeze
			break;
			}
		} while (percent_readout < 100);				
		error = EndAcquisition(false);
		if (error)
			Texit("Camera data acquisition error");


		double DXCentre, DYCentre, TXCentre, TYCentre; // centres for display and TDI arrays
		DXCentre = (XSize - 1.0)/2.0;
		DYCentre = (YSize - 1.0)/2.0;
		TXCentre = (XSize*Bin - 1.0)/2.0;
		TYCentre = (YSize*Bin - 1.0)/2.0;		
		double MXCentre = 3999.5;///(pixel_mag_lookup.size()-1)/2;		
		bool newbinmode = true;

		if (newbinmode)
			{

			for (a=0; a<XSize-1; ++a)
				{
				//TRACE("a = %d\n",a);
				for (b=0; b<YSize; ++b)
					{
					int PMindex = b*8000+MXCentre-DXCentre+a;

					scale = pixel_mag_lookup[PMindex]*2.0;

					x = scale*(a - DXCentre) + TXCentre;		//TODO put real scale factors in here
					y = scale*(b - DYCentre) + TYCentre;

					xi = INT32(x);
					yi = INT32(y);
					temp = (yi+1-y)*((xi+1-x)*TDIArray[xi][yi] + TDIArray[xi+1][yi] + (x-xi)*TDIArray[xi+2][yi]);
					temp +=(xi+1-x)*TDIArray[xi][yi+1] + TDIArray[xi+1][yi+1] + (x-xi)*TDIArray[xi+2][yi+1];
					temp +=(y-yi)*((xi+1-x)*TDIArray[xi][yi+2] + TDIArray[xi+1][yi+2] + (x-xi)*TDIArray[xi+2][yi+2]);

					image[b][a] = (unsigned short)(temp/4);
					}
				}
			delete  TDIArray;
			delete  TDIBuffer;

			}
		else
			{


			for (a=0; a<XSize; ++a)
				{
				xstart = (XSize-1-a)*Bin;
				xend = xstart + Bin;
				for (b=0; b<YSize; ++b)
					{
					temp = 0;
					ystart = b*Bin;
					yend = ystart + Bin;
					for (int x=xstart; x<xend; ++x)
						{
						for (int y = ystart; y < yend; ++y)
							temp += TDIArray[x][y];
						}
					image[b][a] = (unsigned short)(temp/Bin2);
					}
				}
			delete  TDIArray;
			delete  TDIBuffer;
			}


	}


void SI1100::GetCCDSize(int * NumCols, int * NumRows)
	{
	if (LoadConfigurationParameters())
		Texit("Cannot get configuration parameters from camera");
	if (LoadReadoutParameters())
		Texit("Cannot get readout parameters from camera");

	SysReadout[SERIAL_ORIGIN] = 16;				//was 28, changed to 16 for new CCD 2012/16/08
	SysReadout[SERIAL_READ_LENGTH] = 1000;
	SysReadout[SERIAL_BINNING] = 2;
	SysReadout[SERIAL_POSTSCAN] = 0;			//was 8, changed to 0
	SysReadout[PARALLEL_ORIGIN] = 0;			//was 8, changed to 0 for new CCD
	SysReadout[PARALLEL_READ_LENGTH] = 2080;	//was 2040, changed to 2080 for new CCD 2012/06/08
	SysReadout[PARALLEL_BINNING] = 2;
	SysReadout[PARALLEL_POSTSCAN] = 0;			//was 9, changed to 0

	SendReadoutParameters();
	*NumCols = m_NumColumns = 2000;
	*NumRows = m_NumRows = 2080;

	}


void SI1100::Restart(void)
	{
	RestartDSPCode();
	}


int SI1100::InitializeCamera(void)
	{
#define SkipOldCameras true
	int error;

	CCameraDB *camera_db;
	CCamera *reset_camera;
	camera_db = new CCameraDB;
	reset_camera = new CCamera;



	TRACE("Searching for cameras...\n");
	camera_db->ScanInterfaces(SkipOldCameras);

	if (camera_db->camera[0].driver_present)
		{
		TRACE("%d. Driver %s, Model %u, SN %u, Baud=%u, Media Type=%d\n", 1,
			camera_db->camera[0].device_name,
			camera_db->camera[0].model,
			camera_db->camera[0].serial_number,
			camera_db->camera[0].baud_rate,
			camera_db->camera[0].mt);
		}

	error = OpenCamera(camera_db->camera[0].device_name);
	error = HardwareReset();
	camera_db->ScanInterfaces(SkipOldCameras);

	if (camera_db->camera[0].driver_present)
		{
		TRACE("%d. Driver %s, Model %u, SN %u, Baud=%u, Media Type=%d\n", 1,
			camera_db->camera[0].device_name,
			camera_db->camera[0].model,
			camera_db->camera[0].serial_number,
			camera_db->camera[0].baud_rate,
			camera_db->camera[0].mt);
		}

	SetBaudRate(camera_db->camera[0].baud_rate);
	if( LoadConfigurationParameters() == E_NO_ERROR ) {
		TRACE("\tSerial Size=%d, Parallel Size=%d\n",SysConfig[SERIAL_SIZE],SysConfig[PARALLEL_SIZE]);
		} 
	error = GetDSPCodeInfo(&firmware_partnum,&firmware_revision,&firmware_checksum);
	TRACE("\nFirmware version: %d, revision: %d, checksum %X",firmware_partnum,firmware_revision,firmware_checksum);
	m_CameraInitialized = TRUE;
	CoolerOn();
	//CoolerOff();
	CloseShutter();

	return 0;
	}
int SI1100::GetPepperCount(void)
	{
	return 0;
	}

void SI1100::SetPepper(int PepperLevel)
	{
	}

int SI1100::GetPepper(void)
	{
	return 0;
	}

void SI1100::SetGain(int Gain, int Speed)
	{
	SysReadout[ANALOG_ATTENUATION] = Gain;
	if(SendReadoutParameters())
		Texit("cannot send readout parameters to camera");
	}

float SI1100::GetGain(int Gain, int Speed)
	{
	return (float)SysReadout[ANALOG_ATTENUATION];
	}

void SI1100::GetTempPressure(float *Temperature, float *Pressure)
	{
	//TODO
	GetStatus();
	*Pressure = (SysStatus[2]-528/1000); //528 is calbration factor
	*Temperature = (SysStatus[0]/10)-278.0; //278 to bring it to calibration
	}

int SI1100::GetNumActiveColumns(void)
	{
	return 2040;
	}

void SI1100::ErrorSwitch(int ErrorCode)
	{
	switch (ErrorCode)
		{
	case 1:
		Texit("E_TIMEOUT");
		break;
	case 2:
		Texit("E_UNUSABLE_POINTER");
		break;
	case 3:
		Texit("E_COM_ERROR");
		break;
	case 4:
		Texit("E_DMA_ACTIVE");
		break;
	case 5:
		Texit("E_DMA_NOT_ACTIVE");
		break;
	case 6:
		Texit("E_RETRIES_EXHAUSTED");
		break;
	case 7:
		Texit("E_FILE_NOT_FOUND");
		break;
	case 8:
		Texit("E_NO_DRIVERS_INSTALLED");
		break;
	case 9:
		Texit("E_DMA_THREAD_FAILED");
		break;
	case 10:
		Texit("E_DEINTERLACE_THREAD_FAILED");
		break;
	case 11:
		Texit("E_MEMORY_ALLOCATION_FAILED");
		break;
	case 12:
		Texit("E_INVALID_DEINTERLACE_MODE");
		break;
	case 13:
		Texit("E_INVALID_PARAMETER_OFFSET");
		break;
	case 14:
		Texit("E_INVALID_CAMERA_DB_INDEX");
		break;
	case 15:
		Texit("E_MEDIA_TYPE_NOT_SELECTABLE");
		break;
	case 16:
		Texit("E_MEDIA_TYPE_INVALID");
		break;
	case 17:
		Texit("E_STRING_TOO_LONG");
		break;
	case 18:
		Texit("E_NEGATIVE_RESPONSE");
		break;
	case 19:
		Texit("E_FUNCTION_UNSUPPORTED");
		break;
	case 20:
		Texit("E_UNSUPPORTED_RECORD_TYPE");
		break;
	case 21:
		Texit("E_FRAMING_OVERRUN");
		break;
	case 22:
		Texit("E_FILE_OPEN_ERROR");
		break;
		}

	}
