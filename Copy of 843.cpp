#include "StdAfx.h"
#include "C843.h"
#include "843.h"
#include "c843profilevars.h"
#include "C843_GCS_DLL.H"
#include "XYPhiMove.h"
#include "mclmcr.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "MuCatDoc.h"
#include "convert.h"

using namespace std;

int C843::m_ChipReset = 0;
char* C843::m_Report = new char[128];
int C843::m_EndStopPosition;

vector<string> C843::XYlist;
vector<string> C843::Philist;

vector<string> C843::cwXYlist;
vector<string> C843::cwPhilist;
vector<string> C843::acwXYlist;
vector<string> C843::acwPhilist;

//char** C843::XYlist = NULL;
//char** C843::PHIlist = NULL;
int C843::acwProfileSize = 0;
int C843::acwXYProfileSize = 0;
int C843::acwPhiProfileSize = 0;
int C843::cwProfileSize = 0;
int C843::cwXYProfileSize = 0;
int C843::cwPhiProfileSize = 0;
int C843::BoardHandle = -1;
bool C843::m_Profile_Valid = false;
bool C843::m_Library_Initialised = false;

float C843::AngularPixelSize = NULL;
float C843::Source_X = NULL;
float C843::Source_Y = NULL;
double C843::cwProfileStartX = 153;
double C843::cwProfileStartY = 153;
double C843::cwProfileStartPhi = 0;
double C843::acwProfileStartX = 153;
double C843::acwProfileStartY = 153;
double C843::acwProfileStartPhi = 0;

string Yfilename = "YAxis_1.txt";
string Xfilename = "XAxis_1.txt";
string Phifilename = "PhiAxis_1.txt";

C843::C843(int Axis, int Stage)
: m_Velocity(0)
{



	m_Axis = Axis;
 	m_StageType = Stage;
	switch(Stage)
	{
	case LINEAR_PD:
		SetPID(200,50,400,400000);
		m_TopSpeed = 62.5;
		//m_TopSpeed = 250000;
		break;
	case LINEAR_GEARED:
		SetPID(250,50,800,2000000);
		m_TopSpeed = 6.0;
		//m_TopSpeed = 180000;
		break;
	case ROTATIONAL_DIRECT:
		SetPID(200,50,400,750000);
		m_TopSpeed = 45.0;
		break;
	case ROTATIONAL_PD:
		SetPID(510, 500, 400, 1000000);
		m_TopSpeed = 45.0;
		//m_TopSpeed = 180000;
	}
	//C843_SetQMC(BoardHandle,SET_INTEGRATION_LIMIT,m_Axis,2000);
	SetVelocityFraction(0.01);
	//SetVelocityFraction(1.0);
}

C843::~C843()
{
//if (C843::XYlist)
//	{
//	for(int w = 0; w < NUMBER_OF_SENTENCES; w++)
//		{delete [] C843::XYlist[w];}
//	delete [] C843::XYlist;
//	C843::XYlist = NULL;
//	}
}




void C843::Move(long Pos)
{
	double Position;
	int error;

	switch (m_StageType)
		{
	case LINEAR_PD:
		Position = Pos/2000.0;
		break;
	case LINEAR_GEARED:
		Position = Pos/2000.0;		//We understand this now, we will not later
									// Let MuCAT do the scaling and let the C843 card do
									// HW scaling.
		//Position = Pos/303534.0;
	case ROTATIONAL_DIRECT:
	case ROTATIONAL_PD:
	case ROTATIONAL_GEARED:
		Position = Pos/1000.0;
		break;
		}

	char* tmp = new char[5];
	_itoa(m_Axis+1,tmp,10);
	//dPoses[m_Axis -1] = Position;
	dPoses[0] = Position;
	error = C843_MOV(BoardHandle,tmp, dPoses);
	TRACE("\nError report is: %d",error);
	delete [] tmp;	

//C843_SetQMC(BoardHandle,RESET_EVENT_STATUS, m_Axis, 0XF6);
//	C843_SetQMC(BoardHandle,UPDATE,m_Axis,0);
//	MoveA(m_Axis+1, Position);
//	setQMC(m_Axis);
//	setQMC(PI_SET_POS);
//	WriteDWord(Position);
//	setQMC(PI_SET_POS, m_Axis, Position);
//	setQMC(PI_UPDATE,m_Axis,0);
}

void C843::SetPID( int proportional, int integral, int derivative, long acceleration)
	{
	long delme = 0;
	delme = GetPosition();

	//Move(GetPosition()/2000);
	C843_SetQMC(BoardHandle,SET_KP, (BYTE) m_Axis, proportional);
	C843_SetQMC(BoardHandle,SET_KI, (BYTE)m_Axis, integral);
	C843_SetQMC(BoardHandle,SET_KD, (BYTE)m_Axis, derivative);
	Move(delme);
	C843_SetQMC(BoardHandle,UPDATE,(BYTE) m_Axis,0);
	SetAcceleration(acceleration);
	delme = GetPosition();
	}


