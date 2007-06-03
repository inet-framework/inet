#ifndef OSPFD_H_
#define OSPFD_H_

#include "Daemon.h"

extern "C" {

extern struct GlobalVars_ospfd * __activeVars_ospfd;
extern void GlobalVars_initializeActiveSet_ospfd();
extern struct GlobalVars_ospfd * GlobalVars_createActiveSet_ospfd();

}

class Ospfd : public Daemon 
{
	public:
		Module_Class_Members(Ospfd, Daemon, 32768);
		virtual void activity();
        
    protected:
        virtual void activate();
        
    private:
        struct GlobalVars_ospfd *varp_ospfd;
};	

#endif
