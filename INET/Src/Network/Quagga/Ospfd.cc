#include "Ospfd.h"

#include "oppsim_kernel.h"

extern "C" {

int ospfd_main_entry (int argc, char **argv);

};

Define_Module(Ospfd);

void Ospfd::activity()
{
	Daemon::init();

    // randomize start
    wait(uniform(0.002, 0.003));
    current_module = this;
    __activeVars = varp;
    GlobalVars_initializeActiveSet_ospfd();

    EV << "ready for ospfd_main_entry()" << endl;

	char *cmdline[] = { "ospfd", NULL };
    ospfd_main_entry(1, cmdline);
}
