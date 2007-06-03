#include "Bgpd.h"

#include "oppsim_kernel.h"

extern "C" {

int bgpd_main_entry (int argc, char **argv);

};

struct GlobalVars_bgpd *__activeVars_bgpd = NULL;


Define_Module(Bgpd);

void Bgpd::activity()
{
	Daemon::init();
	
	// randomize start
    wait(uniform(0.001, 0.002));

    varp_bgpd = GlobalVars_createActiveSet_bgpd();
    activate();
    GlobalVars_initializeActiveSet_bgpd();

    EV << "ready for bgpd_main_entry()" << endl;

	char *cmdline[] = { "bgpd", NULL };
    bgpd_main_entry(1, cmdline);
}

void Bgpd::activate() {
    Daemon::activate();
    __activeVars_bgpd = varp_bgpd;
}