void C843::SetVelocity(long velocity)
	//velocity is in mm/sec
	{
	double vel[1];
	int error;
	m_Velocity = velocity;

	char* tmp = new char[5];
	_itoa(m_Axis+1,tmp,10);
	vel[0] = velocity;
	error = C843_VEL(BoardHandle,tmp, vel);
	delete [] tmp;	


	//error = C843_VEL(BoardHandle,(char *) m_Axis,&velocity);
	//error = C843_SetQMC(BoardHandle,SET_VELOCITY, m_Axis, (long)(velocity*26.86976));

	}

void C843::SetVelocityFraction(double fraction)
{
	SetVelocity((long)(m_TopSpeed*fraction));
}

unsigned short C843::ReadSignalStatus() 
{
	
	unsigned short status;
//	setQMC(m_Axis);
//	status = ReadWord();
	get_signal_status(m_Axis+1, &status);
	return status;
}

void C843::WaitForStop()
{
BOOL bIsMoving = TRUE;
while(bIsMoving) // wait for end of motion
	{
	C843_IsMoving(BoardHandle, szBAxes, bFlags);
	bIsMoving = FALSE;
	for (axis = 0; axis < nrBAxes; axis++)
		{
		bIsMoving |= bFlags[axis];
		}
	Sleep(100);
	}

//	setQMC(m_Axis);
//	setQMC(PI_CLR_STATUS);
	//unsigned short Status;
/*	long ClockTime, NewTime;
	getQMC(GET_TIME, m_Axis, &ClockTime);
	do
	{
		getQMC(GET_TIME, m_Axis, &NewTime);
	} while (ClockTime == NewTime);
	ClockTime = NewTime;
	do
	{
		getQMC(GET_TIME, m_Axis, &NewTime);
	} while (ClockTime == NewTime);*/
	//Pause(3);
	//while(((Status = ReadActivityStatus()) &1024) != 0);
}

void C843::FindHome()
{	
int error;
//reference (home) the stages synchronsly
		if (!C843_FRF(BoardHandle, szAxes))	
			{::MessageBoxA(NULL,"Error sending the HOME STAGES command","FATAL ERROR!",MB_OK|MB_ICONERROR);}
		
		BOOL bIsMoving = TRUE;
		while(bIsMoving) // wait for end of motion
		{
			error = C843_GetError(BoardHandle);
			C843_IsReferencing(BoardHandle, szBAxes, bFlags);
			bIsMoving = FALSE;
			for (axis = 0; axis < nrBAxes; axis++)
			{
				bIsMoving |= bFlags[axis];
			}
			Sleep(100);
		}

	//check the stages claim to be referenced
		if (!C843_IsReferenceOK(BoardHandle, szRefAxes, bFlags))
		{::MessageBoxA(NULL,"Error homing the stages, check power and that nothing is fouling them","FATAL ERROR!",MB_OK|MB_ICONERROR);}

	// read extreme coordinates
		C843_qTMN(BoardHandle, szBAxes, dMinPoses);
		C843_qTMX(BoardHandle, szBAxes, dMaxPoses);




}





long C843::GetPosition()
{
	int Position;
	C843_GetQMC(BoardHandle,GET_ACTUAL_POSITION,m_Axis,&Position);
	switch (m_StageType)
		{
	case LINEAR_PD:
		return Position;
		break;
	case LINEAR_GEARED:
		return Position /15.1767;	//for the same reasons as in Move()
		//return Position *30353;
	case ROTATIONAL_DIRECT:
	case ROTATIONAL_PD:
	case ROTATIONAL_GEARED:
		return Position;
		break;
		}
}



void C843::Stop()
{
	char* tmp = new char[5];
	_itoa(m_Axis+1,tmp,10);
	C843_HLT(BoardHandle,tmp);
	delete [] tmp;
}

void C843::WaitForPositionPositive(long Position)
{

	while(GetPosition() < Position);
}

void C843::SetAcceleration(long Acc)
{

	C843_SetQMC(BoardHandle,SET_DECELERATION,(BYTE) m_Axis,(long)(Acc*0.0110166));
	C843_SetQMC(BoardHandle,UPDATE,(BYTE) m_Axis,0);
}

int C843::GetEndStopPosition()
{
	return m_EndStopPosition;
}

void C843::MoveToEnd()
{
	Move((m_EndStopPosition*97)/100);
}



void C843::SetTopSpeed(int Speed)
{
	m_TopSpeed = Speed;
}

void C843::Pause(long NumClockCycles)
{
	int ClockTime, NewTime;
	C843_GetQMC(BoardHandle,GET_TIME, 0, &ClockTime);
	do
	{
		C843_GetQMC(BoardHandle,GET_TIME, 0, &NewTime);
	} while ((NewTime - ClockTime) < NumClockCycles);
}




