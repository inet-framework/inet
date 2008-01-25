#include "Ripd.h"

#include "oppsim_kernel.h"

extern "C" {

int ripd_main_entry (int argc, char **argv);

};

Define_Module(Ripd);

void Ripd::activity()
{
	Daemon::init();
	
	// randomize start
    wait(uniform(0.001, 0.002));
    current_module = this;
    __activeVars = varp;
    GlobalVars_initializeActiveSet_ripd();

    EV << "ready for ripd_main_entry()" << endl;

	char *cmdline[] = { "ripd", NULL };
	
    ripd_main_entry(1, cmdline);	
}
