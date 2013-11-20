#include "StdAfx.h"
#include "C843.h"
#include "843.h"
#include "c843profilevars.h"
#include "C843_GCS_DLL.H"
#include "XYPhiMove.h"
#include "mclmcr.h"

int C843::m_ChipReset = 0;
char* C843::m_Report = new char[128];
int C843::m_EndStopPosition;
char** C843::XYPHIlist = NULL;
int C843::ProfileSize = 0;
int C843::BoardHandle = -1;
bool C843::m_Profile_Valid = false;
bool C843::m_Library_Initialised = false;
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
		m_TopSpeed = 3.0;
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
if (C843::XYPHIlist)
	{
	for(int w = 0; w < NUMBER_OF_SENTENCES; w++)
		{delete [] C843::XYPHIlist[w];}
	delete [] C843::XYPHIlist;
	C843::XYPHIlist = NULL;
	}
}




void C843::Move(long Pos)
{
	double Position;
//if (m_Axis == 1 | m_Axis == 4) //both the M351.PD stages
//	{Position = Pos /2000.0;}
//else if (m_Axis == 2)			// the M531.DG stage has different scaleing.
//	{Position = Pos / 303534.0;}
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
	_itoa(m_Axis,tmp,10);
	dPoses[0] = Position;
	C843_MOV(BoardHandle,tmp, dPoses);
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
	_itoa(m_Axis,tmp,10);
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



	//char* tmp = new char[5];
	//_itoa(m_Axis,tmp,10);
	//C843_REF(BoardHandle,tmp);
	////C843_REF(BoardHandle,tmp);
	//TRACE("%d\n",C843_GetError(BoardHandle));
	//delete [] tmp;	

	//long Position;
	//setQMC(SET_ACTUAL_POSITION,m_Axis,0);//		Set position to zero.
	//if (m_StageType == LINEAR_PD)
	//	SetVelocityFraction(0.5);
	//else
	//	SetVelocityFraction(1.0);
	///*if (m_StageType == LINEAR_PD || m_StageType == LINEAR_GEARED)
	//{
	//	if (GetLimitSwitches() & 0x02)
	//	{
	//		Move(-1000000);
	//		while(GetLimitSwitches() & 0x02)
	//			;
	//	}
	//	setQMC(SET_ACTUAL_POSITION,m_Axis,0);//		Set position to zero.
	//	SetVelocityFraction(0.5);
	//	Move(+10000000);//				Hit +ve end stop.
	//	WaitForStop();
	//	setQMC(SET_ACTUAL_POSITION,m_Axis,0);//		Set position to zero.
	//	SetVelocityFraction(1.0);
	//}*/
	//setQMC(SET_SIGNAL_SENSE, m_Axis, 0);
	//Move(1000000);
	//while(ReadSignalStatus()&0x08)
	//	;
	//Move(-10000000);//				Move in -ve direction.
	//while(!(ReadSignalStatus()&0x08))
	//	;
	//SetVelocityFraction(0.1);
	//Move(10000000);//				Reverse at 1/10th speed.
	//Position = GetCapturePos();//		Wait for home and store position.
	//Move(Position);
	//WaitForStop();
	////if (m_StageType == ROTATIONAL_DIRECT)
	////	Step += 133550;//			Sets the offset for rotational stage
	//setQMC(SET_ACTUAL_POSITION,m_Axis,0);//		Set position to zero.
	//SetVelocityFraction(1.0);
	//Move(0L);
}



//long C843::GetCapturePos()
//{
//	long Position;
//	getQMC(GET_CAPTURE_VALUE,m_Axis,&Position);
//	setQMC(SET_SIGNAL_SENSE, m_Axis, 0x00);
//	setQMC(SET_CAPTURE_SOURCE,m_Axis,1);// Set capture for home position
//	setQMC(RESET_EVENT_STATUS,m_Axis,0XF7);
//	while(!(ReadEventStatus() & 0x08))
//		;
//	getQMC(GET_CAPTURE_VALUE,m_Axis,&Position);
//	return Position;
//}

