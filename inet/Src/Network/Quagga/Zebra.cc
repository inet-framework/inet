#include "Zebra.h"

#include "oppsim_kernel.h"

extern "C" {

int zebra_main_entry (int argc, char **argv);

};

Define_Module(Zebra);

void Zebra::activity()
{
	Daemon::init();
	
	// randomize start
    wait(uniform(0, 0.001));
    current_module = this;
    __activeVars = varp;
    GlobalVars_initializeActiveSet_zebra();

    EV << "ready for zebra_main_entry()" << endl;

	char *cmdline[] = { "zebra", NULL };
    zebra_main_entry(1, cmdline);
}