int C843::SetTriggerEvent(BYTE TriggerAxis, short bpNo)
	{
	// this uses the QFL command set, so things are a little different
	// no units, just absolute counts. The first axis is axis zero.
	// we use bit '2' on event status register '1' for breakpoint 1
	// and bit 'E' as BP two 
	// we signal an event on AxisIO pin 0 (Axis 1)

	short tmp = 0;
	BYTE BIT = 2;
	for (tmp = 0; tmp<bpNo+1; tmp++)
		{
		set_triggered_event(0,TriggerAxis,BIT,1);
		BIT+=0x0c;
		}
	return 1;
	}

int C843::SetTriggerPoints(long Xpos1, long Ypos1, long Xpos2, long Ypos2, BOOL bothways)
	{
	//this assumes that X is axis 0 and Y is axis 1

	


	if(!bothways)
		{
		set_nonblocking_breakpoint(0,0,Xpos1);
		//set_nonblocking_breakpoint(1,0,Ypos1);
		}
	else
		{
		set_nonblocking_breakpoint(0,0,Xpos1);
		//set_nonblocking_breakpoint(0,1,Xpos2);
		//set_nonblocking_breakpoint(1,0,Ypos1);
		//set_nonblocking_breakpoint(1,1,Ypos2);
		}
		return 1;
	}






int C843::SetTriggerPoint(BYTE TriggerAxis, long position)
	{
	set_nonblocking_breakpoint(TriggerAxis, 0, position);
return 1;
	}

void C843::set_nonblocking_breakpoint(BYTE axis, BYTE BP_no, long value)
	{
	//Axes run from Axis0 to Axis3 in this way of addressing them. Just to confuse matters.

	WORD BPword = 0x0000;
	BPword |=BP_no;

	WORD BP_second_dataword = 0x0000;
	BP_second_dataword |=(6<<8);

	//BP_second_dataword |=(1<<4);	//Update axis 
	//BP_second_dataword |=(2<<4);	//Hard stop axis -works! yay!
	//BP_second_dataword |=(3<<4);  //slow stop
	//BP_second_dataword |=(4<<4);	//Motor off
	BP_second_dataword |=(0<<4);	//Do Nothing - take no action on a BP, just flag that it happened.
	
	BP_second_dataword |=axis; //
	//BP_second_dataword |=1; //Axis2

	BOOL tmp;
	//HRESULT tmp;
	long BPReadback;

	tmp = C843_SetQMCA(BoardHandle,SET_BREAKPOINT_VALUE, axis, BPword, value);
	//tmp = setQMCA(SET_BREAKPOINT_VALUE, axis, BPword, value);
	//if(tmp == FALSE)
	//{ 
	//  printf("Error setting BP value... but it might work anyway...\n");
	//  tmp = C843_GetQMC(BoardHandle,GET_BREAKPOINT_VALUE, axis, &BPReadback);
	//  //tmp = getQMC(GET_BREAKPOINT_VALUE, axis, &BPReadback);
	//  printf("Value Set was... %d",BPReadback);
	//
	//}

	tmp = C843_SetQMCA(BoardHandle,SET_BREAKPOINT, axis, BPword, BP_second_dataword);
	if(tmp == FALSE)
	{ 
	  printf("Error setting BP on Axis... but it might work anyway...\n");
	}
	}

void C843::set_triggered_event(BYTE AxisIOpin, BYTE TriggerAxis, BYTE bit, BYTE status_register)
	{
		//15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
		//0  0  0  0      R E G   B I T  SOURCE


	unsigned short data = 0x00;	//16bits of goodness to pack with our commands

	C843_SetQMC(BoardHandle,RESET_EVENT_STATUS, TriggerAxis, ~0x4); //reset breakpoint bit in status regsiter
	//setQMC(RESET_EVENT_STATUS, m_Axis, ~0x4); //reset breakpoint bit in status regsiter
	//setQMC(RESET_EVENT_STATUS, 1, 0x8fb); //reset breakpoint bit in status regsiter

	//tell IO pin to track encoder A value as a test

	
	//pack status register as bits 11 to 8

	status_register <<= 4;
	data |= status_register;
	data <<= 4;
	
	//pack BIT as bits 7 to 4
	bit <<= 4;
	data |= bit;

	//pack TriggerAxis as SOURCE

	data |= TriggerAxis;

	
	C843_SetQMC(BoardHandle,SET_AXIS_OUT_SOURCE,0,data);
	
	}

int C843::CheckPositions(void)
	{
#define limiter 5000

	long first_time[limiter], second_time[limiter], xpos[limiter], ypos[limiter], counter;
	for (counter=0;counter<limiter;counter++)
		{
		getQMC(GET_TIME, 0, &first_time[counter]);
		getQMC(GET_ACTUAL_POSITION, 0, &xpos[counter]);
		getQMC(GET_ACTUAL_POSITION, 1, &ypos[counter]);
		getQMC(GET_TIME, 0, &second_time[counter]);
		}
	for (counter=0;counter<limiter;counter++)
		{
		printf("\nTime = %d", first_time[counter]);
		printf("\nXPOS = %d", xpos[counter]);
		printf("\nYPOS = %d", ypos[counter]);
		printf("\nTime = %d", first_time[counter]);
		printf("\n");
		}
return 1;

	}


