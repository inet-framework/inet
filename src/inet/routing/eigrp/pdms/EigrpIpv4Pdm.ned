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


package inet.routing.eigrp.pdms;

simple EigrpIpv4Pdm
{
    parameters:
        xml configData = default(xmldoc("config.xml"));
        string interfaceTableModule;
        string routingTableModule;
    gates:
        input splitterIn;
        input rtpIn;

        output splitterOut;
        output rtpOut;
}
