// MuCatView.cpp : implementation of the CMuCatView class
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "afxpriv.h"
#include "MuCat.h"
#include "convert.h"
#include "MuCatDoc.h"
#include "MuCatView.h"
#include "ZoomDialog.h"
#include "StopDialog.h"
#include "FinishTime.h"
#include "PauseDialog.h"
#include "XrayRestartDialog.h"
#include <vector>
#include <valarray>
#include <fstream>
#include <sstream> 


using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RADtoDEG (180.0/3.1415926535)
CLog Log;
/////////////////////////////////////////////////////////////////////////////
// CMuCatView

IMPLEMENT_DYNCREATE(CMuCatView, CScrollView)

	BEGIN_MESSAGE_MAP(CMuCatView, CScrollView)
		//{{AFX_MSG_MAP(CMuCatView)
		ON_COMMAND(ID_STILL, OnStill)
		ON_WM_CANCELMODE()
		ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
		ON_COMMAND(ID_VIEW_ZOOM, OnViewZoom)
		ON_COMMAND(ID_FOCUS, OnFocus)
		ON_COMMAND(ID_INITFOCUS, OnInitFocus)
		ON_COMMAND(ID_STOP, OnStop)
		ON_COMMAND(ID_AUTOSCALE, OnAutoscale)
		ON_COMMAND(ID_BRIGHT_DOWN, OnBrightDown)
		ON_COMMAND(ID_BRIGHT_UP, OnBrightUp)
		ON_COMMAND(ID_CONTRAST_DOWN, OnContrastDown)
		ON_COMMAND(ID_CONTRAST_UP, OnContrastUp)
		ON_COMMAND(ID_RECORD, OnRecord)
		ON_WM_MOUSEMOVE()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_RBUTTONDOWN()
		ON_COMMAND(ID_SCAN, OnScan)
		ON_WM_SIZE()
		ON_COMMAND(ID_BUTTONSHARPNESS, OnButtonsharpness)
		ON_COMMAND(ID_BUTTONCENTRE, OnButtonCentre)
		ON_COMMAND(ID_HOME, OnButtonHome)
		ON_COMMAND(ID_SHUTTERBUTTON, OnShutter)
		ON_UPDATE_COMMAND_UI(ID_SHUTTERBUTTON, OnUpdateShutter)
		//}}AFX_MSG_MAP
		// Standard printing commands
		ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
		ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
		ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
		ON_COMMAND(ID_LOCATE_CAM_SOURCE, &CMuCatView::OnLocateCamSource)
	END_MESSAGE_MAP()

	int CMuCatView::buffers_init = 0;
	BOOL CMuCatView::m_Initialised = 0;
	unsigned int CMuCatView::m_ScaleTable[41];
	template <typename T> void dump(ostream& out, const valarray<T>& theValArray);
	
	/////////////////////////////////////////////////////////////////////////////
	// CMuCatView construction/destruction

	CMuCatView::CMuCatView()
		: m_MeasuredCentre(0)
		, m_MeasuredWidth(0)
		, m_XrayPCShutdown(false)
		, m_NumRescans(0)
		, m_ShutterOpen(0)
		, m_IZero(0)
		, m_KeepShutterOpen(false)
		, m_Line(NULL)
		, m_Are_doing_sharpness(false)
		, m_SourceToCamera(0)
		, m_valid_pix_mag_lookup(false)
		, m_valid_backgrounds(false)
		//, buffers_init(0)
		, IsPMLValid(false)
		, sharp_x(0)
		, sharp_y(0)
		{
		// TODO: add construction code here
		if (!m_Initialised)
			Initialise();
		m_BitmapArray = NULL;
		m_BitmapBuffer = NULL;
		m_CCDBuffer = NULL;
		m_CCDPixel = NULL;
		m_XSize = m_YSize = 0;
		m_Zoom = 12;
		m_AutoScale = TRUE;
		m_ValidImage = FALSE;
		m_IsRecording = FALSE;
		m_IsDark = FALSE;
		m_CentreOffset = CENTRE_OFFSET;
		m_mmUnits = TRUE;
		TDIArray = NULL;
		TDIBuffer = NULL;

		

		}

	CMuCatView::~CMuCatView()
		{
		if (m_CCDBuffer)
			delete m_CCDBuffer;
		if (m_CCDPixel)
			delete m_CCDPixel;
		if (m_Line)
			delete m_Line;
		if (m_BitmapArray != NULL)
			delete [] m_BitmapArray;
		if (m_BitmapBuffer != NULL)
			delete [] m_BitmapBuffer;
		if (TDIArray){
			delete  TDIArray;
			delete  TDIBuffer;
			}
		//if (TDIArray)
		//	delete TDIArray;
		//	delete TDIBuffer;
		//if (TDIBuffer)
		//	{delete [] TDIBuffer;
		//delete [] TDIArray;}
		/*	CMuCatDoc *pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		pDoc->GetElevator()->Move(280000);
		pDoc->GetMagSlide()->Move(170000);*/
		}

	BOOL CMuCatView::PreCreateWindow(CREATESTRUCT& cs)
		{
		// TODO: Modify the Window class or styles here by modifying
		//  the CREATESTRUCT cs

		return CScrollView::PreCreateWindow(cs);
		}

	/////////////////////////////////////////////////////////////////////////////
	// CMuCatView drawing

	void CMuCatView::OnDraw(CDC* pDC)
		{
		// TODO: add draw code for native data here
		CMuCatDoc *pDoc = GetDocument();
		ASSERT_VALID(pDoc);

		if (m_BitmapBuffer == NULL)
			{
			pDC->TextOut(0,0,"New",3);
			return;
			}
		/*	SetDIBitsToDevice(pDC->GetSafeHdc(), 0,0, m_XSize,m_YSize, 0,0, 0,m_YSize,\
		m_BitmapBuffer, (BITMAPINFO*)&m_BitmapInfo,DIB_RGB_COLORS);*/
		SetStretchBltMode(pDC->m_hDC, HALFTONE);
		StretchDIBits(pDC->GetSafeHdc(), 0,0,m_XSize*m_Zoom/12, m_YSize*m_Zoom/12, 0,0, m_XSize,\
			m_YSize, m_BitmapBuffer,(BITMAPINFO*)&m_BitmapInfo,DIB_RGB_COLORS,SRCCOPY);

		CPen Pen;
		Pen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
		pDC->SelectObject(&Pen);
		int Height = pDoc->GetHeader()->num_slices;
		 int Y=(m_YSize/2-Height/2)*m_Zoom/12-1;
		 int X=(m_XSize*m_Zoom/12)-1;
		pDC->MoveTo(0,Y);
		pDC->LineTo(X-1,Y);
		Y += Height*m_Zoom/12+1;
		pDC->MoveTo(0,Y);
		pDC->LineTo(X-1,Y);
		Y = m_YSize*m_Zoom/12-1;
		pDC->MoveTo(32*m_Zoom/12, 0);
		pDC->LineTo(32*m_Zoom/12, Y);
		pDC->MoveTo((m_XSize-33)*m_Zoom/12, 0);
		pDC->LineTo((m_XSize-33)*m_Zoom/12, Y);

		Y=(m_YSize/2)*m_Zoom/12-1;
		X=(m_XSize*m_Zoom/12)-1;
		CPen DotPen;
		DotPen.CreatePen(PS_DOT, 1, RGB(0,0,255));
		pDC->SelectObject(&DotPen);
		pDC->SetBkMode(TRANSPARENT);
		pDC->MoveTo(0,Y);
		pDC->LineTo(X,Y);

		Height = pDoc->GetHeader()->slices_per_block;
		if (Height > pDoc->GetHeader()->num_slices)
			return;
		Y=(m_YSize/2-Height/2)*m_Zoom/12-1;
		X=(m_XSize*m_Zoom/12)-1;
		CPen DashPen;
		DashPen.CreatePen(PS_DASH, 1, RGB(0,255,0));
		pDC->SelectObject(&DashPen);
		pDC->SetBkMode(TRANSPARENT);
		pDC->MoveTo(0,Y);
		pDC->LineTo(X,Y);
		Y += Height*m_Zoom/12+1;
		pDC->MoveTo(0,Y);
		pDC->LineTo(X,Y);

		if(m_Are_doing_sharpness)
			//Do the line profiles across the display
			{
			COLORREF Pixeldata;
			BYTE Grey;
			BYTE Blue, Green, Red, Padding;
			int plotY;

			//Lower half of display
			Y = (m_YSize/2) + (Height/4)*m_Zoom/12-1;

			for(X=0; X < (m_XSize*m_Zoom/12)-1; X++)
				{
				Pixeldata = pDC->GetPixel(X,Y);
				Padding	= ((Pixeldata >> 24) & 0xff);
				Blue	=  0.114 * ((Pixeldata >> 16) & 0xff);
				Green	=  0.587 * ((Pixeldata >> 8) & 0xff);
				Red		=  0.299 * (Pixeldata & 0xff);
				Grey = Red + Green + Blue;
				plotY = (Y+Grey/2);
				if(Grey<200) 
					pDC->SetPixel(X,plotY,RGB(255,255,0));
				else
					pDC->SetPixel(X,plotY,RGB(0,0,0));
				}

			//Upper half of display
			Y = (m_YSize/2) - (Height/4)*m_Zoom/12-1;

			for(X=0; X < (m_XSize*m_Zoom/12)-1; X++)
				{
				Pixeldata = pDC->GetPixel(X,Y);
				Padding	= ((Pixeldata >> 24) & 0xff);
				Blue	=  0.114 * ((Pixeldata >> 16) & 0xff);
				Green	=  0.587 * ((Pixeldata >> 8) & 0xff);
				Red		=  0.299 * (Pixeldata & 0xff);
				Grey = Red + Green + Blue;
				plotY = (Y+Grey/2);
				if(Grey<200) 
					pDC->SetPixel(X,plotY,RGB(255,255,0));
				else
					pDC->SetPixel(X,plotY,RGB(0,0,0));
				}
			}
		m_Are_doing_sharpness = false;
		}

	void CMuCatView::OnInitialUpdate()
		{
		CScrollView::OnInitialUpdate();
		CSize sizeTotal;
		// TODO: calculate the total size of this view
		sizeTotal.cx = sizeTotal.cy = 100;
		SetScrollSizes(MM_TEXT, sizeTotal);
		}

	/////////////////////////////////////////////////////////////////////////////
	// CMuCatView printing

	BOOL CMuCatView::OnPreparePrinting(CPrintInfo* pInfo)
		{
		// default preparation
		return DoPreparePrinting(pInfo);
		}

	void CMuCatView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
		{
		// TODO: add extra initialization before printing
		}

	void CMuCatView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
		{
		// TODO: add cleanup after printing
		}

	/////////////////////////////////////////////////////////////////////////////
	// CMuCatView diagnostics