void C843::CreateProfile(double radius, double phi_start_angle, double start_angle, double travel_angle, double exposure_time)
	{

//Creates a clocwise profile dataset, cwXYList & cwPhilist; also creates the anti-clockwise datasets acwXYList & acwPhilist

	// Destroy previous profile
	C843::XYlist.clear();
	C843::Philist.clear();
	C843::cwXYlist.clear();
	C843::cwPhilist.clear();
	C843::acwXYlist.clear();
	C843::acwPhilist.clear();
	C843::acwXlist.clear();
	C843::cwYlist.clear();
	C843::cwPhilist.clear();
	C843::acwXlist.clear();
	C843::acwYlist.clear();
	C843::acwPhilist.clear();

	double ramp_angle = 2;



	// DLL Housekeeping here
		// Call the library intialization routine and make sure that the
		// library was initialized properly
		//
	if(!C843::m_Library_Initialised)
		{
		if (!mclInitializeApplication(NULL,0))
			{
			std::cerr << "could not initialize the Runtime library properly" << std::endl;
			exit(1);
			}
		if (!XYPhiMoveInitialize())
			{
			std::cerr << "could not initialize the movement library properly" << std::endl;
			exit(1);
			}
		}
	C843::m_Library_Initialised = true;
// DLL Housekeeping over


// Now we generate two profiles - the cw and the acw ones.
// First set up some constants for each profile

	mwArray omega_acc_xy(540.0);							// Acceleration.
	mwArray phi_fac(1.0);									// Fudge-factor to keep Phi in sync with XY stages.
	mwArray radius_xy(radius);
	mwArray moveback(0);									//Follow same arc back 1=yes, 0=no
	mwArray dwelltime(0.1);									//how long in seconds to dwell.
	double  midpoint_xy[2] = {C843::Source_X,C843::Source_Y};//define the midpoint of the XY profile
	mwArray center_xy(1,2,mxDOUBLE_CLASS);		
		 center_xy.SetData(midpoint_xy,2);
	mwArray omega_xy((double) exposure_time);
	mwArray XGCSLines,YGCSLines,PhiGCSLines;				//somewhere to put the DLL output...
	int nargout = 3;										//Number of arguments the DLL can change.
	
//Generate the cw profile first, using these variables:
	mwArray cwstartangle_xy(start_angle);	
	float phitmp1 = phi_start_angle + (start_angle - 270);
	mwArray cwstartangle_phi(phitmp1);
	mwArray cwstopangle_xy(start_angle - travel_angle);				


	// Call the DLL!
	GenerateXYPhiMotion(omega_xy,omega_acc_xy,phi_fac,\
		cwstartangle_xy,cwstartangle_phi,cwstopangle_xy,center_xy,radius_xy,moveback,dwelltime);
	
	// Process DLL output

	importProfileFile(Yfilename,cwYlist);
	importProfileFile(Xfilename,cwXlist);
	importProfileFile(Phifilename,cwPhilist);

	cwProfileStartY = ExtractStartPosition(cwYlist);
	cwProfileStartX = ExtractStartPosition(cwXlist);
	cwProfileStartPhi = ExtractStartPosition(cwPhilist);

// Here we have the cw profile, now we need to generate the acw profile
// Again set up our variables

	//Generate the acw profile first, using these variables:
	mwArray acwstartangle_xy(start_angle - travel_angle);	

	float phitmp2 = phi_start_angle - (start_angle - 270);
	mwArray acwstartangle_phi(phitmp2);
	mwArray acwstopangle_xy(start_angle);				

	// Call the DLL!
	GenerateXYPhiMotion(omega_xy,omega_acc_xy,phi_fac,\
		acwstartangle_xy,acwstartangle_phi,acwstopangle_xy,center_xy,radius_xy,moveback,dwelltime);

	// Process DLL output
	importProfileFile(Yfilename,acwYlist);
	importProfileFile(Xfilename,acwXlist);
	importProfileFile(Phifilename,acwPhilist);

	acwProfileStartY = ExtractStartPosition(acwYlist);
	acwProfileStartX = ExtractStartPosition(acwXlist);
	acwProfileStartPhi = ExtractStartPosition(acwPhilist);

	// Fix up profile size and get it ready to return...
	ProcessProfile(cwXlist,acwProfileStartX);	// cw / acw is NOT an error
	ProcessProfile(acwXlist,cwProfileStartX);	// cw ends at the acw start pos and inverse.



	////get needed memory size from file sizes.
	//int t_Xsize,t_Ysize,t_Phisize;
	//struct _stat buf;
	//char* Yfilename = "YAxis_1.txt";
	//char* Xfilename = "XAxis_1.txt";
	//char* Phifilename = "PhiAxis_1.txt";

	//_stat(Yfilename,&buf);
	//t_Ysize = buf.st_size;
	//_stat(Xfilename,&buf);
	//t_Xsize = buf.st_size;
	//_stat(Phifilename,&buf);
	//t_Phisize = buf.st_size;
	//string s;


	//ifstream Yaxis("YAxis_1.txt",ios::in|ios::binary|ios::ate);
	//if (Yaxis.is_open())
	//	{
	//	int lineindex = 0;
	//	t_Ysize = Yaxis.tellg();
	//	Yaxis.seekg (0, ios::beg);
	//	while (Yaxis.good())
	//		{
	//		getline(Yaxis,s);
	//		C843::cwXYlist.push_back(s);
	//		//TRACE("\n%s",s.c_str());
	//		if (lineindex == 5)
	//			{
	//			C843::cwProfileStartY = ExtractStartPosition(s);
	//			TRACE("\ncwProfileStartY = %f",C843::cwProfileStartY);
	//			}
	//		lineindex++;
	//		}
	//	Yaxis.close();

	//	//TRACE("\nLoaded %d bytes of YAxis profile data.\n",t_Ysize);
	//	}
	//ifstream Xaxis("XAxis_1.txt",ios::in|ios::binary|ios::ate);
	//if (Xaxis.is_open())
	//	{
	//	int lineindex = 0;
	//	t_Xsize = Xaxis.tellg();
	//	Xaxis.seekg (0, ios::beg);
	//	while (Xaxis.good())
	//		{
	//		getline(Xaxis,s);
	//		C843::cwXYlist.push_back(s);
	//		//TRACE("\n%s",s.c_str());
	//		if (lineindex == 5)
	//			{
	//			C843::cwProfileStartX = ExtractStartPosition(s);
	//			TRACE("\ncwProfileStartX = %f",C843::cwProfileStartX);
	//			}
	//		lineindex++;
	//		}
	//	Xaxis.close();

	//	//TRACE("\nLoaded %d bytes of XAxis profile data.\n",t_Xsize);
	//	}

	//ifstream PHIaxis("PhiAxis_1.txt",ios::in|ios::binary|ios::ate);
	//if (PHIaxis.is_open())
	//	{
	//	int lineindex = 0;
	//	t_Phisize = PHIaxis.tellg();
	//	PHIaxis.seekg (0, ios::beg);
	//	while (PHIaxis.good())
	//		{
	//		getline(PHIaxis,s);
	//		C843::cwPhilist.push_back(s);
	//		//TRACE("\n%s",s.c_str());
	//		if (lineindex == 5)
	//			{
	//			C843::cwProfileStartPhi = ExtractStartPosition(s);
	//			TRACE("\ncwProfileStartPhi = %f",C843::cwProfileStartPhi);
	//			}
	//		lineindex++;
	//		}
	//	PHIaxis.close();

	//	//TRACE("\nLoaded %d bytes of PHIaxis profile data.\n",t_Phisize);
	//	}

// Here we have the cw profile, now we need to generate the acw profile
// Again set up our variables

	//Generate the acw profile first, using these variables:
	//mwArray acwstartangle_xy(start_angle - travel_angle);	

	//float phitmp2 = phi_start_angle - (start_angle - 270);
	//mwArray acwstartangle_phi(phitmp2);
	//mwArray acwstopangle_xy(start_angle);				


	//// Call the DLL!
	//GenerateXYPhiMotion(omega_xy,omega_acc_xy,phi_fac,\
	//	acwstartangle_xy,acwstartangle_phi,acwstopangle_xy,center_xy,radius_xy,moveback,dwelltime);

	//// Process DLL output

	//////get needed memory size from file sizes.

	////_stat(Yfilename,&buf);
	////t_Ysize = buf.st_size;
	////_stat(Xfilename,&buf);
	////t_Xsize = buf.st_size;
	////_stat(Phifilename,&buf);
	////t_Phisize = buf.st_size;


	////ifstream Yaxis("YAxis_1.txt",ios::in|ios::binary|ios::ate);
	//Yaxis.open("YAxis_1.txt",ios::in|ios::binary|ios::ate);
	//if (Yaxis.is_open())
	//	{
	//	int lineindex = 0;
	//	t_Ysize = Yaxis.tellg();
	//	Yaxis.seekg (0, ios::beg);
	//	while (Yaxis.good())
	//		{
	//		getline(Yaxis,s);
	//		C843::acwXYlist.push_back(s);
	//		//TRACE("\n%s",s.c_str());
	//		if (lineindex == 5)
	//			{
	//			C843::acwProfileStartY = ExtractStartPosition(s);
	//			TRACE("\nacwProfileStartY = %f",C843::acwProfileStartY);
	//			}
	//		lineindex++;
	//		}
	//	Yaxis.close();

	//	//TRACE("\nLoaded %d bytes of YAxis profile data.\n",t_Ysize);
	//	}
	////ifstream Xaxis("XAxis_1.txt",ios::in|ios::binary|ios::ate);
	//Xaxis.open("XAxis_1.txt",ios::in|ios::binary|ios::ate);
	//if (Xaxis.is_open())
	//	{
	//	int lineindex = 0;
	//	t_Xsize = Xaxis.tellg();
	//	Xaxis.seekg (0, ios::beg);
	//	while (Xaxis.good())
	//		{
	//		getline(Xaxis,s);
	//		C843::acwXYlist.push_back(s);
	//		//TRACE("\n%s",s.c_str());
	//		if (lineindex == 5)
	//			{
	//			C843::acwProfileStartX = ExtractStartPosition(s);
	//			TRACE("\nacwProfileStartX = %f",C843::acwProfileStartX);
	//			}
	//		lineindex++;
	//		}
	//	Xaxis.close();

	//	//TRACE("\nLoaded %d bytes of XAxis profile data.\n",t_Xsize);
	//	}

	////ifstream PHIaxis("PhiAxis_1.txt",ios::in|ios::binary|ios::ate);
	//PHIaxis.open("PhiAxis_1.txt",ios::in|ios::binary|ios::ate);
	//if (PHIaxis.is_open())
	//	{
	//	int lineindex = 0;
	//	t_Phisize = PHIaxis.tellg();
	//	PHIaxis.seekg (0, ios::beg);
	//	while (PHIaxis.good())
	//		{
	//		getline(PHIaxis,s);
	//		C843::acwPhilist.push_back(s);
	//		//TRACE("\n%s",s.c_str());
	//		if (lineindex == 5)
	//			{
	//			C843::acwProfileStartPhi = ExtractStartPosition(s);
	//			TRACE("\nacwProfileStartPhi = %f",C843::acwProfileStartPhi);
	//			}
	//		lineindex++;
	//		}
	//	PHIaxis.close();

	//	//TRACE("\nLoaded %d bytes of PHIaxis profile data.\n",t_Phisize);
	//	}

	
	C843::acwXYProfileSize = C843::acwXYlist.size();
	C843::cwXYProfileSize = C843::cwXYlist.size();
	C843::acwPhiProfileSize = C843::acwPhilist.size();
	C843::cwPhiProfileSize = C843::cwPhilist.size();
	C843::acwProfileSize = C843::acwXYProfileSize + C843::acwPhiProfileSize;
	C843::cwProfileSize = C843::cwXYProfileSize + C843::cwPhiProfileSize;

	C843::m_Profile_Valid = TRUE;
	}


