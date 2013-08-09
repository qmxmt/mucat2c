// PositionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MuCat.h"
#include "PositionDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPositionDialog dialog


CPositionDialog::CPositionDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPositionDialog::IDD, pParent)
	, m_CameraPixelSize(0)
	{
	//{{AFX_DATA_INIT(CPositionDialog)
	m_Rotation = 0.0;
	m_Azimuth = 0;
	m_Elevation = 0;
	m_Radius = 0;
	//}}AFX_DATA_INIT
}


void CPositionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPositionDialog)
	DDX_Text(pDX, IDC_ROTATION, m_Rotation);
	DDX_Text(pDX, IDC_TRANSLATION, m_Azimuth);
	DDX_Text(pDX, IDC_ELEVATION, m_Elevation);
	DDV_MinMaxInt(pDX, m_Elevation, 0, 300000);
	DDX_Text(pDX, IDC_POSDISTANCE, m_Radius);
	DDV_MinMaxInt(pDX, m_Radius, 0, 9106015);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPositionDialog, CDialog)
	//{{AFX_MSG_MAP(CPositionDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_ROTATIONSPIN, OnDeltaposRotationspin)
	ON_EN_CHANGE(IDC_ROTATION, OnChangeRotation)
	ON_NOTIFY(UDN_DELTAPOS, IDC_TRANSLATIONSPIN, OnDeltaposTranslationspin)
	ON_EN_CHANGE(IDC_ELEVATION, OnChangeElevation)
	ON_NOTIFY(UDN_DELTAPOS, IDC_ELEVATIONSPIN, OnDeltaposElevationspin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_POSDISTANCESPIN, OnDeltaposPosdistancespin)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_POSDISTANCE, &CPositionDialog::OnEnChangePosdistance)
	ON_EN_CHANGE(IDC_TRANSLATION, &CPositionDialog::OnEnChangeTranslation)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPositionDialog message handlers

BOOL CPositionDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CSpinButtonCtrl *pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_ROTATIONSPIN);
	pSpin->SetBuddy(GetDlgItem(IDC_ROTATION));
	pSpin->SetRange32(-360000, 360000);
	m_MaxTranslation = (int)(299000.0/m_StepSize);
	m_OldTranslation = m_Azimuth;
	m_pSlide->SetVelocityFraction(0.5);
	UDACCEL UDAccel[2]={0,1,3,5};
	pSpin->SetAccel(2,UDAccel);
	
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_POSDISTANCESPIN);
	pSpin->SetBuddy(GetDlgItem(IDC_POSDISTANCE));
	pSpin->SetRange32(-30000, 190000);
	//pSpin->SetRange32(-140000, MAXDISTANCE);
	UDACCEL UDAccelP[3]={0,1,1,10,3,250};
	pSpin->SetAccel(3,UDAccelP);

	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_TRANSLATIONSPIN);
	pSpin->SetBuddy(GetDlgItem(IDC_TRANSLATION));
	pSpin->SetRange32(-306000, 306000);
	//pSpin->SetRange(0, m_MaxTranslation);
	//pSpin->SetRange(-m_MaxTranslation, m_MaxTranslation);

	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_ELEVATIONSPIN);
	pSpin->SetBuddy(GetDlgItem(IDC_ELEVATION));
	pSpin->SetRange32(0, 300000);
	//pSpin->SetRange32(-300000, 300000);
	UDACCEL UDAccelE[3]={0,10,1,100,3,250};
	pSpin->SetAccel(3,UDAccelE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPositionDialog::OnDeltaposRotationspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	if (pNMUpDown->iPos == 0)
		pNMUpDown->iDelta += (int)m_Rotation;
	m_pTurntable->Move(-1000*(pNMUpDown->iPos + pNMUpDown->iDelta));
	*pResult = 0;
}


void CPositionDialog::OnChangeRotation() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
}

void CPositionDialog::OnDeltaposTranslationspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	*pResult = 0;
	if (!C843::IsMoving())
		{
		C843::PolarMove(pNMUpDown->iPos,m_Radius/1000.0);
		}
	else
		*pResult = 1; // do not allow change
	//	{pNMUpDown->iPos -= pNMUpDown->iDelta;}
}

void CPositionDialog::OnChangeElevation() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CPositionDialog::OnDeltaposElevationspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	int distance = 2*(300000 - (pNMUpDown->iPos + pNMUpDown->iDelta));
	m_pElevator->Move(distance);
	//m_pElevator->Move((int)(pNMUpDown->iPos + pNMUpDown->iDelta)*(-2));
	*pResult = 0;
}

void CPositionDialog::OnDeltaposPosdistancespin(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	*pResult = 0;
	if (!C843::IsMoving())
		{
		C843::PolarMove(m_Azimuth,(pNMUpDown->iPos + pNMUpDown->iDelta)/1000);
		}
	else
	{
		*pResult = 1;  //do not allow change to iPos
		pNMUpDown->iDelta = 0;
	}
	//	{pNMUpDown->iPos -= pNMUpDown->iDelta;}
	
	
	CWnd* pWnd = GetDlgItem(IDC_POSITIONPIXELSIZE);
	CDC* pDC = pWnd->GetDC();
	CString Text;
	//float PixelSize = m_CameraPixelSize*(pNMUpDown->iPos + pNMUpDown->iDelta)/m_SourceToCamera/10;
	float PixelSize = (C843::AngularPixelSize*1000*(pNMUpDown->iPos + pNMUpDown->iDelta))/1000;
	Text.Format("Pixel Size: %6.3f",PixelSize);
//	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(0,0,255));
	pDC->TextOut(0,0,Text);

}


void CPositionDialog::OnEnChangePosdistance()
	{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	}


void CPositionDialog::OnEnChangeTranslation()
	{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	}
