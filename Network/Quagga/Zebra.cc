#include "Zebra.h"

#include "oppsim_kernel.h"

extern "C" {

int zebra_main_entry (int argc, char **argv);

};

struct GlobalVars_zebra *__activeVars_zebra = NULL;

Define_Module(Zebra);

void Zebra::activity()
{
	Daemon::init();
	
	// randomize start
    wait(uniform(0, 0.001));
    
    varp_zebra = GlobalVars_createActiveSet_zebra();
    activate();
    GlobalVars_initializeActiveSet_zebra();

    EV << "ready for zebra_main_entry()" << endl;

	char *cmdline[] = { "zebra", NULL };
    zebra_main_entry(1, cmdline);
}

void Zebra::activate() {
    Daemon::activate();
    __activeVars_zebra = varp_zebra;
}