#ifdef _DEBUG


	void CMuCatView::AssertValid() const
		{
		CScrollView::AssertValid();
		}

	void CMuCatView::Dump(CDumpContext& dc) const
		{
		CScrollView::Dump(dc);
		}

	CMuCatDoc* CMuCatView::GetDocument() // non-debug version is inline
		{
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMuCatDoc)));
		return (CMuCatDoc*)m_pDocument;
		}
#endif //_DEBUG

	/////////////////////////////////////////////////////////////////////////////
	// CMuCatView message handlers

	void CMuCatView::OnStill() 
		{
		// TODO: Add your command handler code here
		int Bin;
		float Pressure, Temperature;
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		Bin = pDoc->GetBin();
		m_XSize = (pDoc->GetNumCCDRows()-40)/Bin;
		m_YSize = pDoc->GetNumCCDCols()/Bin;
		pDoc->WriteStatus(0,"Creating Lookups...");
		CreateBitmap();
		CreateCCDBuffer();
		m_valid_backgrounds = false;
		pDoc->WriteStatus(0,"Capturing Image");
		Camera.StillImage((float)(pDoc->GetStillExposure()*1000),m_CCDPixel, m_XSize, m_YSize, pDoc->GetBin(),pixel_mag_lookup,m_IsDark);

		m_ValidImage = TRUE;
		if (m_AutoScale)
			AutoScale();
		CreateLookup();
		ExecuteLookup();
		Resize();
		Invalidate();
		Camera.GetTempPressure(&Temperature, &Pressure);
		pDoc->WriteStatus(3,"T = %5.1f C,  P = %5.2f Torr",Temperature,Pressure);
		}

	void CMuCatView::CreateBitmap()
		{
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		int XRoundupSize;
		if (m_XSize % 4)
			XRoundupSize = m_XSize + 4 - (m_XSize % 4);
		else
			XRoundupSize = m_XSize;
		if (m_BitmapArray != NULL)
			delete [] m_BitmapArray;
		if (m_BitmapBuffer != NULL)
			delete [] m_BitmapBuffer;
		m_BitmapBuffer = new unsigned char [m_YSize*XRoundupSize];
		m_BitmapArray = new unsigned char * [m_YSize];
		for (unsigned int a=0; a<m_YSize; ++a)
			m_BitmapArray[a] = &m_BitmapBuffer[a*XRoundupSize];

		for (unsigned int a=0; a<m_YSize; ++a)
			{
			for (int b=0; b<XRoundupSize; ++b)
				m_BitmapArray[a][b] = b*256/XRoundupSize;
			}

		m_BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_BitmapInfo.bmiHeader.biWidth = XRoundupSize;
		m_BitmapInfo.bmiHeader.biHeight = m_YSize;
		m_BitmapInfo.bmiHeader.biPlanes = 1;
		m_BitmapInfo.bmiHeader.biBitCount = 8;
		m_BitmapInfo.bmiHeader.biCompression = BI_RGB;
		m_BitmapInfo.bmiHeader.biSizeImage = 0;
		m_BitmapInfo.bmiHeader.biClrUsed = 256;
		m_BitmapInfo.bmiHeader.biClrImportant = 0;
		for (int a=0; a<256; ++a)
			{
			m_BitmapInfo.bmiColors[a].rgbRed = a;
			m_BitmapInfo.bmiColors[a].rgbGreen = a;//a*9/10;
			m_BitmapInfo.bmiColors[a].rgbBlue = a;//a*2/3;
			}

		if(pDoc->InvalidDueToGeometryChange || !m_valid_pix_mag_lookup)	//have we resized the scan dimensions - recreate the distortion map if so
			{
			CreatePixelMagLookup();
			pDoc->InvalidDueToGeometryChange = false;
			}
		}

	void CMuCatView::Resize()
		{
		RECT ChildRect,FrameRect;
		int Xmax,Ymax;
		BOOL OverSize = FALSE;
		GetParentFrame()->GetParentFrame()->GetClientRect(&FrameRect);
		Xmax = FrameRect.right;
		Ymax = FrameRect.bottom-50;
		//	Ymax = 100;
		SIZE Size = {m_XSize*m_Zoom/12, m_YSize*m_Zoom/12};
		SetScrollSizes(MM_TEXT, Size);
		ResizeParentToFit(FALSE);
		SetScrollSizes(MM_TEXT, Size);
		ResizeParentToFit();
		GetParentFrame()->GetWindowRect(&ChildRect);
		if ((ChildRect.right - ChildRect.left) > Xmax)
			OverSize = TRUE;
		else
			Xmax = ChildRect.right - ChildRect.left;
		if ((ChildRect.bottom - ChildRect.top) > Ymax)
			OverSize = TRUE;
		else
			Ymax = ChildRect.bottom - ChildRect.top;
		if (OverSize)
			{
			GetParentFrame()->SetWindowPos(&wndTop, 0,0, Xmax, Ymax, SWP_NOMOVE);
			SetScrollSizes(MM_TEXT, Size);
			}
		}


	void CMuCatView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
		{
		// TODO: Add your specialized code here and/or call the base class

		CScrollView::OnPrepareDC(pDC, pInfo);
		pDC->SetMapMode(MM_TEXT);
		}

	void CMuCatView::OnCancelMode() 
		{
		CScrollView::OnCancelMode();

		// TODO: Add your message handler code here

		}

	void CMuCatView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
		{
		// TODO: Add your specialized code here and/or call the base class
		int XPrintSize, YPrintSize, XScale, YScale, Scale;
		if (m_BitmapBuffer == NULL)
			return;
		XPrintSize = pDC->GetDeviceCaps(HORZRES);
		YPrintSize = pDC->GetDeviceCaps(VERTRES);
		XScale = XPrintSize/m_XSize;
		YScale = YPrintSize/m_YSize;
		if (XScale < YScale)
			Scale = XScale;
		else
			Scale = YScale;
		StretchDIBits(pDC->GetSafeHdc(), 0,0,m_XSize*Scale, m_YSize*Scale, 0,0, m_XSize,\
			m_YSize, m_BitmapBuffer,(BITMAPINFO*)&m_BitmapInfo,DIB_RGB_COLORS,SRCCOPY);
		}


	void CMuCatView::OnEditCopy() 
		{
		// TODO: Add your command handler code here
		CClientDC ClientDC(this);
		CBitmap Image;	
		Image.CreateCompatibleBitmap(&ClientDC, m_XSize, m_YSize);
		SetDIBits(ClientDC.GetSafeHdc()
			,HBITMAP(Image),0,m_YSize,m_BitmapBuffer,(BITMAPINFO*)&m_BitmapInfo,DIB_RGB_COLORS);
		/*	CDC MemDC;
		MemDC.CreateCompatibleDC(&ClientDC);
		MemDC.SelectObject(&Image);*/
		if (!OpenClipboard())
			return;
		::EmptyClipboard();
		::SetClipboardData(CF_BITMAP, Image.m_hObject);
		Image.Detach();
		::CloseClipboard();
		}

	void CMuCatView::OnViewZoom() 
		{
		// TODO: Add your command handler code here
		CZoomDialog aDlg;
		aDlg.m_Zoom = m_Zoom;
		if (aDlg.DoModal() == IDOK)
			m_Zoom = aDlg.m_Zoom;
		Resize();
		Invalidate();
		}

	void CMuCatView::OnInitFocus() 
		{
		unsigned int Bin,ii;
		CMuCatDoc* pDoc = GetDocument();
		Bin = pDoc->GetBin();
		m_XSize = (pDoc->GetNumCCDRows()-40)/Bin;
		m_YSize = pDoc->GetNumCCDCols()/Bin;
		dark.resize(m_XSize*m_YSize);
		light.resize(m_XSize*m_YSize);
		data.resize(m_XSize*m_YSize);
		
		CreateBitmap();
		CreateCCDBuffer();
		ASSERT_VALID(pDoc);
		
		if (!m_valid_backgrounds)
			{

				Camera.StillImage((float)(pDoc->GetStillExposure()*1000),m_CCDPixel, m_XSize, m_YSize, pDoc->GetBin(),pixel_mag_lookup,true);
				for(ii=0;ii<dark.size(); ii++)
					dark[ii] = m_CCDBuffer[ii];
				long elepos = pDoc->GetElevation();
				pDoc->GetElevator()->Move(C843::mmToElevation(280));	//move stage down out of the way
				pDoc->GetElevator()->WaitForStop();
			
				Camera.StillImage((float)(pDoc->GetStillExposure()*1000),m_CCDPixel, m_XSize, m_YSize, pDoc->GetBin(),pixel_mag_lookup,false);
				for(ii=0;ii<light.size(); ii++)
				light[ii] = m_CCDBuffer[ii];
				
				pDoc->GetElevator()->Move(C843::ElevationtoElevationMove(elepos));
				pDoc->GetElevator()->WaitForStop();
								
				lightfixed.resize(light.size());
				for (ii=0;ii<dark.size();ii++)
					{
					lightfixed[ii]=light[ii]-dark[ii];
					}
				light.resize(0);	//no need for unfixed light image data now
				
				m_valid_backgrounds = true;

			}
				
				Camera.StillImage((float)(pDoc->GetStillExposure()*1000),m_CCDPixel, m_XSize, m_YSize, pDoc->GetBin(),pixel_mag_lookup,false);
				for(ii=0;ii<data.size(); ii++)
					data[ii] = m_CCDBuffer[ii];


		//fstream file ("dark.bin",ios::in|ios::out|ios::binary|ios::trunc);
		//if (file.is_open())
		//	dump(file,dark);
		//	//file.write ((char*)&*(pixel_mag_lookup.begin()),pixel_mag_lookup.size() * sizeof(float));
		//file.close();
		//	
		//fstream file2 ("light.bin",ios::in|ios::out|ios::binary|ios::trunc);
		//if (file2.is_open())
		//	dump(file2,light);
		//	//file.write ((char*)&*(pixel_mag_lookup.begin()),pixel_mag_lookup.size() * sizeof(float));
		//file2.close();


		//valarray<double> lightfixed(light.size());
		

		valarray<double> datafixed(data.size());
		
		for (ii=0;ii<dark.size();ii++)
			{
			datafixed[ii]=data[ii]-dark[ii];
			}
		
		valarray<double> image(data.size());
		for (ii=0;ii<dark.size();ii++)
			{
			image[ii]= (datafixed[ii] / lightfixed[ii])*30000;
			if (image[ii] > 60000)
				image[ii] = 60000;
			if (image[ii] < 0)
				image[ii] = 0;
			}
		

		//fstream file ("image.bin",ios::in|ios::out|ios::binary|ios::trunc);
		//if (file.is_open())
		//	dump(file,image);
		//	//file.write ((char*)&*(pixel_mag_lookup.begin()),pixel_mag_lookup.size() * sizeof(float));
		//file.close();
			
		//Bitmaps and CCDbuffers are dirty, dump them and recreate
		CreateBitmap();
		CreateCCDBuffer();

		//put our image into a sparkly clean buffer. No nasty out of bounds accesses now.
		for (unsigned int a=0; a<m_XSize-1; ++a)
			{

			for (unsigned int b=0; b<m_YSize; ++b)
				{
				m_CCDPixel[b][a] = (unsigned long) image[a+b*m_XSize];
				}
			}
		
		
			m_ValidImage = TRUE;
		if (m_AutoScale)
			AutoScale();
		CreateLookup();
		ExecuteLookup();
		Resize();
		Invalidate();
		m_Stop = true;
		OnButtonsharpness();
		//do
		//	{
		//	OnFocus();
		//	} while (!(GetQueueStatus(QS_INPUT)&4) && !m_Stop);
		//	//	aDlg.DestroyWindow();
		}

	void CMuCatView::OnFocus() 
		{
		// TODO: Add your command handler code here
		int Bin;
//		float Pressure, Temperature;
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		Bin = pDoc->GetBin();
		m_XSize = (pDoc->GetNumCCDRows()-40)/Bin;
		m_YSize = pDoc->GetNumCCDCols()/Bin;
		CreateBitmap();
		CreateCCDBuffer();







		
		
		////	int Max;
		//float Temperature, Pressure;
		//CMuCatDoc* pDoc = GetDocument();
		//ASSERT_VALID(pDoc);
		//Camera.ExposeandReadoutRegion((float)(pDoc->GetStillExposure()*1000),m_CCDPixel, m_XSize/2, m_YSize/2, m_XSize, m_YSize, 1);
		////	Invalidate();
		//m_ValidImage = TRUE;
		//if (m_AutoScale)
		//	AutoScale();
		//CreateLookup();
		//ExecuteLookup();
		//Invalidate();
		//Camera.GetTempPressure(&Temperature, &Pressure);
		//pDoc->WriteStatus(3,"T = %5.1f C,  P = %5.2f Torr",Temperature,Pressure);
		//OnDraw(GetDC());
		//	pDoc->WriteStatus(7,"Max %d",Max);
		}


	void CMuCatView::OnStop() 
		{
		// TODO: Add your command handler code here
		m_Stop = TRUE;
		}

	void CMuCatView::CreateCCDBuffer()
		{
		unsigned int a;
		if (m_CCDBuffer)
			delete [] m_CCDBuffer;
		if (m_CCDPixel)
			delete [] m_CCDPixel;
		if (m_Line)
			delete m_Line;
		m_CCDBuffer = new unsigned long[m_XSize*m_YSize];
		m_CCDPixel = new unsigned long*[m_YSize];
		m_Line = new unsigned short[m_XSize];
		for (a=0; a<m_YSize; ++a)
			m_CCDPixel[a] = &m_CCDBuffer[a*m_XSize];
		}

	void CMuCatView::OnAutoscale() 
		{
		// TODO: Add your command handler code here
		if (!m_ValidImage)
			return;
		m_AutoScale = TRUE;
		AutoScale();
		CreateLookup();
		ExecuteLookup();
		Invalidate(FALSE);
		}

	void CMuCatView::OnBrightUp() 
		{
		// TODO: Add your command handler code here
		if (!m_ValidImage)
			return;
		m_AutoScale = FALSE;
		if (m_Brightness > 0)
			m_Brightness -= 16;
		else
			m_Brightness -= 48;
		m_Brightness = m_Brightness/32*32;
		CreateLookup();
		ExecuteLookup();
		Invalidate(FALSE);
		}

	void CMuCatView::OnBrightDown() 
		{
		// TODO: Add your command handler code here
		if (!m_ValidImage)
			return;
		m_AutoScale = FALSE;
		if (m_Brightness >= 0)
			m_Brightness += 48;
		else
			m_Brightness += 16;
		m_Brightness = (m_Brightness)/32*32;
		CreateLookup();
		ExecuteLookup();
		Invalidate(FALSE);
		}

	void CMuCatView::OnContrastDown() 
		{
		// TODO: Add your command handler code here
		if (!m_ValidImage)
			return;
		m_AutoScale = FALSE;
		if (m_Contrast > 0)
			--m_Contrast;
		CreateLookup();
		ExecuteLookup();
		Invalidate(FALSE);
		}

	void CMuCatView::OnContrastUp() 
		{
		// TODO: Add your command handler code here
		if (!m_ValidImage)
			return;
		m_AutoScale = FALSE;
		if (m_Contrast < 40)
			++m_Contrast;
		CreateLookup();
		ExecuteLookup();
		Invalidate(FALSE);
		}

	void CMuCatView::Initialise()
		{
		m_Initialised = TRUE;
		double Factor = pow(2.0,0.2);
		double Scale = 256;
		for (int a=0; a<41; ++a)
			{
			m_ScaleTable[a] = (unsigned int)(Scale+0.5);
			Scale *= Factor;
			}
		}

	void CMuCatView::CreateLookup()
		{
		int Start,End,Scale,a;
		Scale = m_ScaleTable[m_Contrast];
		if (Scale == 0) Scale = 1;			//fix div by zero
		Start = 65536*m_Brightness/Scale;
		End = 65536*(m_Brightness+255)/Scale;
		if (Start < 0)
			Start = 0;
		if (End > 65535)
			End = 65535;
		for (a=0; a<=Start; ++a)
			m_Lookup[a] = 0;
		for (; a<End; ++a)
			m_Lookup[a] = ((Scale*a)>>16) - m_Brightness;
		for (; a<65535; ++a)
			m_Lookup[a] = 255;
		}

	void CMuCatView::AutoScale()
		{
		unsigned long Min, Max, *line;
		int a, b, Scale, EndX, EndY, Avg;
		double FScale,Sum;
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		Min = 65535;
		Max = 0;
		Sum = 0;
		EndY = m_YSize - 4;
		EndX = m_XSize - 4;
		for (a=0; a<EndY; a += 4)
			{
			line = m_CCDPixel[a];
			for (b=0; b<EndX; ++b)
				{
				if (line[b] > Max)
					Max = line[b];
				else
					{
					if (line[b] < Min)
						Min = line[b];
					}
				Sum += line[b];
				}
			}
		Avg = (int)(Sum/(EndX*EndY)*4);
		FScale = 65535.0 * 255.99 / (Max - Min);
		a=0;
		while (m_ScaleTable[a] < FScale && a < 41)
			++a;
		m_Contrast = a-1;
		Scale = m_ScaleTable[a-1];
		m_Brightness = Min * Scale/65536;
		Log.Write("Max: %d",Max);
		pDoc->WriteStatus(7,"Int. %d/%d",Max,Avg);
		}

	void CMuCatView::ExecuteLookup()
		{
		unsigned long *CCDLine;
		unsigned char *BitmapLine;
		unsigned int a,b;
		for (a=0; a<m_YSize; ++a)
			{
			CCDLine = m_CCDPixel[a];
			BitmapLine = m_BitmapArray[a];
			for (b=0; b<m_XSize-1; ++b)
				BitmapLine[b] = m_Lookup[CCDLine[b]];\
			}
		}

	void CMuCatView::OnScan() 
		{


		int Bin, Frame=0, NumFrames=1;
		//unsigned int Velocity;
		float Temperature, Pressure;
		struct thead *pHeader;
		double PixelStep;
		double radius;
		double start_angle;
		double travel_angle;
		double start_phi;
		signed int direction = 0;

		CTime StartTime = CTime::GetCurrentTime();
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		m_VerticalPosition = pDoc->GetElevation();
		Bin = pDoc->GetBin();
		m_XSize = pDoc->GetHeader()->num_samples+64;
		m_YSize = pDoc->GetNumCCDCols()/Bin;
		pHeader = pDoc->GetHeader();
		pDoc->RoundExposure();
		PixelStep = CCD_PIXEL_SIZE *2/ pHeader->mag_factor;
		radius = pDoc->m_Radius;

		double ramp_angle = 2;
		double detector_runon_runoff_angle = (1024*C843::AngularPixelSize)* RADtoDEG; //detector width in px to degrees
		double image_angle = (m_XSize*C843::AngularPixelSize)* RADtoDEG;


		start_angle = 270 + (image_angle/2 + detector_runon_runoff_angle/2 + ramp_angle);
		travel_angle = image_angle + detector_runon_runoff_angle + (2*ramp_angle);
		start_phi = pDoc->GetTheta();// + travel_angle/2.0;

		//GetParent()->PostMessage(WM_SETMESSAGESTRING, "OnScan");



		if (!C843::m_Profile_Valid)
			{
			pDoc->WriteStatus(0,"Creating Profile, this might take a while...");
			C843::CreateProfile(radius,start_phi,start_angle,travel_angle,pDoc->GetHeader()->current);
			CreateTDIBuffers(m_XSize,m_YSize,Bin);
			buffers_init = true;
			
			}

		double currentposition = (pDoc->GetSlide()->GetPosition()/2000);
		//if (currentposition > C843::Source_X)
			direction = 1;
		//elsek
		//	direction = -1;


		if (!m_IsRecording)
			{
			CreateBitmap();
			CreateCCDBuffer();

			pDoc->GetSlide()->SetVelocityFraction(1.0);
			pDoc->GetSlide()->SetAcceleration(100000);
			//decide if we go cloclwise or anti clockwise to start - depends where we currently are.

			//uncomment thos when we get dual-direction scanning going. Need to check comment status in DoProfile in C843 as well

			//if (direction == 1)	//which side of the centre line are we?
			//	{
				pDoc->SetTranslation((int) C843::acwProfileStartX*2000);
				pDoc->m_Header.z = m_VerticalPosition;
				pDoc->m_Header.distance = (float) radius/10;
				//pDoc->SetTheta(C843::cwProfileStartPhi);
				pDoc->SetAzimuth(C843::CalcAzimuth(radius,C843::cwProfileStartX));
				pDoc->WriteStatus(0,"Moving to start");
				C843::MoveToStart(C843::cwProfileStartX,C843::cwProfileStartY,C843::cwProfileStartPhi,m_VerticalPosition);

			//	}
			//else
			//	{
			//	pDoc->SetTranslation((int) C843::acwProfileStartX*2000);
			//	pDoc->m_Header.z = m_VerticalPosition;
			//	pDoc->m_Header.distance = (float) radius/10;
			//	//pDoc->SetTheta(C843::acwProfileStartPhi);
			//	pDoc->SetAzimuth(C843::CalcAzimuth(radius,C843::acwProfileStartX));
			//	pDoc->WriteStatus(0,"Moving to start");
			//	C843::MoveToStart(C843::acwProfileStartX,C843::acwProfileStartY,C843::acwProfileStartPhi,m_VerticalPosition);

			//	}

			}
		/*if (direction == 1)
			{*/
			C843::MoveToStart(C843::cwProfileStartX,C843::cwProfileStartY,C843::cwProfileStartPhi,m_VerticalPosition);
			C843::UnsetBreakPoints();
			C843::SetTriggerEvent(0,0);
			C843::SetTriggerPoint(0,(C843::cwProfileStartX*2000)-(2000 * radius * 0.0174551*ramp_angle)); //0.0174551 = tan 1
			/*}
		if (direction == -1)
			{
			C843::UnsetBreakPoints();
			C843::SetTriggerEvent(0,0);
			C843::SetTriggerPoint(0,(C843::acwProfileStartX*2000)+(2000 * radius * 0.0174551*ramp_angle));
			}*/

		if(!buffers_init)
			{
			CreateTDIBuffers(m_XSize,m_YSize,Bin);
			buffers_init = true;
			}



		unsigned int TDIError,y;
		unsigned long Brightness;
		int m_con_rescans = 0;				//consecutive rescans counter
		pDoc->WriteStatus(0,"Running profile");
		if (m_IsRecording)
			NumFrames = pHeader->num_frames;
		else
			NumFrames = NumFrames = pHeader->num_frames; // was fixed at 1
		for (Frame = 0; Frame < NumFrames; ++Frame)
			{
			/*if (direction == 1)
			{*/
			//don't try to move to start while still finishing a move
			pDoc->GetSlide()->WaitForStop();
			pDoc->GetMagSlide()->WaitForStop();
			pDoc->GetTurntable()->WaitForStop();
			C843::MoveToStart(C843::cwProfileStartX,C843::cwProfileStartY,C843::cwProfileStartPhi,m_VerticalPosition);
			pDoc->GetSlide()->WaitForStop();
			//pDoc->GetMagSlide()->WaitForStop();
			C843::UnsetBreakPoints();
			C843::SetTriggerEvent(0,0);
			C843::SetTriggerPoint(0,(C843::cwProfileStartX*2000)-(2000 * radius * 0.0174551*ramp_angle)); //0.0174551 = tan 1
			/*}
		if (direction == -1)
			{
			C843::UnsetBreakPoints();
			C843::SetTriggerEvent(0,0);
			C843::SetTriggerPoint(0,(C843::acwProfileStartX*2000)+(2000 * radius * 0.0174551*ramp_angle));
			}*/

			do
				{
				if(m_IsDark)
					{
					Camera.CloseShutter();
					TDIError = Camera.TDIReadout(m_CCDPixel,\
						m_XSize,\
						m_YSize,\
						Bin,\
						PixelStep,\
						m_IsDark,\
						m_KeepShutterOpen,\
						Frame,\
						NumFrames,\
						pDoc->GetHeader()->issue,\
						direction,\
						pixel_mag_lookup);
					}
				else
					{
					Camera.OpenShutter();
					TDIError = Camera.TDIReadout(m_CCDPixel,\
						m_XSize,\
						m_YSize,\
						Bin,\
						PixelStep,\
						m_IsDark,\
						m_KeepShutterOpen,\
						Frame,\
						NumFrames,\
						pDoc->GetHeader()->issue,\
						direction,\
						pixel_mag_lookup);
					}

				Brightness = 0;
				
				for (y=0; y<m_YSize; ++y)
					Brightness += m_CCDPixel[y][0] +m_CCDPixel[y][m_XSize-1];
				
				Brightness /= (2*m_YSize);
				
				if (m_IsRecording)
					{
					if (Brightness < m_IZero*3/4)
						{
						TDIError = 1;
						pDoc->WriteStatus(2,"Re-scanning %d",++m_NumRescans);
						m_con_rescans++;
						if (m_con_rescans >= 10)
							{::MessageBox(NULL, \
							"More than 10 consecutive rescans have occured! Hit turn Xrays back on and hit OK to continue scan", \
							"Caution", MB_OK|MB_ICONWARNING);}
						}
					}
				else
					m_IZero = Brightness;
				} while (TDIError);
			}
		m_ValidImage = TRUE;
		if (m_AutoScale)
			AutoScale();
		CreateLookup();
		ExecuteLookup();
		if (!m_IsRecording)
			Resize();
		Camera.GetTempPressure(&Temperature, &Pressure);
		pDoc->WriteStatus(3,"T = %5.1f C,  P = %5.2f Torr",Temperature,Pressure);
		if (Camera.GetPepper())
			pDoc->WriteStatus(5,"Pepper %d",Camera.GetPepperCount());
		else
			pDoc->WriteStatus(5,"Pepper Off");
		CTimeSpan Duration = CTime::GetCurrentTime() - StartTime;
		pDoc->WriteStatus(6,"%4d s",Duration.GetTotalSeconds());
		Invalidate();
		pDoc->WriteStatus(0,"Finished!");
		}

	void CMuCatView::OnRecord() 
		{
		
		BOOL Pause = FALSE;
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		CFile DataFile;
		char FileName[256];
		double Theta,ThetaStep;
		int Block,Proj;
		CFinishTime Finish;
		m_AutoScale = TRUE;
		Camera.SetPepper(pDoc->GetHeader()->pepper);
		pDoc->OnEditSpec();
		pDoc->OnParameterDim();
		pDoc->OnEditXray();
		pDoc->OnEditGeometry();
		CPauseDialog aDlg;
		if (aDlg.DoModal() == IDOK)
			Pause = aDlg.m_Pause;
		int NumBlocks = pDoc->GetHeader()->num_blocks;
		int ZShift = (int)(pDoc->GetHeader()->slices_per_block * pDoc->GetHeader()->pixel_size *(-2.0));
		if (aDlg.m_KillZ)
			ZShift = 0;
		int NumProj = pDoc->GetHeader()->num_proj;
		if (NumProj != 1 && !aDlg.m_KillTheta)
			ThetaStep = 360.0/NumProj;
		else
			ThetaStep = 0;
		CFileDialog FD(FALSE, "mcd", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, \
			"MuCat data files (*.mcd)|*.mcd|All files (*.*)|*.*||");
		if (FD.DoModal() != IDOK)
			return;
		CXrayRestartDialog bDlg;
		bDlg.DoModal();
		m_XrayPCShutdown = bDlg.m_Shutdown;
		CTime ScanStartTime;
		CTime GoTime;

		if(bDlg.m_Wait45 && Pause)
			{
			RecordWaitPause(FileName, &DataFile, &FD);

			//clean up tests
			bDlg.m_Wait45 = FALSE;
			Pause = FALSE;
			}
		else
			{
			if (bDlg.m_Wait45)
				{
				GoTime = CTime::GetCurrentTime() + CTimeSpan(0,0,45,0);
				while (CTime::GetCurrentTime() < GoTime)
					{
					pDoc->WriteStatus(2,"Waiting %d",GoTime - CTime::GetCurrentTime());
					}
				}
			ScanStartTime = CTime::GetCurrentTime();
			strcpy(pDoc->GetHeader()->time, (LPCTSTR)ScanStartTime.Format("%c"));
			strcpy(FileName,(LPCTSTR)FD.GetPathName());
			pDoc->SetTitle(FileName);
			GetParent()->UpdateWindow();

			DataFile.Open(FileName,CFile::modeWrite|CFile::modeCreate);
			DataFile.Write(pDoc->GetHeader(),512);

			if (Pause)
				MessageBox("Remove Specimen and Switch X-rays on.","Waiting for user action.",MB_ICONEXCLAMATION);
			else
				m_IsDark = TRUE;

			OnScan();
			GetParent()->GetParent()->UpdateWindow();
			UpdateWindow();
			SaveProjection(DataFile);
			m_IsDark = FALSE;
			m_KeepShutterOpen = true;

			Camera.OpenShutter();
			DataFile.Close();
			if (Pause)
				MessageBox("Replace Specimen and Switch X-rays on.","Waiting for user action.",MB_ICONEXCLAMATION);
			Log.Clear();
			Log.Write("File: %s",(LPCTSTR)FileName);
			}


		m_NumRescans = 0;
		for (Block=0; Block<NumBlocks; ++Block)
			{
			DataFile.Open(FileName,CFile::modeReadWrite);
			DataFile.SeekToEnd();
			Theta = pDoc->GetTheta();
			//blocks doesn't work yet - so don't move height.
			//pDoc->GetElevator()->Move(300000 -(pDoc->GetElevation() + (NumBlocks-1-Block)*ZShift));
			//pDoc->GetElevator()->WaitForStop();
			pDoc->GetTurntable()->SetAcceleration(10000);
			for (Proj=0; Proj < NumProj; ++Proj)
				{
				pDoc->WriteStatus(2,"Projection %d:%d of %d:%d",Block+1,Proj+1,NumBlocks,NumProj);
				Log.Write("Projection: %d:%d/%d",Block+1,Proj+1,m_NumRescans);
				Log.Write("Angle: = %6.2lf",Theta);
				Log.Write("Finish: "+Finish.GetFinishTime(NumProj*Block+Proj,NumProj*NumBlocks));
				pDoc->WriteStatus(8,(LPCTSTR)Finish.GetShortFinishTime(NumProj*Block+Proj,NumProj*NumBlocks));
				

				Theta -= ThetaStep;
				m_IsRecording = TRUE; // ???Update after first projection
				OnScan();
				GetParent()->GetParent()->UpdateWindow();
				UpdateWindow();
				SaveProjection(DataFile);
				Log.Dump("MuCat.mon");
				double currentposition = (pDoc->GetSlide()->GetPosition()/2000);
				
				IncrementRotationProfile(currentposition,ThetaStep);
				
				}
			DataFile.Close();
			}

			//if numblocks = 0 and cal issue = 0 too, then we have landed here without recording anything but
		// dark and I0 scans so far.

	if (pDoc->GetHeader()->issue == 1)
		{
		Log.Write("Calibrating");
		int cal_height;
		int cal_posdistance;
		float cal_angle;
	
		//get the calibration positions
		FILE *CalibrationFile;
		if ((CalibrationFile = fopen("calibration.txt","r")) == NULL)
			{
			Log.Write("Cannot open calibration file calibration.txt, exiting");
			Texit("Cannot open calibration file 'calibration.txt'");
			}
		fscanf(CalibrationFile, "%d %f %d",&cal_height, &cal_angle, &cal_posdistance);
		fclose(CalibrationFile);
	
		//save the current X and elevation positions
		short oldheaderX = pDoc->GetHeader()->num_samples;
	//	long oldheaderZ = pDoc->GetHeader()->z;
	
		//move stages to safe position
		pDoc->GetElevator()->Move(C843::mmToElevation(280));
		pDoc->GetElevator()->WaitForStop();
		pDoc->GetMagSlide()->FindHome();


		//set the new parameters up
		pDoc->GetHeader()->num_samples = 600;
		pDoc->GetTurntable()->SetAcceleration(10000);
		pDoc->GetElevator()->Move(C843::mmToElevation(cal_height));
		pDoc->GetElevator()->WaitForStop();
		pDoc->GetMagSlide()->Move(cal_posdistance*(2));
		pDoc->GetMagSlide()->WaitForStop();
		pDoc->GetTurntable()->Move(-cal_angle*1000);

		//record the projections
		DataFile.Open(FileName,CFile::modeReadWrite);
		DataFile.SeekToEnd();
		Theta = cal_angle;
		ThetaStep = 360.0/9;

		for (Proj=0; Proj < 9; ++Proj)
			{
			pDoc->WriteStatus(2,"Projection %d",Proj+1);
			Log.Write("Calibrating: %d",Proj+1);
			pDoc->GetTurntable()->Move(-(long)(Theta*1000.0));
			pDoc->GetTurntable()->SetAcceleration(100000);
			Theta -= ThetaStep;
			pDoc->GetTurntable()->WaitForStop();
			m_IsRecording = TRUE; 
			OnScan();
			GetParent()->GetParent()->UpdateWindow();
			UpdateWindow();
			SaveProjection(DataFile);
			Log.Dump("MuCat.mon");
			}
		DataFile.Close();
		pDoc->GetHeader()->num_samples = oldheaderX;
		
		//move stages to safe position
		pDoc->GetElevator()->Move(C843::mmToElevation(280));
		pDoc->GetElevator()->WaitForStop();
		pDoc->GetMagSlide()->FindHome();
	}

		pDoc->GetTurntable()->SetAcceleration(750000);
		m_IsRecording = FALSE;
		m_KeepShutterOpen = false;
		Camera.CloseShutter();
		m_ShutterOpen = 0;
		pDoc->GetTurntable()->Move(-(long)(pDoc->GetTheta()*1000.0));
		Log.Write("File: %s",(LPCTSTR)FileName);
		Log.Write("FINISHED!!!!");
		Log.Dump("MuCat.mon");
		CTimeSpan Duration = CTime::GetCurrentTime() - ScanStartTime;
		strcpy(pDoc->GetHeader()->duration, (LPCTSTR)Duration.Format("%D:%H:%M:%S"));
		DataFile.Open(FileName,CFile::modeReadWrite);
		DataFile.Write(pDoc->GetHeader(),512);
		DataFile.Close();
		pDoc->WriteStatus(2,"Finished!  %d rescans",m_NumRescans);
	
		}

		void CMuCatView::RecordWaitPause(char *FileName,CFile *DataFile, CFileDialog *FD)
			{
			CTime GoTime;
			CMuCatDoc* pDoc = GetDocument();
			ASSERT_VALID(pDoc);

			strcpy(FileName,(LPCTSTR)FD->GetPathName());
			pDoc->SetTitle(FileName);
			GetParent()->UpdateWindow();
			//	CFile DataFile(FileName,CFile::modeWrite|CFile::modeCreate);
			DataFile->Open(FileName,CFile::modeWrite|CFile::modeCreate);
			DataFile->Write(pDoc->GetHeader(),512);

			MessageBox("Remove Specimen and Switch X-rays on.","Waiting for user action.",MB_ICONEXCLAMATION);
			m_IsDark = TRUE;
			OnScan();
			GetParent()->GetParent()->UpdateWindow();
			UpdateWindow();
			SaveProjection(*DataFile);
			pDoc->GetElevator()->WaitForStop();
			m_IsDark = FALSE;
			m_KeepShutterOpen = true;

			DataFile->Close();
			MessageBox("Replace Specimen and Switch X-rays on.","Waiting for user action.",MB_ICONEXCLAMATION);
			Log.Clear();
			Log.Write("File: %s",(LPCTSTR)FileName);

			//done dark  scan by now, now wait 45 min

			MessageBox("Waiting for 45 min","Waiting for you to hit OK.",MB_ICONEXCLAMATION);
			GoTime = CTime::GetCurrentTime() + CTimeSpan(0,0,45,0);
			while (CTime::GetCurrentTime() < GoTime)
				{
				pDoc->WriteStatus(2,"Waiting %d",GoTime - CTime::GetCurrentTime());
				}

			}


		void CMuCatView::IncrementRotationProfile(double currentposition, double ThetaStep)
			{
			double oldpos,newpos;
				string temp,fixedstring;
				vector<string> tokens;
				stringstream tss;

				//if (currentposition > C843::Source_X)
				//	{
					for (unsigned int index = 5; index <C843::cwPhilist.size()-1;index++)	//start at 5 to skip the bits we do not need to modify here
						{
						stringstream ss(C843::cwPhilist[index]);
						temp.clear();
						tokens.clear();
						fixedstring.clear();
						while (ss>>temp)			
							{
							tokens.push_back(temp);
							}

						istringstream ( tokens[5] ) >> oldpos;
						newpos = oldpos - ThetaStep;
						if (index == 5) C843::cwProfileStartPhi = newpos;
						temp.clear();
						temp = ConvertNumberToString<double>(newpos);
						tokens.at(5) = temp;
						for (unsigned int i = 0; i<tokens.size(); ++i)
							{
							fixedstring +=tokens[i];
							fixedstring +=" ";
							}
						C843::cwPhilist.at(index) = fixedstring;

						/*float tmp;
						ostringstream s;
						int offset;
						if(index <15)
							{ offset = 19;}
						else
							{ offset = 20;}

						tmp = strtod(C843::cwPhilist[index].substr(offset,10).c_str(),NULL);
						tmp -=ThetaStep;
						Theta = tmp;
						s<<tmp;
						C843::cwPhilist[index].replace(offset+1,6,s.str().substr(1,9));*/

						}
					//}
				//else
				//	{
				//	for (unsigned int index = 5; index <C843::acwPhilist.size()-1;index++)	//start at 5 to skip the bits we do not need to modify here
				//		{

				//		stringstream ss(C843::acwPhilist[index]);
				//		temp.clear();
				//		tokens.clear();
				//		fixedstring.clear();
				//		while (ss>>temp)			
				//			{
				//			tokens.push_back(temp);
				//			}

				//		istringstream ( tokens[5] ) >> oldpos;
				//		
				//		newpos = oldpos - ThetaStep; //was - with no effect
				//		//TRACE("Old Position: %d   New Position: %d  Step: %d",oldpos,newpos,ThetaStep);
				//		if (index == 5) C843::acwProfileStartPhi = newpos;
				//		temp.clear();
				//		temp = ConvertNumberToString<double>(newpos);
				//		tokens.at(5) = temp;
				//		for (unsigned int i = 0; i<tokens.size(); ++i)
				//			{
				//			fixedstring +=tokens[i];
				//			fixedstring +=" ";
				//			}
				//		C843::acwPhilist.at(index) = fixedstring;

				//		/*float tmp;
				//		ostringstream s;
				//		int offset;
				//		if(index <15)
				//			{ offset = 19;}
				//		else
				//			{ offset = 20;}

				//		tmp = strtod(C843::acwPhilist[index].substr(offset,10).c_str(),NULL);
				//		tmp -=ThetaStep;
				//		Theta = tmp;
				//		s<<tmp;
				//		C843::acwPhilist[index].replace(offset+1,6,s.str().substr(1,9));*/

				//		}
				//	//}

				////			DataFile.Flush();

			}


	void CMuCatView::OnMouseMove(UINT nFlags, CPoint point) 

		{
		// TODO: Add your message handler code here and/or call default
		int XPos, YPos;
		CMuCatDoc* pDoc = GetDocument();
		CClientDC aDC(this);
		OnPrepareDC(&aDC);
		aDC.DPtoLP(&point);
		XPos = point.x*12/m_Zoom - m_XSize/2;
		YPos = m_YSize/2 - point.y*12/m_Zoom;
		if (m_mmUnits)
			{
			float X = (float)(XPos*pDoc->GetHeader()->pixel_size/1000.0);
			float Y = (float)(-YPos*pDoc->GetHeader()->pixel_size/1000.0 + m_VerticalPosition/1000.0);
			pDoc->WriteStatus(4,"X=%7.3f Y=%7.3f",(float)X,Y);
			}
		else
			{
			pDoc->WriteStatus(4,"X=%d Y=%d",XPos,(int)(-YPos));
			}
		CScrollView::OnMouseMove(nFlags, point);
		}

	void CMuCatView::SaveProjection(CFile &DataFile)
		{
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		unsigned int x,y;
		unsigned long *pLine;
		unsigned int Height = pDoc->GetHeader()->num_slices;
		for (y=0; y<Height; ++y)
			{
			pLine = m_CCDPixel[m_YSize/2-Height/2 +y];
			for (x=0; x<m_XSize; ++x)
				m_Line[x] = (unsigned short)pLine[x];
			DataFile.Write(m_Line,m_XSize*2);
			}
		}

	void CMuCatView::OnLButtonDblClk(UINT nFlags, CPoint point) 
		{
		// TODO: Add your message handler code here and/or call default
		CScrollView::OnLButtonDblClk(nFlags, point);
		m_mmUnits = !m_mmUnits;
		}


	void CMuCatView::OnRButonDown(UINT nFlags, CPoint point)
		{
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);

		float Min,Max,Mid,Sharpness,Centre,Width,Value,Left60,Left50,Left40,Right40,Right50,Right60;
		unsigned int x,y;
		unsigned int IMin,LeftMid,RightMid,LeftMin,RightMin;//, NumSlices;
		unsigned long *Pixel;
		Sharpness = Centre = Width = 0;

		sharp_x = x = point.x;
		sharp_y = y = point.y;

		Pixel = m_CCDPixel[y];
			Max = 0;
			for (x=2; x<32; ++x)
				Max += Pixel[x] + Pixel[m_XSize-1-x];
			Max /= 60;
			IMin = 65535;
			for (x=0; x<m_XSize; ++x)
				{
				if (Pixel[x] < IMin)
					IMin = Pixel[x];
				}
			Mid = (IMin + Max)/2;
			for (LeftMid = 0; LeftMid < m_XSize; ++LeftMid)
				{
				if (Pixel[LeftMid] < Mid)
					break;
				}
			for (RightMid = m_XSize-1; RightMid > LeftMid; --RightMid)
				{
				if (Pixel[RightMid] < Mid)
					break;
				}
			LeftMin = (LeftMid*3 + RightMid)/4;
			RightMin = (LeftMid + RightMid*3)/4;
			Min = 0;
			for (x = LeftMid; x<= RightMid; ++x)
				Min += Pixel[x];
			Min /= (RightMid - LeftMid + 1);

			Value = (Max*3 + Min*2) / 5;
			x=0;
			while (Pixel[x] > Value)
				++x;
			Left60 = ((Value - Pixel[x])*(x - 1) + (Pixel[x-1] - Value)*x) / (Pixel[x-1] - Pixel[x]);

			Value = (Max + Min) / 2;
			x=0;
			while (Pixel[x] > Value)
				++x;
			Left50 = ((Value - Pixel[x])*(x - 1) + (Pixel[x-1] - Value)*x) / (Pixel[x-1] - Pixel[x]);

			Value = (Max*2 + Min*3) / 5;
			while (Pixel[x] > Value)
				++x;
			Left40 = ((Value - Pixel[x])*(x - 1) + (Pixel[x-1] - Value)*x) / (Pixel[x-1] - Pixel[x]);

			x = (LeftMid + RightMid)/2;

			while (Pixel[x] < Value)
				++x;
			Right40 = ((Pixel[x] - Value)*(x-1) + (Value - Pixel[x-1])*x) / (Pixel[x] - Pixel[x-1]);

			Value = (Max + Min) / 2;
			while (Pixel[x] < Value)
				++x;
			Right50 = ((Pixel[x] - Value)*(x-1) + (Value - Pixel[x-1])*x) / (Pixel[x] - Pixel[x-1]);

			Value = (Max*3 + Min*2) / 5;
			while (Pixel[x] < Value)
				++x;
			Right60 = ((Pixel[x] - Value)*(x-1) + (Value - Pixel[x-1])*x) / (Pixel[x] - Pixel[x-1]);

			Sharpness += Right60 - Right40 + Left40 - Left60;
			Centre += Left50 + Right50;
			Width += Right50 - Left50;
			
		Sharpness /= (1*2);
		Centre /= (1*2);
		Centre -= (m_XSize - 1)/2.0f;
		Width /= 1;
		m_MeasuredCentre = Centre;
		m_MeasuredWidth = Width;
		pDoc->WriteStatus(9, "Sharp = %6.2f,   Centre = %6.1f",Sharpness, Centre);
		m_Are_doing_sharpness = true;
		Invalidate();
		UpdateWindow();

		}




	void CMuCatView::OnButtonsharpness() 
		{

		// TODO: Add your command handler code here
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		float Min,Max,Mid,Centre,Width,Value,Left60,Left50,Left40,Right40,Right50,Right60;
		int x,y,IMin,LeftMid,RightMid,LeftMin,RightMin, NumSlices;
		unsigned long *Pixel;
		Sharpness = Centre = Width = 0;
		NumSlices = GetDocument()->GetHeader()->slices_per_block;
		if (NumSlices > (m_YSize-8))
			NumSlices = m_YSize-8;
		for (y=(m_YSize - NumSlices)/2; y<(m_YSize + NumSlices)/2; ++y)
			{
			Pixel = m_CCDPixel[y];
			Max = 0;
			for (x=2; x<32; ++x)
				Max += Pixel[x] + Pixel[m_XSize-1-x];
			Max /= 60;
			IMin = 65535;
			for (x=0; x<m_XSize; ++x)
				{
				if (Pixel[x] < IMin)
					IMin = Pixel[x];
				}
			Mid = (IMin + Max)/2;
			for (LeftMid = 0; LeftMid < m_XSize; ++LeftMid)
				{
				if (Pixel[LeftMid] < Mid)
					break;
				}
			for (RightMid = m_XSize-1; RightMid > LeftMid; --RightMid)
				{
				if (Pixel[RightMid] < Mid)
					break;
				}
			LeftMin = (LeftMid*3 + RightMid)/4;
			RightMin = (LeftMid + RightMid*3)/4;
			Min = 0;
			for (x = LeftMid; x<= RightMid; ++x)
				Min += Pixel[x];
			Min /= (RightMid - LeftMid + 1);

			Value = (Max*3 + Min*2) / 5;
			x=0;
			while (Pixel[x] > Value)
				++x;
			Left60 = ((Value - Pixel[x])*(x - 1) + (Pixel[x-1] - Value)*x) / (Pixel[x-1] - Pixel[x]);

			Value = (Max + Min) / 2;
			x=0;
			while (Pixel[x] > Value)
				++x;
			Left50 = ((Value - Pixel[x])*(x - 1) + (Pixel[x-1] - Value)*x) / (Pixel[x-1] - Pixel[x]);

			Value = (Max*2 + Min*3) / 5;
			while (Pixel[x] > Value)
				++x;
			Left40 = ((Value - Pixel[x])*(x - 1) + (Pixel[x-1] - Value)*x) / (Pixel[x-1] - Pixel[x]);

			x = (LeftMid + RightMid)/2;

			while (Pixel[x] < Value)
				++x;
			Right40 = ((Pixel[x] - Value)*(x-1) + (Value - Pixel[x-1])*x) / (Pixel[x] - Pixel[x-1]);

			Value = (Max + Min) / 2;
			while (Pixel[x] < Value)
				++x;
			Right50 = ((Pixel[x] - Value)*(x-1) + (Value - Pixel[x-1])*x) / (Pixel[x] - Pixel[x-1]);

			Value = (Max*3 + Min*2) / 5;
			while (Pixel[x] < Value)
				++x;                                  
			Right60 = ((Pixel[x] - Value)*(x-1) + (Value - Pixel[x-1])*x) / (Pixel[x] - Pixel[x-1]);

			Sharpness += Right60 - Right40 + Left40 - Left60;
			Centre += Left50 + Right50;
			Width += Right50 - Left50;
			}
		Sharpness /= (NumSlices*2);
		Centre /= (NumSlices*2);
		Centre -= (m_XSize - 1)/2.0f;
		Width /= NumSlices;
		m_MeasuredCentre = Centre;
		m_MeasuredWidth = Width;
		pDoc->WriteStatus(9, "Sharp = %6.2f,   Centre = %6.1f",Sharpness, Centre);
		m_Are_doing_sharpness = true;
		Invalidate();
		UpdateWindow();

		//int Y, X;
		//int Height = pDoc->GetHeader()->num_slices;
		//	Y = (m_YSize/2)+ (Height/4)*m_Zoom/12-1;
		//	X = (m_XSize*m_Zoom/12)-1;
		//	CPen DashPen;
		//	DashPen.CreatePen(PS_DASHDOT, 1, RGB(128,128,0));
		//	pDC->SelectObject(&DashPen);
		//	pDC->SetBkMode(TRANSPARENT);
		//	pDC->MoveTo(0,Y);
		//	pDC->LineTo(X,Y);
		//	m_Are_doing_sharpness = false;


		}





	void CMuCatView::OnButtonCentre(void)
		{
		CMuCatDoc* pDoc = GetDocument();
		CString Message;
		ASSERT_VALID(pDoc);
		float Centre, Width;
		OnScan();
		OnButtonsharpness();
		Centre = m_MeasuredCentre;
		Width = m_MeasuredWidth;
		pDoc->GetTurntable()->Move((long)((pDoc->GetTheta()+180.0)*(-1000)));
		pDoc->GetTurntable()->WaitForStop();
		OnScan();
		OnButtonsharpness();
		Centre += m_MeasuredCentre;
		Width += m_MeasuredWidth;
		pDoc->GetTurntable()->Move((long)((pDoc->GetTheta())*(-1000)));
		Centre /= 2.0;
		Width /= 2.0;
		Message.Format("Absolute Centre = %7.2f\nAbsolute Width = %8.2f",Centre, Width);
		::MessageBox(NULL,Message, "Centre",MB_OK|MB_ICONINFORMATION);
		//	pDoc->WriteStatus(9,"Absolute Centre = %7.2f",Centre);
		}


	void CMuCatView::OnUpdateShutter(CCmdUI* pCmdUI)
		{
		pCmdUI->SetCheck(m_ShutterOpen);
		}

	void CMuCatView::OnShutter(void)
		{
		if (m_ShutterOpen)
			Camera.CloseShutter();
		else
			Camera.OpenShutter();
		m_ShutterOpen ^= 1;
		}


	void CMuCatView::OnLocateCamSource()
		{
#define NUMBEROFLINES 27	//numbers of lines in the raster file, one full position per line
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);

		pDoc->GetTurntable()->SetVelocityFraction(1.0);
		float x,y,phi,junk1,junk2;
		FILE *RasterScanFile;	//file format is X Y H Phi, all float values.
		CFile DataFile;
		if ((RasterScanFile = fopen("ScanForSource.txt","r")) == NULL)
			Texit("Cannot open raster file 'ScanForSource.txt'");

		char FileName[256];
		CFileDialog FD(FALSE, "mcd", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, \
			"MuCat data files (*.mcd)|*.mcd|All files (*.*)|*.*||");
		if (FD.DoModal() != IDOK)
			return;
		strcpy(FileName,(LPCTSTR)FD.GetPathName());
		pDoc->SetTitle(FileName);
		GetParent()->UpdateWindow();
		DataFile.Open(FileName,CFile::modeWrite|CFile::modeCreate);
		DataFile.Write(pDoc->GetHeader(),512);
		m_IsDark = FALSE;
		m_KeepShutterOpen = true;

		m_AutoScale = TRUE;
		Camera.SetPepper(pDoc->GetHeader()->pepper);
		pDoc->OnEditSpec();
		pDoc->OnEditXray();
		pDoc->m_Header.num_slices = 800;
		pDoc->WriteStatus(0,"Collecting Dark Frames");
		m_IsDark = TRUE;
		OnStill();
		GetParent()->GetParent()->UpdateWindow();
		UpdateWindow();
		
		OnStill();
		GetParent()->GetParent()->UpdateWindow();
		UpdateWindow();

		OnStill();
		GetParent()->GetParent()->UpdateWindow();
		UpdateWindow();
		SaveProjection(DataFile);
		pDoc->WriteStatus(0,"Collecting Light Frame");
		//MessageBox("Turn XRays on","Waiting for user action.",MB_ICONEXCLAMATION);
		m_IsDark = FALSE;
		pDoc->GetElevator()->Move(280*2000);	//move stage down out of the way
		pDoc->GetElevator()->WaitForStop();
		OnStill();
		GetParent()->GetParent()->UpdateWindow();
		UpdateWindow();
		SaveProjection(DataFile);
		pDoc->GetElevator()->Move(229*2000);	//move stage back up
		pDoc->GetElevator()->WaitForStop();

		//process the sourcepos file
		char junk_first_line [1000];
		fgets(junk_first_line,999,RasterScanFile);	//swallow the first line from the file


		for (int linecounter=0; linecounter<NUMBEROFLINES; ++linecounter)
			{
			pDoc->WriteStatus(9, "Exposure %d of %d",linecounter, NUMBEROFLINES);
			fscanf(RasterScanFile,"%f %f %f %f %f",&y,&x,&phi,&junk1,&junk2);


			/*pDoc->GetElevator()->Move(h);
			pDoc->GetElevator()->WaitForStop();*/
			pDoc->WriteStatus(0,"Moving Azimuth, %d   %d   %d",x,y,phi);
			pDoc->GetSlide()->Move((long)(x*2000.0));
			pDoc->GetSlide()->WaitForStop();
			
			pDoc->WriteStatus(0,"Moving Magnification, %d   %d   %d",x,y,phi);
			pDoc->GetMagSlide()->Move((long)(y*2000.0));
			pDoc->GetMagSlide()->WaitForStop();

			pDoc->WriteStatus(0,"Rotating Rod, %d   %d   %d",x,y,phi);
			pDoc->GetTurntable()->Move((long)(phi*1000.0));
			pDoc->GetTurntable()->WaitForStop();

			pDoc->WriteStatus(0,"Capturing Image, %d   %d   %d",x,y,phi);
			m_IsDark = FALSE;
			OnStill();
			GetParent()->GetParent()->UpdateWindow();
			UpdateWindow();
			SaveProjection(DataFile);

			}

		if (fclose(RasterScanFile)!=0)
			{TRACE("Error Closing raster file");
			}

		}


	void CMuCatView::GetSourceCameraDistance(void)
		{
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		m_SourceToCamera = pDoc->m_SourceToCamera;

		/*FILE *GeoFile;
		if ((GeoFile = fopen("geometry.txt","r")) == NULL)
		Texit("Cannot open geometry file 'geometry.txt'");
		fscanf(GeoFile, "%f",&m_SourceToCamera);
		fclose(GeoFile);*/
		}


	vector<float> * CMuCatView::Get_pixel_mag_lookup(void)
		{
		return  &pixel_mag_lookup;
		}


	void CMuCatView::CreatePixelMagLookup(void)
		{
		if (!m_valid_pix_mag_lookup)
			{
		double R;
		float tmp, xc,yc;
		GetSourceCameraDistance();
		pixel_mag_lookup.clear();


		yc = 0;
		for(int ycount=m_YSize/2;ycount >0;ycount--)
			{
			calc_pixel_size_quadrant.clear();

			//calc_pixel_size_quadrant.resize((m_XSize/2));
			calc_pixel_size_quadrant.resize((8000/2));
			//yc = ycount;// - 0.5;
			//TRACE("calc_pixel_size_quadrant.size() = %d\n",calc_pixel_size_quadrant.size());
			//for (int xcount=(m_XSize/2); xcount >0; xcount--)
			//for (int xcount=0; xcount <(m_XSize/2); xcount++)
			for (int xcount=0; xcount <(8000/2); xcount++)
				{
				xc = (float) xcount - 0.5;
				//TRACE("Xcount = %d\n",xcount);
				R=sqrt((double)(yc*yc + xc*xc));
				tmp = (float) atan((double)(R*60.0)/((int)(m_SourceToCamera*10000)))*(int)(m_SourceToCamera*10000)/(60*R);
				//tmp = atan((double)R*60.0/250000.0)*250000.0/(60*R);
				calc_pixel_size_quadrant[xcount] = tmp;

				}

			//calc_pixel_size_quadrant.resize((m_XSize/2));	//drop the blank pixel
			tophalf.insert(tophalf.end(),calc_pixel_size_quadrant.rbegin(),calc_pixel_size_quadrant.rend());	//first quadrant
			tophalf.insert(tophalf.end(),calc_pixel_size_quadrant.begin(),calc_pixel_size_quadrant.end());		//second quadrant
			}
		pixel_mag_lookup.insert(pixel_mag_lookup.end(),tophalf.begin(),tophalf.end());		//tophalf
		pixel_mag_lookup.insert(pixel_mag_lookup.end(),tophalf.rbegin(),tophalf.rend());	//bottomhalf


		// END TODO

		//&&&&&&&&&&&&&&&&&&&Temp Debugging stuff&&&&&&&&&&&&&&&&&&&&&&&&&&	
		/*fstream file ("pixel_look_up.bin",ios::in|ios::out|ios::binary|ios::trunc);
		if (file.is_open())
			file.write ((char*)&*(pixel_mag_lookup.begin()),pixel_mag_lookup.size() * sizeof(float));
		file.close();*/
			}
		m_valid_pix_mag_lookup = true;
		}


	void CMuCatView::OnButtonHome(void)
		{
		CMuCatDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		::MessageBox(NULL, "Remove specimen and holder!\n\nCheck slide paths are clear!", "Caution", MB_OK|MB_ICONWARNING);
		pDoc->GetSlide()->FindHome();

		//update positions
		pDoc->SetTranslation(153*2000); 
		pDoc->SetPosDistance((int) (153*1000) - C843::Source_Y);
		pDoc->SetTheta(0);
		pDoc->SetElevation(153*2000);
		}


	void CMuCatView::CreateTDIBuffers(int X, int y, int bin)
		{
		if (TDIArray){
			delete  TDIArray;
			delete  TDIBuffer;
			}

		TDIBuffer = new unsigned short[(X * bin + 4) * y * bin];

		TDIArray  = new unsigned short*[(X * bin + 4)];
		for (int a=0; a<(X * bin + 4); ++a)
			TDIArray[a] = &TDIBuffer[a*y*bin];
		}



	



//	double TDISharp(VecDoub_I &z)
//{
//		
//	//CMuCatView::OnStill();
//	//OnButtonsharpness();
//	return (double)Sharpness;
//}
	



	// uses Down Hill Simplex method (Amoeba) to minimize sharpness value (maximize sharpness)
	//void CMuCatView::OnAutoFocus(void)
	//	{

	//	const double FTOL = 0.01;	//fractional convergence error
	//	const int np = 2;			//number of variable to solve for
	//	const int mp = np+1;			//number of dimensions to solve in; Number of variables+1
	//	

	//	//these are the first guesses - get these aproximately correct, else we'll have to do a lot of TDI scans
	//	double p_array[mp*np] = {
	//		98.2, 7002,
	//		37.6, 0.0,
	//		3.1415926535, 112
	//		};
	//	
	//	MatDoub p(mp, np, p_array);
	//	Amoeba a(FTOL);
	//	a.minimize(p, TDISharp);

	//	}


	template <typename T>
	void dump(ostream& out, const valarray<T>& theValArray)
		{
		for (int i = 0; i < theValArray.size(); i++)
			out << theValArray[i];
		}

	
