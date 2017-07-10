INET Framework for OMNEST/OMNeT++
=================================

The INET framework (inet.omnetpp.org) is an open-source communication networks 
simulation package, written for the OMNEST/OMNeT++ simulation system. The INET 
framework contains models for several Internet protocols: beyond TCP and IP there 
is UDP, Ethernet, PPP and MPLS with LDP and RSVP-TE signalling. See the CREDITS 
file for the names of people who have contributed to the INET Framework.

IMPORTANT: The INET Framework is continuously being improved: new parts
are added, bugs are corrected, and so on. We cannot assert that any protocol
implemented here will work fully according to the specifications. YOU ARE
RESPONSIBLE YOURSELF TO MAKE SURE THAT THE MODELS YOU USE IN YOUR SIMULATIONS
WORK CORRECTLY, AND YOU'RE GETTING VALID RESULTS.

Contributions are highly welcome. You can make a difference!

See the WHATSNEW file for recent changes.


GETTING STARTED
---------------
You may start by downloading and installing the INET framework. Read the INSTALL
file for further information.

Then you can gather initial experience by starting some examples or following the 
INET wireless tutorial (tutorials/wireless). After that, you can learn the NED 
language from the OMNeT++ manual & sample simulations.

After that, you may write your own topologies using the NED language. You may
assign some of the submodule parameters in NED files. You may leave some of
them unassigned.

Then, you may assign unassigned module parameters in omnetpp.ini of your
simulation. (You can refer to sample simulations & manual for the content of
omnetpp.ini)

Finally, you will be ready to run your simulation. As you see, you may use
the INET framework without writing any C++ code, as long as you use the
available modules.

To implement new protocols or modify existing ones, you'll need to add your 
code somewhere under the src directory. If you add new files under the 'src' 
directory you will need to regenerate the makefiles (using the 'make makefiles' 
command).

If you want to use external interfaces in INET, please install libpcap-dev (on linux)
or winpcap (on windows from www.winpcap.org) and enable the "Emulation" feature
either in the IDE or using the inet_featuretool then regenerate the INET makefile
using 'make makefiles'.
