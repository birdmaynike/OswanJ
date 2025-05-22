/*
$Date: 2010-05-19 08:16:59 +0900 (2010/05/19 (水)) $
$Rev: 97 $
*/

#ifndef WSFILEIO_H_
#define WSFILEIO_H_

extern wchar_t IniPath[512];

int WsCreate(wchar_t *CartName);
void WsRelease(void);
void WsLoadEeprom(void);
void WsSaveEeprom(void);
void WsLoadState(int num);
void WsSaveState(int num);

#endif
