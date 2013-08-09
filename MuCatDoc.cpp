// MuCatDoc.cpp : implementation of the CMuCatDoc class
//

#include "stdafx.h"
#include "MuCat.h"
#include "MuCatDoc.h"
#include "BinDialog.h"
#include "DimDialog.h"
#include "SpecDialog.h"
#include "XrayDialog.h"
#include "PositionDialog.h"
#include "MainFrm.h"
#include "GainDialog.h"
#include "MagDialog.h"
#include "DurationDialog.h"
#include "Geometry.h"
#include <string.h>
#include <math.h>
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMuCatDoc

IMPLEMENT_DYNCREATE(CMuCatDoc, CDocument)

BEGIN_MESSAGE_MAP(CMuCatDoc, CDocument)
	//{{AFX_MSG_MAP(CMuCatDoc)
	ON_COMMAND(ID_PARAMETER_BIN, OnParameterBin)
	ON_COMMAND(IDD_PARAMETER_GAIN, OnParameterGain)
	ON_COMMAND(ID_POSITION, OnPosition)
	ON_COMMAND(ID_PARAMETER_MAG, OnParameterMag)
	ON_COMMAND(ID_PARAMETER_EXP, OnParameterExp)
	ON_COMMAND(ID_EDIT_SPEC, OnEditSpec)
	ON_COMMAND(ID_EDIT_XRAY, OnEditXray)
	ON_COMMAND(ID_PARAMETER_DIM, OnParameterDim)
	ON_COMMAND(ID_EDIT_GEOMETRY, OnEditGeometry)
	ON_COMMAND(ID_BUTTONDIM, OnButtondim)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMuCatDoc construction/destruction

 bool CMuCatDoc::InvalidDueToGeometryChange = false;
 bool CMuCatDoc::FirstOnPosCall = false;
CMuCatDoc::CMuCatDoc():m_Turntable(2,ROTATIONAL_PD),m_Slide(0,LINEAR_PD),
	m_Elevator(3,LINEAR_PD),m_MagnificationSlide(1,LINEAR_GEARED)
	, m_SourceToCamera(0)
	, m_TDIDelay(0)
	, m_Azimuth(0)
	, m_SourcePosX(0)
	, m_SourcePosY(0)
	, m_Radius(0)
	, m_AngularPixelSize(0)
	
	{
	// TODO: add one-time construction code here
	CFile HeaderFile;
	m_NumActiveCols = 2000; // Inhibit initial height adjustment.
	if (HeaderFile.Open("header.bin",CFile::modeRead))
	{
		HeaderFile.Read(&m_Header, 512);
		SetBin(m_Header.bin);
		if (m_Header.num_frames < 1 || m_Header.num_frames > 100)
			m_Header.num_frames = 1;
	}
	else
	{
		SetBin(2);
		m_Header.gain = 0;
		m_Header.speed = 3;
		m_Header.num_frames = 1;
	}
	m_StillExposure = 1;
	FILE *GeoFile;
	if ((GeoFile = fopen("geometry.txt","r")) == NULL)
		Texit("Cannot open geometry file 'geometry.txt'");
	fscanf(GeoFile, "%f %f %f",&m_SourceToCamera,&m_SourcePosX,&m_SourcePosY);
	C843::Source_X = m_SourcePosX;
	C843::Source_Y = m_SourcePosY;
	C843::AngularPixelSize = (float)CCDPIXELSIZE*2.0/(m_SourceToCamera*10000.0);
	
	//radius is defined in the App constructor, just after we home the stages.
	m_Radius = m_Header.distance*10.0;
	m_Elevation = m_Header.z;
	m_Theta = m_Header.theta/1000.0;
	fclose(GeoFile);
}

