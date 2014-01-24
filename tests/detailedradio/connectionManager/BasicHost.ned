//***************************************************************************
//* file:        BasicHost.ned
//*
//* author:      Daniel Willkomm
//*
//* copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//***************************************************************************
//* part of:     framework implementation developed by tkn
//* description: BasicHost Compound Module
//*
//***************************************************************************
//* changelog:   $Revision: 327 $
//*              last modified:   $Date: 2006-06-23 21:56:51 +0200 (Fr, 23 Jun 2006) $
//*              by:              $Author: koepke $
//***************************************************************************

package inet.tests.connectionManager;

import inet.mobility.static.StationaryMobility;

import inet.tests.TestNode;

// The BasicHost is a compound module that implements a very
// simple host. It only consists of basic modules and is the
// most basic host you can create for the MF
//
// @author Daniel Willkomm
module BasicHost extends TestNode
{
    parameters:
        double numHosts; // total number of hosts in the network
        string phyLayer; //physical layer type

        @display("bgb=180,200,white,,;bgp=10,10");
        @node();
    gates:
        input radioIn;

    submodules:
        mobility: StationaryMobility {
            parameters:
                @display("p=130,130;i=cogwheel2");
        }
        nic: TestNic {
            parameters:
                phyLayerType = phyLayer;

                @display("b=32,30;p=60,166;i=iface");
        }
    connections:

        radioIn --> nic.radioIn;

}

