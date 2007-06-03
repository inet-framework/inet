#ifndef BGPD_H_
#define BGPD_H_

#include "Daemon.h"

extern "C" {

extern struct GlobalVars_bgpd * __activeVars_bgpd;
extern void GlobalVars_initializeActiveSet_bgpd();
extern struct GlobalVars_bgpd * GlobalVars_createActiveSet_bgpd();

}

class Bgpd : public Daemon 
{
	public:
		Module_Class_Members(Bgpd, Daemon, 32768);
		virtual void activity();
        
    protected:
        virtual void activate();
        
    private:
        struct GlobalVars_bgpd *varp_bgpd;
};


#endif
