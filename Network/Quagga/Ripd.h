#ifndef __RIPD_H__
#define	__RIPD_H__

#include "Daemon.h"

extern "C" {

extern struct GlobalVars_ripd * __activeVars_ripd;
extern void GlobalVars_initializeActiveSet_ripd();
extern struct GlobalVars_ripd * GlobalVars_createActiveSet_ripd();

}


class Ripd : public Daemon 
{
	public:
		Module_Class_Members(Ripd, Daemon, 32768);
		virtual void activity();
        
    protected:
        virtual void activate();
        
    private:
        struct GlobalVars_ripd *varp_ripd;
};

#endif
