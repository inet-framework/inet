// $Id$
//------------------------------------------------------------------------------
//	Splitter.ned --
//
//	This file defines a splitter module connecting several packet generators
//	(each representing a user) to a FIFO queue on the ONU.
//	We need this module due to lack of point-to-multipoint connections support
//	in OMNeT++.
//
//  Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


package inet.linklayer.hybridpon;


simple Splitter
{
    parameters:
        int numUsersPerOnu;
    gates:
        input fromUser[];         // IP packets from packet generators
        output toMac;      // IP packets to ONU Mac
}