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
// Implements the Reliable Transport Protocol (RTP) for EIGRP IPv4. Handles
// reliable delivery of EIGRP messages, including sequence numbering, acknowledgments,
// and retransmissions. This IPv4-specific version works with the EigrpIpv4Pdm
// module to ensure reliable communication between EIGRP routers.
//
simple EigrpRtp extends SimpleModule
{
    parameters:
        @class(EigrpRtp);

    gates:
        input pdmIn;

        output pdmOut;
}
