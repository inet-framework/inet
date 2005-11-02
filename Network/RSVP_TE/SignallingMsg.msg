
cplusplus {{
#include "IPAddress.h"
#include "IntServ.h"

#define	MSG_PSB_TIMER				1
#define	MSG_PSB_TIMEOUT				2

#define	MSG_RSB_REFRESH_TIMER		3
#define	MSG_RSB_COMMIT_TIMER		4
#define	MSG_RSB_TIMEOUT				5

#define	MSG_HELLO_TIMER				6
#define	MSG_HELLO_TIMEOUT			7

#define	MSG_PATH_NOTIFY				8

#define	PATH_CREATED				1
#define	PATH_UNFEASIBLE				2
#define	PATH_FAILED					3
#define	PATH_PREEMPTED				4
#define	PATH_RETRY					5
}}

class noncobject IPAddress;
class noncobject SessionObj_t;
class noncobject SenderTemplateObj_t;

//
// FIXME missing documentation
//
message SignallingMsg
{
	fields:
		int command = 0;
};

//
// FIXME missing documentation
//
message PsbTimerMsg extends SignallingMsg
{
	fields:
		int id;
	
		int command = MSG_PSB_TIMER;
};

//
// FIXME missing documentation
//
message PsbTimeoutMsg extends SignallingMsg
{
	fields:
		int id;
	
		int command = MSG_PSB_TIMEOUT;
};

//
// FIXME missing documentation
//
message RsbRefreshTimerMsg extends SignallingMsg
{
	fields:
		int id;
	
		int command = MSG_RSB_REFRESH_TIMER;
};

//
// FIXME missing documentation
//
message RsbCommitTimerMsg extends SignallingMsg
{
	fields:
		int id;
		
		int command = MSG_RSB_COMMIT_TIMER;
};

//
// FIXME missing documentation
//
message RsbTimeoutMsg extends SignallingMsg
{
	fields:
		int id;
	
		int command = MSG_RSB_TIMEOUT;
};

//
// FIXME missing documentation
//
message HelloTimerMsg extends SignallingMsg
{
	fields:
		IPAddress peer;
	
		int command = MSG_HELLO_TIMER;
};

//
// FIXME missing documentation
//
message HelloTimeoutMsg extends SignallingMsg
{
	fields:
		IPAddress peer;
	
		int command = MSG_HELLO_TIMEOUT;
};

//
// FIXME missing documentation
//
message PathNotifyMsg extends SignallingMsg
{
	fields:
		SessionObj_t session;
		SenderTemplateObj_t sender;
		
		int status;
		
		int command = MSG_PATH_NOTIFY;
};
