// MuCatView.h : interface of the CMuCatView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MUCATVIEW_H__BFC9FE2F_EC06_11D1_A406_0000010049AB__INCLUDED_)
#define AFX_MUCATVIEW_H__BFC9FE2F_EC06_11D1_A406_0000010049AB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define CENTRE_OFFSET 4338  // increase to move object image to right

#include <math.h>
#include <vector>
#include <valarray>
#include "nr3.h"
#include "amoeba.h"
using namespace std;


typedef struct tagGBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[256];
} GBITMAPINFO;


class CMuCatView : public CScrollView
{
protected: // create from serialization only
	CMuCatView();
	DECLARE_DYNCREATE(CMuCatView)

// Attributes
public:
	CMuCatDoc* GetDocument();

// Operations
public:
	afx_msg void OnScan();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuCatView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL m_KillZ;
	void CreateBitmap();
	virtual ~CMuCatView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	int m_VerticalPosition;
	BOOL m_mmUnits;
	int m_CentreOffset;
	bool m_IsDark;
	void SaveProjection(CFile &DataFile);
	BOOL m_IsRecording;
	BOOL m_ValidImage;
	void ExecuteLookup();
	void AutoScale();
	BOOL m_AutoScale;
	void CreateLookup();
	unsigned char m_Lookup[65535];
	int m_Contrast;
	int m_Brightness;
	void Initialise();
	static unsigned int m_ScaleTable[41];
	int m_Offset;
	void CreateCCDBuffer();
	unsigned long * m_CCDBuffer;
	unsigned long ** m_CCDPixel;
	BOOL m_Stop;
	int m_Zoom;
	void Resize();
	static BOOL m_Initialised;
	bool m_valid_backgrounds;
	unsigned int m_YSize;
	unsigned int m_XSize;
	unsigned char **m_BitmapArray;
	GBITMAPINFO m_BitmapInfo;
	//{{AFX_MSG(CMuCatView)
	afx_msg void OnStill();
	afx_msg void OnCancelMode();
	afx_msg void OnEditCopy();
	afx_msg void OnViewZoom();
	afx_msg void OnFocus();
	afx_msg void OnInitFocus();
	afx_msg void OnStop();
	afx_msg void OnAutoscale();
	afx_msg void OnBrightDown();
	afx_msg void OnBrightUp();
	afx_msg void OnContrastDown();
	afx_msg void OnContrastUp();
	afx_msg void OnRecord();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButonDown(UINT nFlags,CPoint point);
	afx_msg void OnButtonsharpness();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	unsigned char * m_BitmapBuffer;
	unsigned short *TDIBuffer ,**TDIArray;
	valarray<unsigned long> dark;
	valarray<unsigned long> light;
	valarray<unsigned long> data;
	valarray<double> lightfixed;
	valarray<double> datafixed;
	valarray<double> image;
protected:
	float m_MeasuredCentre;
	void OnButtonCentre(void);
	float m_MeasuredWidth;
	BOOL m_XrayPCShutdown;
	int m_NumRescans;
	int m_ShutterOpen;
public:
	void OnUpdateShutter(CCmdUI* pCmdUI);
	void OnShutter(void);
protected:
	unsigned long m_IZero;
	bool m_KeepShutterOpen;
	unsigned short*m_Line;
private:
	// just a flag for the sharpness button. If set, the OnDraw will try to fit a line profile to the image in the display, for left/right shadows in the sharpness measurements
	bool m_Are_doing_sharpness;
public:
	afx_msg void OnLocateCamSource();
	vector<float> calc_pixel_size_quadrant;
	vector<float> tophalf;
	vector<float> pixel_mag_lookup;
	float m_SourceToCamera;
	void GetSourceCameraDistance(void);

	vector<float> * Get_pixel_mag_lookup(void);
	void CreatePixelMagLookup(void);
	bool m_valid_pix_mag_lookup;
	void OnButtonHome(void);
	// //stores the state of TDIBuffer and TDIArray on the first attempt to use them.
	static int buffers_init;
	void CreateTDIBuffers(int X, int y, int bin);
	// Rewrites the Phi Profile to start at the current rotation stage position
	void FixPhiStart(void);
	bool IsPMLValid;


private:
	int sharp_x;
	int sharp_y;
public:
	// uses DownHill Simplex method (Amoeba) to minimize sharpness value (maximize sharpness)
	void OnAutoFocus(void);
	double TDISharp(VecDoub_I &z);
	float Sharpness;
	void RecordWaitPause(char *FileName, CFile *DataFile, CFileDialog *FD);
	void IncrementRotationProfile(double currentposition, double ThetaStep);
};

#ifndef _DEBUG  // debug version in MuCatView.cpp
inline CMuCatDoc* CMuCatView::GetDocument()
   { return (CMuCatDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUCATVIEW_H__BFC9FE2F_EC06_11D1_A406_0000010049AB__INCLUDED_)
