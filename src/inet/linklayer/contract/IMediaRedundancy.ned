//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for MediaRedundancyNodes
//
moduleinterface IMediaRedundancy
{
    parameters:
        @display("i=block/network2");
        double ccmInterval @unit(ms);
        string mrpRole;
        int timingProfile @unit(ms);
        int uuid0;
		int uuid1;
		int ringPort1;
		int ringPort2;
		bool interconnectionLinkCheckAware;
		bool interconnectionRingCheckAware;
       	bool nonBlockingMRC;
        bool reactOnLinkChange;
        bool checkMediaRedundancy;
        bool noTopologyChange;	
        bool visualize;
    gates:
        input relayIn;
        output relayOut;
}

