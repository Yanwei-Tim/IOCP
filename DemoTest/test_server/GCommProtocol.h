
#pragma once

#include "GSocket.h"

void GCommProt_Init(void);
BOOL GCommProt_ProcessReceive(PGHND_DATA pHndData, char* pBuf, DWORD dwBytes, PFN_ON_GHND_DATA_EVENT pfnOnProc);