#pragma once
#include "sicamlib.h"
#include <vector>
using namespace std;

void Texit(char *Message);
#define CCDPIXELSIZE 30
#define VERTICAL_MAGNIFICATION 1.0

//#define PORT1OFFSET   456
//#define PORT2OFFSET   496

#define ROW_READ_TIME	1.70455		//fastest we can read a row in mS - experimentally determined on 04/11/2010 [MuCAT2]

//4 SEEMS to be ports A and B. 6 SEEMS to be ports C and D. Emperical evidence only!
#define portsAB	4;
#define portsCD	6;

class SI1100 : public CG2Camera
	{
	public:
		SI1100(void);
		~SI1100(void);

	bool	m_configLoaded;
	bool	m_readoutLoaded;
	bool	m_cooler_is_on ;
	bool	m_Acquiring;

	//enum read_direction { READ_CLOCKWISE=0,READ_ANTICLOCKWISE };
	

	private:
		int m_NumRows;
		int m_NumColumns;
		int		m_CameraInitialized;		// flag for camera ready


	protected:
		unsigned short * m_pRawImage;			// raw image
		HGLOBAL			m_ImgHandle;			// handle for image allocation

	public:
		// Allocates space for a camera fram
		int NewRaw(unsigned short Xsize, unsigned short Ysize);
		// Delete the pointer to old image and free up RAM
		void DeleteRaw(void);
		unsigned short * GetImagePtr(void)const{ return m_pRawImage;}
		//int TDIReadout(unsigned long ** image, unsigned short *TDIBuffer, unsigned short **TDIArray, int XSize, int YSize, int Bin, double StepSize, bool IsDark, bool ShutterOpen, int FrameNum, int NumFrames,unsigned int TDIDelay,int direction,vector<float>  pixel_mag_lookup);
		int TDIReadout(unsigned long ** image, int XSize, int YSize, int Bin, double StepSize, bool IsDark, bool ShutterOpen, int FrameNum, int NumFrames,unsigned int TDIDelay,int direction,vector<float>  pixel_mag_lookup);
		void Texit(char* Message);
		int ImageInternalTDI(void);
		void ExposeandReadoutRegion(float exposure, unsigned long ** image, int X, int Y, int XSize, int YSize, int Bin);
		int DMAStatus(void);
		void StillImage(float exposure, unsigned long ** image , int XSize, int YSize, int Bin, vector<float>  pixel_mag_lookup, bool IsDark);
		void GetCCDSize(int * NumCols, int * NumRows);
		void Restart(void);
		int GetPepperCount(void);
		void SetPepper(int PepperLevel);
		int GetPepper(void);
		float GetTemperature(void);
		int GetNumActiveColumns(void);
		int InitializeCamera(void);
		void SetGain(int Gain, int Speed);
		float GetGain(int Gain, int Speed);
		void GetTempPressure(float *Temperature, float *Pressure);
		u32 firmware_partnum;
		u32 firmware_revision;
		u32 firmware_checksum;

		void ErrorSwitch(int ErrorCode);
	};

extern SI1100 Camera;

