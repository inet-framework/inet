// $Id$
//------------------------------------------------------------------------------
//	LambdaSplitter.ned --
//
//	This file defines a LambdaSplitter module, a logical wavelength router
//	on the path between the OLT and the ONUs in a hybrid TDM/WDM-PON.
//
//  Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


package inet.nodes.pon;


simple LambdaSplitter
{
    parameters:
		@node();
		@display("i=device/switch");
//        int numOnus;              // number of ONUs 
        string distances;
    gates:
        inout olt;					// connected to an OLT
        inout onus[];				// connected to ONUs
}