void C843::RunProfile(vector<string> vXYlist, vector<string> vPhilist,int vXYProfileSize,int vPhiProfileSize)
		{
		Clear_Errors();
		int index = 0;
		int debugging_answer = -1000;
		bool bDoneThisAlready = false;
//		char* tmp = new char[90];
		char tmp[90];
		C843_GcsCommandset(BoardHandle,"UPC $ $ -1 -1");

		do
			{

			//strcpy_s(tmp,89,C843::XYlist[index].c_str());	//copy the string from the profile to some scratch space
			strcpy_s(tmp,89,vXYlist[index].c_str());	//copy the string from the profile to some scratch space

			//uncomment these lines to check the error status of the profile in debug mode
			TRACE("%s :",tmp);
			TRACE("%d\n",C843_GetError(BoardHandle));
			//TRACE("%d\n",index);
			C843_GcsCommandset(BoardHandle, tmp);
			//if (index < XYlist.size()) {index++;}
			if (index < vXYProfileSize) {index++;}
			if (index >= vXYProfileSize) {bDoneThisAlready = TRUE;}


			}
			while(!bDoneThisAlready);




			index = 0;
			while (index < vPhiProfileSize)
				{
				//strcpy_s(tmp,89,C843::Philist[index].c_str());
				strcpy_s(tmp,89,vPhilist[index].c_str());
				TRACE("%s :",tmp);
				//debugging_answer = C843_GcsCommandset(BoardHandle, tmp);
				TRACE("%d\n",C843_GetError(BoardHandle));
				TRACE("%d\n",index);
				C843_GcsCommandset(BoardHandle, tmp);
				if (index < vPhiProfileSize) {index++;}
				}
	//		delete[] tmp;
				
			C843_GcsCommandset(BoardHandle,"UPA A 0");	//activate the data clusters
			TRACE("%d\n",C843_GetError(BoardHandle));
			C843_GcsCommandset(BoardHandle,"UPA B 0");
			TRACE("%d\n",C843_GetError(BoardHandle));
			C843_GcsCommandset(BoardHandle,"UPA C 0");
			TRACE("%d\n",C843_GetError(BoardHandle));
			C843_GcsCommandset(BoardHandle,"UPR 1 A 0 2 B 0 3 C 0");	//send the run command
			TRACE("%d\n",C843_GetError(BoardHandle));
			//TRACE("\n Start X:%d  Start Y:%d  Start Phi:%d",C843::vProfileStartX,C843::vProfileStartY,C843::vProfileStartPhi);
			//_CrtDumpMemoryLeaks();
		}
