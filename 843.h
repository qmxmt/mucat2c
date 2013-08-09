#pragma once
#include <vector>
#include <string>
using namespace std;

#define LINEAR_PD 1
#define LINEAR_GEARED 2
#define ROTATIONAL_DIRECT 3
#define ROTATIONAL_GEARED 4
#define ROTATIONAL_PD 5


class C843  
{
public:
	void SetAcceleration(long);
	void WaitForPositionPositive(long Position);
	void Stop();
	long GetPosition();
	void StopAtEnd();
	void FindHome();
	void WaitForStop();
	void SetVelocityFraction(double fraction);
	void SetVelocity(long velocity);
	void SetPID(int integral, int proportional, int derivative, long acceleration);
	void Move(long Pos);
	C843(int Axis, int Stage);
	virtual ~C843();
	unsigned short ReadSignalStatus();
	void MoveToEnd();
	static int SetTriggerPoints(long Xpos1, long Ypos1, long Xpos2, long Ypos2, BOOL bothways);
	static void SetTriggerPoints(double startangle, double endangle, double radius, BOOL bothways);
	static int SetTriggerPoint(BYTE TriggerAxis, long position);
	static int SetTriggerEvent(BYTE TriggerAxis, short bpNo);
	int C843::CheckPositions(void);

protected:
	int GetEndStopPosition();
	static int m_EndStopPosition;
	int m_StageType;
	int GetLimitSwitches();
	long GetCapturePos();
	long GetCaptureNeg();

	static int m_ChipReset;
	static char* m_Report;
	long m_TopSpeed;
	static int BoardHandle;
	int m_Axis;
	//unsigned char m_Axis;

	static void set_nonblocking_breakpoint(BYTE axis, BYTE BP_no, long value);
	static void set_triggered_event(BYTE AxisIOpin, BYTE TriggerAxis, BYTE bit, BYTE status_register);

public:
	unsigned short ReadEventStatus(void);
	unsigned short ReadActivityStatus(void);
protected:
	long m_Velocity;
public:
	static void Output(char OutputData);
	static void TDITriggerHigh(void);
	static void TDITriggerLow(void);
	static void TriggerCamera(void);
	void SetTopSpeed(int Speed);
	static void Pause(long NumClockCycles);
	

// From the Profile Code
public:
//static const unsigned NUMBER_OF_SENTENCES = 10000;		//HORRIBLY MAGIC.
//static const unsigned MAX_SENTENCE_LENGTH = 90;		//less HORRIBLY MAGIC, but still...
static const int CCD_Size = 60;		//width in mm
static const int ramp_distance = 30;	//distance we need to move to have ramped up to full speed.
static bool m_Profile_Valid;
static bool m_Library_Initialised;
static float AngularPixelSize;
static float Source_X;
static float Source_Y;


	BOOL IsUserProfileActive(void);
	static bool IsMoving(void);
	static void gettimetest(void);
	char* InsertRotationGCS(long speed);
	//int Run3AxisProfile(void);
	static void MoveToStart(double X, double Y, double start_angle, double elevation);

	static std::vector<std::string> XYlist;		//temp profile data
	static std::vector<std::string> Philist;	//temp profile data
	static std::vector<std::string> cwXYlist;		//clockwise profile data
	static std::vector<std::string> cwPhilist;		//clockwise profile data
	static std::vector<std::string> acwXYlist;		//anti-clocwise profile data
	static std::vector<std::string> acwPhilist;		//anti-clocwise profile data

	static vector<string> cwXlist;		//these needed for on-load patching of profiles
	static vector<string> cwYlist;
	static vector<string> acwXlist;
	static vector<string> acwYlist;


	static double cwProfileStartX;
	static double cwProfileStartY;
	static double cwProfileStartPhi;
	static int cwProfileSize;
	static int cwXYProfileSize;
	static int cwPhiProfileSize;
	
	static double acwProfileStartX;
	static double acwProfileStartY;
	static double acwProfileStartPhi;
	static int acwProfileSize;
	static int acwXYProfileSize;
	static int acwPhiProfileSize;

	static double vProfileStartX;
	static double vProfileStartY;
	static double vProfileStartPhi;

	void Move(int axis, long position);
	static void CreateProfile(double radius, double phi_start_angle, double start_angle, double travel_angle, double exposure_time);
	static void RunProfile(std::vector<std::string> vXYlist, std::vector<std::string> vPhilist,int vXYProfileSize,int vPhiProfileSize);
	//static void RunProfile(void);
	

	static void UnsetBreakPoints(void);
	static void Reset(void);
	static void debug_move(int axis, long value);
	// Can only call this once. Only call when you exit the main application, else app restart is needed.
	static void TerminateProfileDLL(void);
	static void InvalidateProfile(void);
	static double *PolarMove(float Azimuth, float Radius);
	static double ExtractStartPosition(string CommandString);
	// Calculates an azimuth from a radius and translation position returned from the Profile Generator. Translation is in stage counts, Radius in mm
	static double CalcAzimuth(double radius, double Translation);
	static void DoProfile(void);
	static void Clear_Errors(void);

	static string FixProfileSize(string CommandString);
	static vector<string> patchProfile(string penultimate, string oldfinal, double newfinalpos);
	static int importProfileFile(string filename, vector<string> &storage);
	static double ExtractStartPosition(vector<string> AxisData);
	static void ProcessProfile(vector<string> &AxisData,double newendpos);

	static int mmToElevation(double ElevationInmm);
	// Takes the output of a getelevation call and returns something you can feed a move call to get the same result.
	static int ElevationtoElevationMove(int Elepos);
};
