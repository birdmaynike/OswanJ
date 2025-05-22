/*
$Date: 2010-09-29 17:51:41 +0900 (2010/09/29 (水)) $
$Rev: 102 $
*/

#include <windows.h>
#include <tchar.h>
#include "WSError.h"

void  ErrorMsg(long err)
{
    TCHAR dst[512];

    _stprintf_s(dst, 512, TEXT("%lX"), err);
    MessageBox(NULL, dst, TEXT("WS エラー"), MB_OK);
}