void C843::MoveToStart(double X,double Y, double start_angle, double elevation)
	{
	Clear_Errors();
	int error = 0xdead;
	double velocity[4];
	/*double ramp_angle = 2;
	start_angle = start_angle - ramp_angle;*/

	//use the two following lines instead for centre of axis motion
	//startX = (radius * cos(start_angle*(3.1415926/180))+136);
	//startY = (radius * -sin(start_angle*(3.1415926/180))+34);
	//startPhi = start_angle;


	dPoses[0] = X;
	velocity[0] = 50;
	error = C843_VEL(BoardHandle,"1",velocity);
	error = C843_MOV(BoardHandle,"1", dPoses);

	dPoses[0] = Y;
	velocity[0] = 3;
	error = C843_VEL(BoardHandle,"2",velocity);
	error = C843_MOV(BoardHandle,"2", dPoses);

	dPoses[0] = start_angle;
	velocity[0] = 50;
	error = C843_VEL(BoardHandle,"3",velocity);
	error = C843_MOV(BoardHandle,"3", dPoses);

	//don't move the elevation axis
	//dPoses[0] = (400000 - elevation)/2000;
	//velocity[0] = 50;
	//error = C843_VEL(BoardHandle,"4",velocity);
	//error = C843_MOV(BoardHandle,"4", dPoses);


	BOOL bIsMoving = TRUE;
		while(bIsMoving) // wait for end of motion
		{
			C843_IsMoving(BoardHandle, szBAxes, bFlags);
			bIsMoving = FALSE;
			for (axis = 0; axis < nrBAxes; axis++)
			{
				bIsMoving |= bFlags[axis];
			}
			Sleep(100);
		}

	}