CMuCatDoc::~CMuCatDoc()
{
	m_Header.bin = m_Bin;
	CFile HeaderFile("header.bin", CFile::modeWrite | CFile::modeCreate);
	HeaderFile.Write(&m_Header,512);
	m_Elevator.SetVelocityFraction(0.25);
	m_MagnificationSlide.Move(250000);
	m_MagnificationSlide.WaitForStop();
	m_Turntable.SetVelocityFraction(1.0);
	m_Turntable.Move(0);
	m_MagnificationSlide.WaitForStop();
//	m_MagnificationSlide.SetVelocityFraction(1.0);
	m_MagnificationSlide.FindHome();
	//m_Elevator.Move(300000);
	//m_MagnificationSlide.Move(170000);
	C843::TerminateProfileDLL();

}

BOOL CMuCatDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CMuCatDoc serialization

void CMuCatDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		ar.Read(&m_Header, 512);
		SetBin(m_Header.bin);
		m_Theta = m_Header.theta/1000.0;
		m_Elevation = m_Header.z;
		FirstOnPosCall = true;
		OnPosition();
		m_Turntable.Move(-m_Header.theta);
		m_Elevator.Move(m_Elevation*(-2));
		m_Elevator.WaitForStop();
		SetAzimuth(0);
		C843::PolarMove(0,(m_Header.posdistance*10.0));
		//m_MagnificationSlide.Move(m_Header.posdistance);// Change to angular move with radius and azimuth
		GetCCDSize();
		SetGain();
		SetMaxHeight();
		if (m_Header.num_frames < 1 || m_Header.num_frames > 100)
			m_Header.num_frames = 1;
		RoundExposure();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMuCatDoc diagnostics

#ifdef _DEBUG
void CMuCatDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMuCatDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMuCatDoc commands

void CMuCatDoc::OnParameterBin() 
{
	// TODO: Add your command handler code here
	CBinDialog aDlg;
	aDlg.m_Bin = m_Bin;
	if (aDlg.DoModal() == IDOK)
	{
		SetBin(aDlg.m_Bin);
		m_Header.bin = aDlg.m_Bin;
	}
}

int CMuCatDoc::GetBin()
{
	return m_Bin;
}

void CMuCatDoc::OnEditSpec() 
{
	// TODO: Add your command handler code here
	CSpecDialog aDlg;
	aDlg.m_Comment = m_Header.scan;
	aDlg.m_Description = m_Header.specimen;
	aDlg.m_Owner = m_Header.owner;
	aDlg.m_User = m_Header.user;
	if (aDlg.DoModal() == IDOK)
	{
		strcpy(m_Header.scan, aDlg.m_Comment);
		strcpy(m_Header.specimen, aDlg.m_Description);
		strcpy(m_Header.owner, aDlg.m_Owner);
		strcpy(m_Header.user, aDlg.m_User);
	}
}


void CMuCatDoc::OnEditXray() 
{
	// TODO: Add your command handler code here
	CXrayDialog aDlg;
	aDlg.m_Current = m_Header.current;
	aDlg.m_Filter  = m_Header.thickness;
	aDlg.m_Voltage = m_Header.voltage;
	aDlg.m_PepperThreshold = m_Header.pepper;
	aDlg.m_Issue = m_Header.issue;
	if (Camera.GetPepper())
		aDlg.m_PepperCheck = TRUE;
	if (aDlg.DoModal() == IDOK)
	{
		m_Header.current = aDlg.m_Current;
		m_Header.thickness = aDlg.m_Filter;
		m_Header.voltage = aDlg.m_Voltage;
		m_Header.pepper = aDlg.m_PepperThreshold;
		m_Header.issue = aDlg.m_Issue;
		if (aDlg.m_PepperCheck)
			Camera.SetPepper(aDlg.m_PepperThreshold);
		else
			Camera.SetPepper(0);
	}
}

/*void CMuCatDoc::WriteStatus(int Pane, LPCTSTR Text, int Value1, int Value2)
{
	CString String;
	String.Format(Text,Value1,Value2);
	CMainFrame* pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	pFrame->GetStatus()->SetPaneText(Pane, String);
}*/

