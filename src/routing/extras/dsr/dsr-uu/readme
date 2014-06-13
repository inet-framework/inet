DSR-UU is a DSR implementation that runs in Linux (kernel 2.4/2.6) and
in the ns-2 network simulator.


DISCLAIMER
==========

This is experimental software. It will probably crash your computer at
some point. Use at your own risk. Bug reports are welcome. Send to
erikn@it.uu.se.


General Notes
=============

DSR-UU implements most of the basic DSR features specified in the DSR
draft (version 10). One big exception is flow extensions.

DSR-UU does NOT use ARP, so do not be surprised if you do not see ARP
traffic. DSR-UU instead uses its own neighbor table that sets up the
MAC-to-IP translation during route discovery.

Ns-2 version
============

In ns-2, DSR-UU has very similar (if not the same) performance as the
ns-2 bundled DSR implementation. In addition, DSR-UU supports network
layer acknowledgements.

Trace support:

DSR-UU uses the SRNodeNew class that the ns-2 DSR implementation
uses. However, because DSR-UU has "real" DSR headers, DSR packet
parsing is not supported in the trace file. May be added in the future.

Linux version
=============

DSR-UU implements a virtual network interface (dsr0). This enables
your DSR network to coexist with the regular non-multihop ad hoc
network at the same time. You need to configure a seperate subnet for
the DSR interface.

To be able to add the extra DSR headers to data packets, DSR-UU
reserves DSR_OPTS_MAX_SIZE bytes for the DSR headers by reducing the
MTU of the virtual network device. This also puts a maximum size limit
on the length of DSR headers. DSR_OPTS_MAX_SIZE can be changed in
dsr.h at compile time.

Routing Cache
=============

DSR-UU supports multiple routing cache implementations. The routing
cache is loaded as a seperate module in Linux, so that different
implementations of a routing cache can be tested without
recompilation. DSR-UU implements a link cache that supports multiple
routing metrics. Currently only minimal hop count is used.

Link Layer Acknowledgement
==========================

Link layer acknowledgements are implemented in the ns-2 version of
DSR-UU. It is enabled by default, but can be turned off by setting the
configuration value UseNetworkLayerAck_ to 1 in your simulation
script. Note that this also reduces the efficiency of packet
salvaging, since packets are not fetched from the interface queue
(this is to better mimic reality).


Installation
============

* To compile for Linux (against currently running kernel), simply do:

> make

* To compile against another kernel:

> make KERNEL_DIR=/path/to/configured/kernel/sources

* To compile against MIPS (e.g., LinkSys WRT54G routers):

> make mips KERNEL_DIR=/path/to/mips/kernel/sources

NOTE: You must have the Mips cross compiler in your PATH.

* Install the modules:

> make install

* Uninstall:

> make uninstall

* Compile for ns-2 version 2.28:

1. Symlink the dsr-uu directory into the ns-2 source tree:

> cd /path/to/ns-2/ns-2.28 && ln -s /path/to/dsr-uu dsr-uu;

2. Apply the ns-2 DSR-UU patch

> cd ns-allinone-X.XX/ns-X.XX/
> patch -p1 < /path/to/dsr-uu/patches/ns-2.patch 

3. Compile ns-2

4. Use "DSRUU" to select the protocol in the ns-2 scenario file you
use for the simulation.


* Running DSR-UU in Linux:

Insert the kernel modules "dsr.{o,ko}" and "linkcache.{o,ko}" by doing:

> insmod linkcache.{o,ko} && insmod dsr.{o,ko} ifname=<interface to attach to>

Bring the "dsr0" interface up:

> ifconfig dsr0 192.168.2.2 up

DSR is now running.

Alternatilvely, use the dsr-uu.sh start script.

Start:

> dsr-uu.sh start ethX

Stop:

> dsr-uu.sh stop

Runtime Configuration Values
============================

Configuration values are listed in dsr.h. Note that not all
configuration values actually do anything at this point. They are
simply taken from the DSR draft. Strangly, the draft also list some
values that are actually never used in the draft.

Linux:
------

DSR configuration values can be set at runtime through
/proc/net/dsr_config.

Do:

> cat /proc/net/dsr_config

to list configuration values (not all of them might do anything at
this point in time).

Do for example:

> echo "PrintDebug=0" > /proc/net/dsr_config

to set a configuration value.

See /proc/net/ for other output and debugging options.

Available /proc/net files:

/proc/net/dsr_config     - List or set configuration values.
/proc/net/dsr_dbg        - DSR debug output.
/proc/net/dsr_lc         - Link cache
/proc/net/dsr_neigh_tbl  - Neighbor table
/proc/net/dsr_rreq_tbl   - Route request table
/proc/net/maint_buf      - Packets in maintenance buffer
/proc/net/send_buf       - Packets in send buffer


Ns-2
----

Configuration values can be set at runtime from a ns-2 simulation
script. To set a configuration value for a node, do e.g.:

set r [$node_($i) set ragent_]  # Get routing agent
$r set UseNetworkLayerAck_ 1
$r set PrintDebug_ 1

# eof

