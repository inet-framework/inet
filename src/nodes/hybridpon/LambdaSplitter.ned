//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
//
// This software is written and distributed under the GNU General
// Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
// You must not remove this notice, or any other, from this software.
//


package inet.nodes.hybridpon;

//
// Model of a lambda (wavelenth) splitter -- implemented by
// <a href="http://en.wikipedia.org/wiki/Arrayed_waveguide_grating">arrayed
// waveguide grating (AWG)</a> device in practice -- on the path between
// Olt and Onu in a passive optical network (PON).
//
// <i>Flow-through</i> reception mode is used at all input gates to model
// all-optical tranmission of frames through the device.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-03-22
//
simple LambdaSplitter
{
    parameters:
        @node();
        @labels(node,hybridpon-node);
        @display("i=abstract/opticalswitch_s");
    gates:
        inout demuxg[];	// gate vector for WDM-demultiplexed optical streams (for ONUs)
        inout muxg[];	// gate vector for WDM-multiplexed optical streams (for OLT)
}
