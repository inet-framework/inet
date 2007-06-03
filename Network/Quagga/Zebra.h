#ifndef ZEBRA_H_
#define ZEBRA_H_

#include "Daemon.h"

extern "C" {

extern struct GlobalVars_zebra * __activeVars_zebra;
extern void GlobalVars_initializeActiveSet_zebra();
extern struct GlobalVars_zebra * GlobalVars_createActiveSet_zebra();

}


class Zebra : public Daemon 
{
	public:
		Module_Class_Members(Zebra, Daemon, 32768);
		virtual void activity();
        
    protected:
        virtual void activate();
        
    private:
        struct GlobalVars_zebra *varp_zebra;
};	

#endif
