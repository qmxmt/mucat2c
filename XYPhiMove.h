//
// MATLAB Compiler: 4.14 (R2010b)
// Date: Fri Sep 21 13:32:05 2012
// Arguments: "-B" "macro_default" "-W" "cpplib:XYPhiMove" "-T" "link:lib" "-d"
// "D:\scratch\PhiMove\XYPhiMove\src" "-w" "enable:specified_file_mismatch"
// "-w" "enable:repeated_file" "-w" "enable:switch_ignored" "-w"
// "enable:missing_lib_sentinel" "-w" "enable:demo_license" "-v"
// "D:\scratch\PhiMove\GenerateXYPhiMotion.m" 
//

#ifndef __XYPhiMove_h
#define __XYPhiMove_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_XYPhiMove
#define PUBLIC_XYPhiMove_C_API __global
#else
#define PUBLIC_XYPhiMove_C_API /* No import statement needed. */
#endif

#define LIB_XYPhiMove_C_API PUBLIC_XYPhiMove_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_XYPhiMove
#define PUBLIC_XYPhiMove_C_API __declspec(dllexport)
#else
#define PUBLIC_XYPhiMove_C_API __declspec(dllimport)
#endif

#define LIB_XYPhiMove_C_API PUBLIC_XYPhiMove_C_API


#else

#define LIB_XYPhiMove_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_XYPhiMove_C_API 
#define LIB_XYPhiMove_C_API /* No special import/export declaration */
#endif

extern LIB_XYPhiMove_C_API 
bool MW_CALL_CONV XYPhiMoveInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_XYPhiMove_C_API 
bool MW_CALL_CONV XYPhiMoveInitialize(void);

extern LIB_XYPhiMove_C_API 
void MW_CALL_CONV XYPhiMoveTerminate(void);



extern LIB_XYPhiMove_C_API 
void MW_CALL_CONV XYPhiMovePrintStackTrace(void);

extern LIB_XYPhiMove_C_API 
bool MW_CALL_CONV mlxGenerateXYPhiMotion(int nlhs, mxArray *plhs[], int nrhs, mxArray 
                                         *prhs[]);

extern LIB_XYPhiMove_C_API 
long MW_CALL_CONV XYPhiMoveGetMcrID();


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_XYPhiMove
#define PUBLIC_XYPhiMove_CPP_API __declspec(dllexport)
#else
#define PUBLIC_XYPhiMove_CPP_API __declspec(dllimport)
#endif

#define LIB_XYPhiMove_CPP_API PUBLIC_XYPhiMove_CPP_API

#else

#if !defined(LIB_XYPhiMove_CPP_API)
#if defined(LIB_XYPhiMove_C_API)
#define LIB_XYPhiMove_CPP_API LIB_XYPhiMove_C_API
#else
#define LIB_XYPhiMove_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_XYPhiMove_CPP_API void MW_CALL_CONV GenerateXYPhiMotion(const mwArray& omega_xy, const mwArray& omega_acc_xy, const mwArray& phi_fac, const mwArray& startangle_xy, const mwArray& startangle_phi, const mwArray& stopangle_xy, const mwArray& center_xy, const mwArray& radius_xy, const mwArray& moveback, const mwArray& dwelltime);

#endif
#endif