bool C843::IsMoving(void)
	{
	BOOL bIsMoving = TRUE;
//		while(bIsMoving) // wait for end of motion
//		{
			C843_IsMoving(BoardHandle, szBAxes, bFlags);
			bIsMoving = FALSE;
			for (axis = 0; axis < nrBAxes; axis++)
			{
				bIsMoving |= bFlags[axis];
			}
//			Sleep(100);
//		}
		return bIsMoving;
	}

void C843::SetTriggerPoints(double startangle, double endangle, double radius, BOOL bothways)
	{

	             //(radius * cos(startangle *(3.1415926/180))+150);
	long Xpos1 = (radius * cos(startangle *(3.1415926/180))+153)*2000  ;
	long Ypos1 = (radius * -sin(startangle *(3.14/180))+153);
	long Xpos2 = (radius * cos(endangle *(3.14/180))+153)*2000;
	long Ypos2 = (radius * -sin(endangle *(3.14/180))+153);

	//pass all values even if we only need 1 or 2 breakpoints, BOOL bothways deals with what is actually used.

	SetTriggerPoints(Xpos1,Ypos1,Xpos2,Ypos2,bothways);
	
	}

void C843::UnsetBreakPoints(void)
	{
	C843_SetQMC(BoardHandle,RESET_EVENT_STATUS, 0, ~0x4); //reset breakpoint bit in status regsiter
	}


void C843::Reset(void)
	{
	//XYPHIlist = NULL;	//do this in a static.
	if(BoardHandle < 0)
		{
		BoardHandle = C843_Connect(board); // Call Connect function, use return value as ID
		if (BoardHandle < 0)
			{
			::MessageBoxA(NULL,"Unable to initialise I/O card","FATAL ERROR!",MB_OK|MB_ICONERROR);
			}

		//make a handle to the stages
		for (axis = 0; axis<nrAxes; axis++)
			{
			szAxis[0] = szAxes[axis];
			if (!C843_CST(BoardHandle, szAxis, szStages[axis]))	
				{
				TRACE("%d\n",C843_GetError(BoardHandle));
				::MessageBoxA(NULL,"Error identifying stages, probably missing the PISTAGES2.DAT file","FATAL ERROR!",MB_OK|MB_ICONERROR);
				}
			}

		//init stages
		if (!C843_INI(BoardHandle, szAxes))	
			{
			TRACE("%d\n",C843_GetError(BoardHandle));
			::MessageBoxA(NULL,"Error initialising stages","FATAL ERROR!",MB_OK|MB_ICONERROR);
			}
		}
		//C843_SetQMC(BoardHandle,RESET,0,0);
	}


void C843::debug_move(int axis, long value)
	{
	char* tmp = new char[5];
	_itoa(axis,tmp,10);
	dPoses[0] = value;
	C843_MOV(BoardHandle,tmp, dPoses);
	delete [] tmp;
	}


// Can only call this once. Only call when you exit the main application, else app restart is needed.
void C843::TerminateProfileDLL(void)
	{
	// DLL Cleanup.
	XYPhiMoveTerminate();
    // Shut down all MCR instances
    mclTerminateApplication();
	}


void C843::InvalidateProfile(void)
	{
	m_Profile_Valid = false;
	}


