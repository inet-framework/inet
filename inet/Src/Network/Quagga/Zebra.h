#ifndef ZEBRA_H_
#define ZEBRA_H_

#include "Daemon.h"

class Zebra : public Daemon 
{
	public:
		Module_Class_Members(Zebra, Daemon, 32768);
		virtual void activity();
};	

#endif
