//***************************************************************************
//* file:        Network.ned
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
//* description: A test networl for cc connections
//*
//***************************************************************************
//* changelog:   $Revision: 80 $
//*              last modified:   $Date: 2004-07-23 20:05:13 +0200 (Fr, 23 Jul 2004) $
//*              by:              $Author: omfw-willkomm $
//***************************************************************************

package inet.tests.connectionManager;

import inet.tests.ExtTestNetwork;

// Test network for ConnectionManager.
// @author Karl Wessel
network ConnectionManagerTestNet extends ExtTestNetwork
{
    submodules:
        connectionManager: TestCM {
            parameters:
                @display("p=201,0;i=abstract/multicast;is=s");
        }
        node[numHosts]: BasicHost {
            parameters:
                numHosts = numHosts;
                @display("p=50,50;i=laptop");
        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}