void CMuCatDoc::WriteStatus(int Pane, LPCTSTR Text, ...)
{
	CString String;
	double *Arg = (double*)((&Text)+1);
	String.Format(Text,Arg[0],Arg[1],Arg[2],Arg[3]);
	CMainFrame* pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	pFrame->GetStatus()->SetPaneText(Pane, String);
}

void CMuCatDoc::GetCCDSize()
{
	Camera.GetCCDSize(&m_NumCCDCols, &m_NumCCDRows);
	m_NumActiveCols = Camera.GetNumActiveColumns();
}

int CMuCatDoc::GetNumCCDCols()
{
	return m_NumCCDCols;
}

int CMuCatDoc::GetNumCCDRows()
{
	return m_NumCCDRows;
}

void CMuCatDoc::SetBin(int Bin)
{
	m_Bin = Bin;
	WriteStatus(1,"Bin: %1d",m_Bin);
	m_CCDBinnedHeight = m_NumActiveCols / m_Bin;
	if (m_Header.num_slices > m_CCDBinnedHeight)
		m_Header.num_slices = m_CCDBinnedHeight;
}

void CMuCatDoc::OnParameterGain() 
{
	// TODO: Add your command handler code here
	CGainDialog aDlg;
	aDlg.m_Gain = m_Header.gain;
	if (aDlg.DoModal() == IDOK)
	{
		m_Header.gain = aDlg.m_Gain;
		SetGain();
	}
}


void CMuCatDoc::SetGain()
{
	Camera.SetGain(m_Header.gain, m_Header.speed);
}

void CMuCatDoc::OnPosition()				
{
	

CPositionDialog aDlg;
	aDlg.m_pTurntable = &m_Turntable;
	aDlg.m_pSlide = &m_Slide;
	aDlg.m_pElevator = &m_Elevator;
	aDlg.m_pMagnificationSlide = &m_MagnificationSlide;	
	
	
	aDlg.m_StepSize = CCD_PIXEL_SIZE*m_Bin/m_Header.mag_factor;
	aDlg.m_SourceToCamera = m_SourceToCamera;
	aDlg.m_CameraPixelSize = C843::AngularPixelSize*(m_SourceToCamera*10000)/m_Header.mag_factor;

	aDlg.m_Rotation = m_Theta;
	aDlg.m_Elevation = m_Elevation;

	aDlg.m_Radius = (int)(m_Radius*1000); //convert to microns
	
	//check if we are being called by the app constructor - the first time we are ever called. If so,
	//the software has just started up, and we want Az to be zero, not whatever value it gets from the stage.
	//when you ok the OnPos dialog, it'll move to Az zero.
	if (FirstOnPosCall)
		{
		aDlg.m_Azimuth = 0;
		}
	else
		aDlg.m_Azimuth = floor(C843::CalcAzimuth(m_Radius, m_Slide.GetPosition()/2000.0)+0.5);
	
	//reset the flag, as we shouldn't ever have to force Az to be zero again in this run of the software.
	FirstOnPosCall = false;

	if (aDlg.DoModal()== IDOK)
		{
		SetTheta(aDlg.m_Rotation);
		m_Elevation = aDlg.m_Elevation;
		m_Header.z = (long)aDlg.m_Elevation;
		m_Radius = aDlg.m_Radius/1000.0;  // convert back to mm
		m_Header.distance = m_Radius /10;  //convert to cm
		m_Azimuth = aDlg.m_Azimuth;
		}
	

	//m_Turntable.Move(-1000*(m_Header.theta=(long)m_Theta));
	m_Turntable.Move(-(m_Header.theta=1000*m_Theta+0.5));
	m_Elevator.SetVelocityFraction(0.3);
 	m_Elevator.Move(600000 - 2*aDlg.m_Elevation);
	//m_Elevator.Move(aDlg.m_Elevation);
	//m_Elevator.Move(aDlg.m_Elevation*(-2));
	//m_MagnificationSlide.Move(m_Header.posdistance);
	C843::PolarMove(aDlg.m_Azimuth, m_Radius);
	
	m_Header.pixel_size = CCDPIXELSIZE*m_Bin/m_Header.mag_factor*(m_Header.distance*10)/(m_SourceToCamera*10.0);

	C843::InvalidateProfile();
}

