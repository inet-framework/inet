#include "Ospfd.h"

#include "oppsim_kernel.h"

extern "C" {

int ospfd_main_entry (int argc, char **argv);

};

struct GlobalVars_ospfd *__activeVars_ospfd = NULL;

Define_Module(Ospfd);

void Ospfd::activity()
{
	Daemon::init();

    // randomize start
    wait(uniform(0.002, 0.003));
    
    varp_ospfd = GlobalVars_createActiveSet_ospfd();
    activate();
    GlobalVars_initializeActiveSet_ospfd();

    EV << "ready for ospfd_main_entry()" << endl;

	char *cmdline[] = { "ospfd", NULL };
    ospfd_main_entry(1, cmdline);
}

void Ospfd::activate() {
    Daemon::activate();
    __activeVars_ospfd = varp_ospfd;
}
