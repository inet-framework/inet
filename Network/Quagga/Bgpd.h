#ifndef BGPD_H_
#define BGPD_H_

#include "Daemon.h"

class Bgpd : public Daemon 
{
	public:
		Module_Class_Members(Bgpd, Daemon, 32768);
		virtual void activity();
};


#endif
