// $Id$
//------------------------------------------------------------------------------
//	LambdaSplitter.ned --
//
//	This file defines a LambdaSplitter module, a logical wavelength router
//	on the path between the OLT and the ONUs in a PON.
//
//  Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//------------------------------------------------------------------------------


package inet.nodes.pon;


simple LambdaSplitter
{
    parameters:
		@node();
		@display("i=device/switch");
    gates:
        inout mux;					// a port for WDM-multiplexed optical streams
        inout demux[];				// ports for WDM-demultiplexed optical streams
}
