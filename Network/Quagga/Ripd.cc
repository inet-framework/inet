#include "Ripd.h"

#include "oppsim_kernel.h"

extern "C" {

int ripd_main_entry (int argc, char **argv);

};

struct GlobalVars_ripd *__activeVars_ripd = NULL;

Define_Module(Ripd);

void Ripd::activity()
{
	Daemon::init();
	
	// randomize start
    wait(uniform(0.001, 0.002));

    varp_ripd = GlobalVars_createActiveSet_ripd();
    activate();
    GlobalVars_initializeActiveSet_ripd();

    EV << "ready for ripd_main_entry()" << endl;

	char *cmdline[] = { "ripd", NULL };
	
    ripd_main_entry(1, cmdline);	
}

void Ripd::activate() {
    Daemon::activate();
    __activeVars_ripd = varp_ripd;
}