void CMuCatDoc::OnParameterDim() 
{
	// TODO: Add your command handler code here
	CDimDialog aDlg;
	aDlg.m_Height = m_Header.num_slices;
	aDlg.m_MaxHeight = m_CCDBinnedHeight;
	aDlg.m_ScanLength = m_Header.num_samples;
	aDlg.m_NumProj = m_Header.num_proj;
	aDlg.m_NumBlocks = m_Header.num_blocks;
	aDlg.m_Shift = m_Header.slices_per_block;
	aDlg.m_NumFrames = m_Header.num_frames;
	if (m_Header.distance < 0.1 || m_Header.pixel_size < 0.1)
		aDlg.m_Radius = 0;
	else
		aDlg.m_Radius = m_Header.distance * 100000 / m_Header.pixel_size;
	if (aDlg.DoModal() == IDOK)
	{
		m_Header.num_slices = aDlg.m_Height;
		m_Header.num_samples = aDlg.m_ScanLength;
		m_Header.num_proj = aDlg.m_NumProj;
		m_Header.slices_per_block = aDlg.m_Shift;
		m_Header.num_blocks = aDlg.m_NumBlocks;
		m_Header.num_frames = aDlg.m_NumFrames;
		InvalidDueToGeometryChange = true;
	}
	UpdateAllViews(NULL);
}

void CMuCatDoc::SetMaxHeight()
{
	m_CCDBinnedHeight = m_NumActiveCols/m_Bin;
}

void CMuCatDoc::OnParameterMag() 
{
	// TODO: Add your command handler code here
	CMagDialog aDlg;
	aDlg.m_MagFactor = m_Header.mag_factor;
	if (aDlg.DoModal() == IDOK)
		m_Header.mag_factor = aDlg.m_MagFactor;
}

struct thead* CMuCatDoc::GetHeader()
{
	return &m_Header;
}


void CMuCatDoc::OnParameterExp() 
{
	// TODO: Add your command handler code here
	CDurationDialog aDlg;
	aDlg.m_Duration = (int)((m_Header.exposure+500)/1000.0);
	aDlg.m_StillDuration = m_StillExposure;
	if (aDlg.DoModal() == IDOK)
	{
		//DAVE - edit for new exposure
		//Calculate new exposure and round it to nearest 1uS
		m_Header.exposure = aDlg.m_Duration*1000.0;
		RoundExposure();
	}
}

int CMuCatDoc::GetStillExposure()
{
	return m_StillExposure;
}

C843* CMuCatDoc::GetSlide()
{
	return &m_Slide;
}

C843* CMuCatDoc::GetTurntable()
{
	return &m_Turntable;
}

void CMuCatDoc::SetTheta(double Theta)
{
	m_Theta = Theta;
	m_Header.theta = long(m_Theta*1000+0.5);
}

double CMuCatDoc::GetTheta()
{
	return m_Theta;
}

void CMuCatDoc::SetRadius(double Radius)
	{
	m_Radius = Radius;
	m_Header.distance = m_Radius/10;
	}

void CMuCatDoc::SetTranslation(int Translation)
	{
	m_Azimuth = Translation;
	}
C843* CMuCatDoc::GetElevator()
{
	return &m_Elevator;
}

void CMuCatDoc::SetElevation(int Elevation)
{
	m_Elevation = Elevation;
	m_Header.z = m_Elevation;
}

int CMuCatDoc::GetElevation()
{
	return m_Elevation;
}

