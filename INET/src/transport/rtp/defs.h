//temporary header file --AV, 20050423
#include "IPAddress.h"
#define IN_Addr IPAddress
#define IN_Port int
#define IPSuite_PORT_UNDEF 0

#ifndef ev
#define ev EV<<getParentModule()->getFullPath()<<"."<<getClassName()<<":"
#endif
