//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// @author Jan Zavrel (honza.zavrel96@gmail.com)
// @author Jan Bloudicek (jbloudicek@gmail.com)
// @author Vit Rek (rek@kn.vutbr.cz)
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

//TODO review
package inet.routing.eigrp;

import inet.common.SimpleModule;

//
// Routes EIGRP packets between the network layer and the appropriate protocol
// data modules based on IP version. Directs IPv4 packets to the IPv4 PDM and
// IPv6 packets to the IPv6 PDM. Acts as a central connection point for dual-stack
// EIGRP implementations, enabling protocol operation over both IP versions.
//
simple EigrpSplitter extends SimpleModule
{
    parameters:
        @class(EigrpSplitter);
    gates:
        input ipIn @labels(Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down,Ipv6ControlInfo/down);

        input splitterIn @labels(Ipv4ControlInfo/up);
        input splitter6In  @labels(Ipv6ControlInfo/up);
        output splitterOut @labels(Ipv4ControlInfo/down);
        output splitter6Out @labels(Ipv6ControlInfo/down);
}
