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
package inet.routing.eigrp.pdms;

import inet.common.SimpleModule;

//
// Protocol Dependent Module (PDM) for EIGRP IPv6 implementation. Handles the
// IPv6-specific aspects of EIGRP routing, including route computation, neighbor
// relationship management, and topology table maintenance. Communicates with
// the Reliable Transport Protocol (RTP) module for message delivery and the
// splitter for network layer interaction.
//
simple EigrpIpv6Pdm extends SimpleModule
{
    parameters:
        @class(EigrpIpv6Pdm);
        xml configData = default(xmldoc("config.xml"));
        string interfaceTableModule;
        string routingTableModule;

    gates:
        input splitterIn;
        input rtpIn;

        output splitterOut;
        output rtpOut;
}
