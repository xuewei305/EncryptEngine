#ifndef _LOG_H
#define _LOG_H
#include <fltKernel.h>
#define LOGFL_ERRORS    0x00000001  // if set, display error messages
#define LOGFL_READ      0x00000002  // if set, display READ operation info
#define LOGFL_WRITE     0x00000004  // if set, display WRITE operation info
#define LOGFL_DIRCTRL   0x00000008  // if set, display DIRCTRL operation info
#define LOGFL_VOLCTX    0x00000010  // if set, display VOLCTX operation info
#define LOGFL_FILENAME  0x00000020  // if set, display file name 
#define LOGFL_FIND_UNICODESTRING  0x00000040  // if set, display file name  
#define LOGFL_SE  0x00000080

#define LOGFL (0x00000000 | LOGFL_SE)

#define LOG_PRINT( _logFlag, _string )                              \
    (FlagOn(LOGFL,(_logFlag)) ?                              \
        //DbgPrint ("Line:%4d ", __LINE__) , //DbgPrint( _string) :            \
        ((void)0))
        
        
#endif