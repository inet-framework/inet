#ifndef OSPFD_H_
#define OSPFD_H_

#include "Daemon.h"

class Ospfd : public Daemon 
{
	public:
		Module_Class_Members(Ospfd, Daemon, 32768);
		virtual void activity();
};	

#endif
