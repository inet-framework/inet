#include "Bgpd.h"

#include "oppsim_kernel.h"

extern "C" {

int bgpd_main_entry (int argc, char **argv);

};

Define_Module(Bgpd);

void Bgpd::activity()
{
	Daemon::init();
	
	// randomize start
    wait(uniform(0.001, 0.002));
    current_module = this;
    __activeVars = varp;
    GlobalVars_initializeActiveSet_bgpd();

    EV << "ready for bgpd_main_entry()" << endl;

	char *cmdline[] = { "bgpd", NULL };
    bgpd_main_entry(1, cmdline);
}
