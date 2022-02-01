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

package inet.routing.eigrp;

simple EigrpSplitter
{
    gates:
        input ipIn @labels(Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down,Ipv6ControlInfo/down);

        input splitterIn @labels(Ipv4ControlInfo/up);
        input splitter6In  @labels(Ipv6ControlInfo/up);
        output splitterOut @labels(Ipv4ControlInfo/down);
        output splitter6Out @labels(Ipv6ControlInfo/down);
}