double *C843::PolarMove(float Azimuth, float Radius)
	//Azimuth in angular pixels, Radius in mm, AngularPixelSize in radians
	{
	Clear_Errors();
	TRACE("Azimuth: %f , Radius: %f\n",Azimuth,Radius);
	int error;

	double X,Y,theta;
	double *ret = new double[2];
//	theta = atan2((double)Radius,(double)Azimuth);
	//Azimuth = (Azimuth * 3.1415926 / 180);
	theta = (Azimuth) * AngularPixelSize;
	X = C843::Source_X + (Radius) * sin(theta);
	Y = C843::Source_Y + (Radius) * cos(theta);
	TRACE("Xposition: %f, Yposition: %f\n",X,Y);


	char* tmp = new char[5];
	_itoa(1,tmp,10);
	dPoses[0] = X;
	error = C843_MOV(BoardHandle,tmp, dPoses);
	delete [] tmp;	

	char* tmp1 = new char[5];
	_itoa(2,tmp1,10);
	dPoses[0] = Y;
	error = C843_MOV(BoardHandle,tmp1, dPoses);
	delete [] tmp1;

	ret[0] = X;
	ret[1] = Y;
	return ret;
	/*CMuCatDoc::m_Azimuth = X;
	CMuCatDoc::m_Radius = Y;*/

	/*m_Radius = Y;
	m_Azimuth = X;
	m_pSlide->Move(X*2000);
	m_pMagnificationSlide->Move(Y*1000);*/
	}


double C843::ExtractStartPosition(string CommandString)
	{
	string temp;
	double StartPosition;
	int offsetindex = 0;
	for (unsigned int i=0; i < CommandString.size(); i++)
		{
		if (isdigit(CommandString[i])|| (CommandString[i] == '-'))
			{
			offsetindex++;
			if (offsetindex == 10)
				{
				for (unsigned int a=i; a<i+10; a++)
					{
					temp += CommandString[a];               
					}
				//the first numeric block is extracted
				break;
				}

			}    
		}
	istringstream stream(temp);
	stream >> StartPosition; 
	return StartPosition;
	}


// Calculates an azimuth from a radius and translation position returned from the Profile Generator. Translation is in stage counts, Radius in mm
double C843::CalcAzimuth(double radius, double Translation)
	{
	
	double tmp = asin((Translation - C843::Source_X)/(radius))/C843::AngularPixelSize;
	return tmp;

	}


void C843::DoProfile(void)
	{
	int Position;
	C843_GetQMC(BoardHandle,GET_ACTUAL_POSITION,0,&Position);
	if ((Position/2000) < C843::Source_X)
		{
		RunProfile(acwXYlist,acwPhilist,acwXYProfileSize,acwPhiProfileSize);
		
		}
	else
		{
		RunProfile(cwXYlist,cwPhilist,cwXYProfileSize,cwPhiProfileSize);
		
		}
	}


void C843::Clear_Errors(void)
	{
	int error;
	error = C843_GetError(BoardHandle);
	if (error !=0)
		{
		TRACE("\nClearing errors...\n");
		C843_CLR(BoardHandle,NULL);
		}
	}

int C843::importProfileFile(string filename, vector<string> &storage)
	{
	int failed = true;
	string s;
	ifstream pfile(filename.c_str(),ios::in|ios::binary|ios::ate);
	if (pfile.is_open())
		{
		failed = false;
		int lineindex = 0;
		size_t t_pfilesize = pfile.tellg();
		pfile.seekg (0, ios::beg);
		while (pfile.good())
			{
			getline(pfile,s);
			storage.push_back(s);
			}
		pfile.close();

	}
	return failed;
	}
void ProcessProfile(vector<string> &AxisData,double newendpos)
	{
	int index,offset,oldsize,newsize;
	string temp,fixedstring;
	vector<string> tokens;
	stringstream tss;

	for (index = 0; index < 5; index++)
		{
		if (index == 0||index == 2)
			{
			stringstream ss(AxisData[index]);
			temp.clear();
			tokens.clear();
			fixedstring.clear();
			while (ss>>temp)			
				{
				tokens.push_back(temp);
				cout<<temp<<endl;
				}
			if (tokens[0] =="UPC")
				offset = 3;
			else
				offset = 4;

			istringstream ( tokens[offset] ) >> oldsize;
			newsize = oldsize+1;
			temp.clear();
			temp = ConvertNumberToString<int>(newsize);

			tokens.at(offset) = temp;
			for (int i = 0; i<tokens.size(); ++i)
				{
				fixedstring +=tokens[i];
				fixedstring +=" ";
				cout<<fixedstring<<endl;
				}
			cout<<AxisData[index]<<endl;
			AxisData.at(index) = fixedstring;
			cout<<AxisData[index]<<endl;
			}
		}
	vector<string> patchedProfile = C843::patchProfile(AxisData[AxisData.size()-3],AxisData[AxisData.size()-2],newendpos);
	AxisData.pop_back();
	AxisData.pop_back();
	AxisData.pop_back();
	AxisData.insert(AxisData.end(),patchedProfile.begin(),patchedProfile.end());

	cout<<endl;
	cout<<endl;
	cout<<endl;
	for (int i = 0; i<AxisData.size(); i++)
		{

		cout<<AxisData[i]<<endl;
		}
	}

double ExtractStartPosition(vector<string> AxisData)
	{
	double startpos;
	string temp;
	vector<string> tokens;

	temp = AxisData[5];
	stringstream ss(temp);
	
	while (ss>>temp)
		{
		tokens.push_back(temp);
		}

	istringstream ( tokens[5] ) >> startpos;

	return startpos;
	}
