// $Id$
//------------------------------------------------------------------------------
//	LambdaSplitter.ned --
//
//	This file defines a LambdaSplitter module, a logical wavelength router
//	on the path between an OLT and ONUs in a PON.
//
//  Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


package inet.linklayer.hybridpon;


simple LambdaSplitter
{
    parameters:
        @node();
        @display("i=abstract/opticalswitch_s");
    gates:
        inout demuxg[];	// gates for WDM-demultiplexed optical streams
        inout muxg;		// a gate for WDM-multiplexed optical streams
}
