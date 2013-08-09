//
// MATLAB Compiler: 4.14 (R2010b)
// Date: Fri Sep 21 13:32:05 2012
// Arguments: "-B" "macro_default" "-W" "cpplib:XYPhiMove" "-T" "link:lib" "-d"
// "D:\scratch\PhiMove\XYPhiMove\src" "-w" "enable:specified_file_mismatch"
// "-w" "enable:repeated_file" "-w" "enable:switch_ignored" "-w"
// "enable:missing_lib_sentinel" "-w" "enable:demo_license" "-v"
// "D:\scratch\PhiMove\GenerateXYPhiMotion.m" 
//

#include <stdio.h>
#define EXPORTING_XYPhiMove 1
#include "XYPhiMove.h"

static HMCRINSTANCE _mcr_inst = NULL;


#if defined( _MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__) || defined(__LCC__)
#ifdef __LCC__
#undef EXTERN_C
#endif
#include <windows.h>

static char path_to_dll[_MAX_PATH];

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, void *pv)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (GetModuleFileName(hInstance, path_to_dll, _MAX_PATH) == 0)
            return FALSE;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }
    return TRUE;
}
#endif
#ifdef __cplusplus
extern "C" {
#endif

static int mclDefaultPrintHandler(const char *s)
{
  return mclWrite(1 /* stdout */, s, sizeof(char)*strlen(s));
}

#ifdef __cplusplus
} /* End extern "C" block */
#endif

#ifdef __cplusplus
extern "C" {
#endif

static int mclDefaultErrorHandler(const char *s)
{
  int written = 0;
  size_t len = 0;
  len = strlen(s);
  written = mclWrite(2 /* stderr */, s, sizeof(char)*len);
  if (len > 0 && s[ len-1 ] != '\n')
    written += mclWrite(2 /* stderr */, "\n", sizeof(char));
  return written;
}

#ifdef __cplusplus
} /* End extern "C" block */
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_XYPhiMove_C_API
#define LIB_XYPhiMove_C_API /* No special import/export declaration */
#endif

LIB_XYPhiMove_C_API 
bool MW_CALL_CONV XYPhiMoveInitializeWithHandlers(
    mclOutputHandlerFcn error_handler,
    mclOutputHandlerFcn print_handler)
{
    int bResult = 0;
  if (_mcr_inst != NULL)
    return true;
  if (!mclmcrInitialize())
    return false;
  if (!GetModuleFileName(GetModuleHandle("XYPhiMove"), path_to_dll, _MAX_PATH))
    return false;
    {
        mclCtfStream ctfStream = 
            mclGetEmbeddedCtfStream(path_to_dll, 
                                    97127);
        if (ctfStream) {
            bResult = mclInitializeComponentInstanceEmbedded(   &_mcr_inst,
                                                                error_handler, 
                                                                print_handler,
                                                                ctfStream, 
                                                                97127);
            mclDestroyStream(ctfStream);
        } else {
            bResult = 0;
        }
    }  
    if (!bResult)
    return false;
  return true;
}

LIB_XYPhiMove_C_API 
bool MW_CALL_CONV XYPhiMoveInitialize(void)
{
  return XYPhiMoveInitializeWithHandlers(mclDefaultErrorHandler, mclDefaultPrintHandler);
}

LIB_XYPhiMove_C_API 
void MW_CALL_CONV XYPhiMoveTerminate(void)
{
  if (_mcr_inst != NULL)
    mclTerminateInstance(&_mcr_inst);
}

LIB_XYPhiMove_C_API 
long MW_CALL_CONV XYPhiMoveGetMcrID() 
{
  return mclGetID(_mcr_inst);
}

LIB_XYPhiMove_C_API 
void MW_CALL_CONV XYPhiMovePrintStackTrace(void) 
{
  char** stackTrace;
  int stackDepth = mclGetStackTrace(&stackTrace);
  int i;
  for(i=0; i<stackDepth; i++)
  {
    mclWrite(2 /* stderr */, stackTrace[i], sizeof(char)*strlen(stackTrace[i]));
    mclWrite(2 /* stderr */, "\n", sizeof(char)*strlen("\n"));
  }
  mclFreeStackTrace(&stackTrace, stackDepth);
}


LIB_XYPhiMove_C_API 
bool MW_CALL_CONV mlxGenerateXYPhiMotion(int nlhs, mxArray *plhs[], int nrhs, mxArray 
                                         *prhs[])
{
  return mclFeval(_mcr_inst, "GenerateXYPhiMotion", nlhs, plhs, nrhs, prhs);
}

LIB_XYPhiMove_CPP_API 
void MW_CALL_CONV GenerateXYPhiMotion(const mwArray& omega_xy, const mwArray& 
                                      omega_acc_xy, const mwArray& phi_fac, const 
                                      mwArray& startangle_xy, const mwArray& 
                                      startangle_phi, const mwArray& stopangle_xy, const 
                                      mwArray& center_xy, const mwArray& radius_xy, const 
                                      mwArray& moveback, const mwArray& dwelltime)
{
  mclcppMlfFeval(_mcr_inst, "GenerateXYPhiMotion", 0, 0, 10, &omega_xy, &omega_acc_xy, &phi_fac, &startangle_xy, &startangle_phi, &stopangle_xy, &center_xy, &radius_xy, &moveback, &dwelltime);
}
