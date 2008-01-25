#ifndef __RIPD_H__
#define	__RIPD_H__

#include "Daemon.h"

class Ripd : public Daemon 
{
	public:
		Module_Class_Members(Ripd, Daemon, 32768);
		virtual void activity();
};

#endif