void CMuCatDoc::OnEditGeometry() 
{
	// TODO: Add your command handler code here
	CGeometry aDlg;
	//aDlg.m_Radius = m_Header.radius/1000;
	//aDlg.m_PixelSize = m_Header.pixel_size;
	//aDlg.m_Gradient = m_Header.gradient;
	float NewDistance;
	bool DistanceChanged = FALSE;
	do
	{
		aDlg.m_Radius = m_Header.distance*10;
		aDlg.m_PixelSize = m_Header.pixel_size;
		aDlg.m_Gradient = m_Header.gradient;
		if (aDlg.DoModal() == IDOK)
		{
			if (fabs(m_Header.distance*10 - aDlg.m_Radius) < .001 || fabs(m_Header.pixel_size - aDlg.m_PixelSize) < .001)
			{
				m_Header.gradient = aDlg.m_Gradient;
				if (aDlg.m_Radius > (m_Header.distance*10 + .001))
				{
					NewDistance = aDlg.m_Radius;
					if (NewDistance > MAXDISTANCE)
						::MessageBox(NULL, "Radius is too large","Error",MB_OK|MB_ICONHAND);
					else
					{
						m_Header.distance = aDlg.m_Radius/10;
						m_Header.pixel_size = (C843::AngularPixelSize*m_Header.distance*10000)/m_Header.mag_factor;
						DistanceChanged = TRUE;
						break;
					}
				}
				if (aDlg.m_PixelSize > (m_Header.pixel_size + .001))
				{
				NewDistance = aDlg.m_PixelSize/((C843::AngularPixelSize*1000)*m_Header.mag_factor);   //FIX ME
					if (NewDistance > MAXDISTANCE)
						::MessageBox(NULL, "Radius is too large","Error",MB_OK|MB_ICONHAND);
					else
					{
						m_Header.pixel_size = aDlg.m_PixelSize;
						m_Header.distance = NewDistance/10;
						DistanceChanged = TRUE;
						break;
					}
				}
				if (aDlg.m_PixelSize < (m_Header.pixel_size-.001) || aDlg.m_Radius <  (m_Header.distance*10-.001))
					::MessageBox(NULL, "Cannot reduce pixel size or source distance here, move magnification slide first","Error",MB_OK|MB_ICONHAND);
				else
					break;
			}
			else
				::MessageBox(NULL,"Cannot specify both pixel size and source distance","Error",MB_OK|MB_ICONHAND);
		}
		else
			break;
	} while (1);
	if (DistanceChanged)
	{
		m_Radius = NewDistance;
		m_Header.posdistance = (long)NewDistance*2000;
		C843::PolarMove(m_Azimuth, NewDistance);
	}
	InvalidDueToGeometryChange = true;
}

void CMuCatDoc::OnButtondim() 
{
	// TODO: Add your command handler code here
	OnParameterDim();
}

C843* CMuCatDoc::GetMagSlide(void)
{
	return &m_MagnificationSlide;
}

void CMuCatDoc::RoundExposure(void)
{
	float tmp_var = (int(m_Header.exposure + 500)/1000)*1000.0/GetNumCCDRows() - ROW_READ_TIME;  //units = ms
	//float tmp_var = (int(m_Header.exposure + 500)/1000)*1000.0/GetNumCCDRows() - m_Header.voltage;			//cheaty access to a float value from the app.
	int tmp_int = tmp_var * (1000/SI_MICROSECOND); // Convert to "Spectral Instruments" microseconds
	//int tmp_int = tmp_var * (1000/m_Header.voltage); // Convert to "Spectral Instruments" microseconds
	if (tmp_int < 0)
		tmp_int = 0;
	tmp_var = m_TDIDelay = tmp_int;
	tmp_var /= (1000/SI_MICROSECOND);
	//tmp_var /= (1000/m_Header.voltage);
	m_Header.exposure = (float)(tmp_var + ROW_READ_TIME)*GetNumCCDRows();
//	m_Header.exposure = (float)(tmp_var + m_Header.voltage)*GetNumCCDRows(); //cheaty access to a float value from the app.
}

void CMuCatDoc::SetPosDistance(int Distance)
	{
	m_Header.posdistance = Distance;
	}


void CMuCatDoc::SetAzimuth(double Azimuth)
	{
	m_Azimuth = Azimuth;
	}