long C843::GetPosition()
{
	int Position;
	C843_GetQMC(BoardHandle,GET_ACTUAL_POSITION,m_Axis-1,&Position);
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

//int C843::GetLimitSwitches()
//{
//	// Bit 0 = +ve switch, bit 1 = -ve switch, 1 = set.
//	unsigned short Status;
//	Status = ReadSignalStatus();
//	return (Status>>4) & 0x03;
//}

void C843::Stop()
{
	char* tmp = new char[5];
	_itoa(m_Axis,tmp,10);
	C843_HLT(BoardHandle,tmp);
	delete [] tmp;

	/*setQMC(SET_STOP_MODE,m_Axis,2);
	setQMC(UPDATE,m_Axis,0);
	SetVelocity(m_Velocity);*/
}

void C843::WaitForPositionPositive(long Position)
{
	/*setQMC(m_Axis);
	setQMC(PI_SET_I);
	setQMC(PI_SET_INTRPT_MASK);
	WriteWord(0x04);//			Set interrupt for breakpoint
	setQMC(PI_RST_INTRPT);
	setQMC(PI_SET_BRK_PNT);
	WriteDWord(Position);
	setQMC(PI_SET_ACTL_POS_BRK);
	while(_inp(MOTOR_BASE+1)&0x40)// Wait for interrupt line (polling)
		;*/
	while(GetPosition() < Position)
		;
}

void C843::SetAcceleration(long Acc)
{
//	setQMC(SET_ACCELERATION,m_Axis,(long)(Acc*0.0110166));
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

//unsigned short C843::ReadEventStatus(void)
//{
//	unsigned short Status;
//	get_event_status(m_Axis+1, &Status);
//	return Status;
//}

//unsigned short C843::ReadActivityStatus(void)
//{
//	unsigned short Status;
//	get_activity_status(m_Axis+1, &Status);
//	return Status;
//}

//long C843::GetCaptureNeg()
//{
//	long Position;
//	getQMC(GET_CAPTURE_VALUE,m_Axis,&Position);
//	setQMC(SET_SIGNAL_SENSE, m_Axis, 0x08);
//	setQMC(RESET_EVENT_STATUS,m_Axis,0XF7);
//	setQMC(SET_CAPTURE_SOURCE,m_Axis,1);// Set capture for home position
//	while(!(ReadEventStatus() & 0x08))
//		;
//	getQMC(GET_CAPTURE_VALUE,m_Axis,&Position);
//	return Position;
//}

//void C843::Output(char OutputData)
//{
//	setQMC(WRITE_IO, 0, OutputData);
//}
//
//void C843::TDITriggerHigh(void)
//{
//	execute(0,CN,1,m_Report);
//}
//
//void C843::TDITriggerLow(void)
//{
//	execute(0,CF,1,m_Report);
//}
//
//void C843::TriggerCamera(void)
//{
//	TDITriggerLow();
//	Pause(1);
//	TDITriggerHigh();
//}

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

//void C843::Reset(void)
//{
//	if (!m_ChipReset)
//	{
//		OpenBoard();
//		setQMC(RESET,0,0);
//		OpenBoard();
//	//	m_ChipReset = 1;
//		TDITriggerHigh();
//	}
//}


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


void C843::CreateProfile(double radius, double start_angle, double travel_angle, double exposure_time)
	{
	double ramp_angle = 2;
	//double ramp_angle = ((180 * ramp_distance)/(3.1415926*radius)); //angle in degrees.

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

	// Prep the arrays the DLL needs
		//EXPOSURE TIME calculation is WRONG!
		//Have hardcoded a value for now
		//double test123 = 2.34;
	//	mwArray omega_xy(test123);		// Angular velocity deg/s
	
	//mwArray omega_xy((2*ramp_angle + travel_angle)/exposure_time);		// Angular velocity deg/s
	mwArray omega_acc_xy(540.0);										// Acceleration.
	mwArray phi_fac(1.0);												// Fudge-factor to keep Phi in sync with XY stages.
	mwArray startangle_xy(start_angle - ramp_angle);						
	mwArray startangle_phi(start_angle - ramp_angle);
	mwArray stopangle_xy(start_angle + travel_angle + ramp_angle);				
	mwArray radius_xy(radius);
	mwArray moveback(1);						//Follow same arc back 1=yes, 0=no
	mwArray dwelltime(0.1);						//how long in seconds to dwell.
	double  midpoint_xy[2] = {X_source_pos,Y_source_pos};			//define the midpoint of the XY profile
	mwArray center_xy(1,2,mxDOUBLE_CLASS);		
		center_xy.SetData(midpoint_xy,2);
	mwArray omega_xy((double) exposure_time);
	

	mwArray XGCSLines,YGCSLines,PhiGCSLines;	//somewhere to put the DLL output...
	int nargout = 3;							//Number of arguments the DLL can change.

	// Call the DLL!
	//GenerateXYPhiMotion(nargout,XGCSLines,YGCSLines,PhiGCSLines,omega_xy,omega_acc_xy,phi_fac,\
		startangle_xy,startangle_phi,stopangle_xy,center_xy,radius_xy,moveback,dwelltime);

	GenerateXYPhiMotion(omega_xy,omega_acc_xy,phi_fac,\
		startangle_xy,startangle_phi,stopangle_xy,center_xy,radius_xy,moveback,dwelltime);
	


	// Process DLL output
	
	int index = 0;									
	
	//space to store 10000 * 90 * sizeof(char) bytes
	C843::XYPHIlist = new char* [NUMBER_OF_SENTENCES];
	for(int w = 0; w < NUMBER_OF_SENTENCES; w++)
		{C843::XYPHIlist[w] = new char[MAX_SENTENCE_LENGTH];}

	//char all[90000];
	char* all = new char[90000];
	//char* all = all1;
	//sprintf(all,"%s%s%s",XGCSLines.ToString(),YGCSLines.ToString(),PhiGCSLines.ToString());
	strcpy_s(all,30000,XGCSLines.ToString());
	strcat_s(all,30000,YGCSLines.ToString());
	strcat_s(all,30000,PhiGCSLines.ToString());
	
	

	char *pch;
 	pch = strtok(all,"'");
	while (pch != NULL)
		{
		int wibble = strncmp(pch,"    [1x1",8);
		if (strncmp(pch,"    [1x1",8) != 0)					//strip the MatLAB array prettifiers.
			{
		if (*pch != (char)0x0a)								//strip the embedded newlines
			{
			strcpy_s(XYPHIlist[index],89,pch);
			
			//uncomment the following line to see the generated profile in debug mode
			//TRACE("%s\n",XYPHIlist[index]);
			++index;
			}
			}
		pch = strtok(NULL,"'");
		}
	
	C843::ProfileSize = index;

	C843::m_Profile_Valid = TRUE;
	delete [] all;
	}


void C843::RunProfile(void)
	{
	int index = 0;
	int debugging_answer = -1000;
	bool bDoneThisAlready = FALSE;
	do
		{

		char* tmp = new char[90];
		strcpy_s(tmp,89,C843::XYPHIlist[index]);					//copy the string from the profile to some scratch space
		//if (!strncmp(tmp," ",1))							//TRAP spaces on a newline by themselves
		//	{
		//	delete[] tmp;
		//	index++;
		//	char* tmp = new char[90];
		//	strcpy_s(tmp,89,XYPHIlist[index]);}


		//if (strncmp(tmp,"UPR",3)==0)						//TRAP the run command
		//	{
		//	bDoneThisAlready = TRUE;	
		//	}

		//uncomment these lines to check the error status of the profile in debug mode
		TRACE("%s :",tmp);
		//debugging_answer = C843_GcsCommandset(BoardHandle, tmp);
		TRACE("%d\n",C843_GetError(BoardHandle));
		
		C843_GcsCommandset(BoardHandle, tmp);
		delete[] tmp;
		if (index < ProfileSize) {index++;}
		if (index >= ProfileSize) {bDoneThisAlready = TRUE;}


		}
		while(!bDoneThisAlready);

		//uncomment these lines to check the GCS DLL version in debug mode.
		//char* tmp = new char[90];
		//C843_qIDN(BoardHandle,tmp,90);
		//TRACE("%s\n",tmp);
		//delete [] tmp;

		C843_GcsCommandset(BoardHandle,"UPA A 0");	//activate the data clusters
		TRACE("%d\n",C843_GetError(BoardHandle));
		C843_GcsCommandset(BoardHandle,"UPA B 0");
		TRACE("%d\n",C843_GetError(BoardHandle));
		C843_GcsCommandset(BoardHandle,"UPA C 0");
		TRACE("%d\n",C843_GetError(BoardHandle));
		C843_GcsCommandset(BoardHandle,"UPR 1 A 0 2 B 0 3 C 0");	//send the run command
		TRACE("%d\n",C843_GetError(BoardHandle));
		//_CrtDumpMemoryLeaks();
	}
void C843::MoveToStart(double X,double Y, double start_angle, double elevation)
	{
	/*double ramp_angle = 2;
	start_angle = start_angle - ramp_angle;*/

	//use the two following lines instead for centre of axis motion
	//startX = (radius * cos(start_angle*(3.1415926/180))+136);
	//startY = (radius * -sin(start_angle*(3.1415926/180))+34);
	//startPhi = start_angle;


	//pre move to start position
	dPoses[0] = X;
	dPoses[1] = Y;
	dPoses[2] = start_angle;
	dPoses[3] = 206-elevation;

	double velocity[4];
	velocity[0] = 50;
	velocity[1] = 50;
	velocity[2] = 50;
	velocity[3] = 50;

	C843_VEL(BoardHandle,szBAxes,velocity);
	C843_MOV(BoardHandle,szBAxes, dPoses);
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
