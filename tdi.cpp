#include <atlinkc.pkg>
#include <42serv.h>
#include <stdafx.h>

void TDIReadout(unsigned short * * image, int XSize, int YSize, int Bin)
{
	long a,b,NumRows,Status,BytesRead;
	LPSERVER pserver;
	unsigned short *TDIBuffer, **TDIArray;
	unsigned short *Row;

	NumRows = XSize * Bin;
	pserver=server_lock(m_Server);
	if (Bin != 1)
		return;

	TDIBuffer = new unsigned short[NumRows * YSize];
	TDIArray  = new unsigned short*[NumRows];
	for (a=0; a<XSize * Bin; ++a)
		TDIArray[a] = &TDIBuffer[a*YSize];

	cs42_set_ccd_size(pserver->cs, YSize, 1);//  Set CCD size to 1 row only
	cs42_set_region(pserver->cs,0,0,YSize,1);//  Set region to 1 row
	cs42_set_ccd_read(pserver->cs,NumRows,0);// Set to read appropriate number of rows
	
	for (a=0; a<NumRows; ++a)
		atlink_testread((ATLINK_HANDLE)pserver->comm_device,TDIArray[a],YSize,1,30,\
			&Status,&BytesRead,DF9_IDF,pserver->adc.const1,pserver->adc.const2,\
			pserver->adc.const3);

	for (a=0; a<XSize; ++a)
	{
		for (b=0; b<YSize; ++b)
			image[b][a] = TDIArray[a][b];
	}
	server_unlock(m_Server);
}

