//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.physicallayer.wireless.common.pathloss;

import inet.common.CompoundModule;
import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

//
// This is a statistical path-loss model for UWB propagation in residential
// environments, for both line-of-sight (LOS) and non-line-ofsight (NLOS) cases.
// It implements the Ghassmezadeh stochastic UWB channel path loss model.
//
// Path loss in dB at a distance d is modeled as
// follows:
//
// PL(d) = [PL0 + 10 * sigma *log10(d/d0)] + S(d), where d >= 0
//
// where the reference distance d0 is equal to 1 m, PL0 is the path loss at the
// reference distance, sigma is the path-loss exponent and S is the shadow fading.
// The path-loss exponent follows a normal distribution N[muSigma, sigmaSigma] and
// the shadow fading is shown to be log-normal (with a standard deviation).
//
// The parameter values of the Ghassemzadeh channel model for LOS and NLOS
// environments are the following:
//
// LOS:
//
// *.radioMedium.pathLoss.PL0 = 47dB
// *.radioMedium.pathLoss.muGamma = 1.7
// *.radioMedium.pathLoss.muSigma = 1.6
// *.radioMedium.pathLoss.sigmaGamma = 0.3
// *.radioMedium.pathLoss.sigmaSigma = 0.5
//
// NLOS:
//
// *.radioMedium.pathLoss.PL0 = 51dB
// *.radioMedium.pathLoss.muGamma = 3.5
// *.radioMedium.pathLoss.muSigma = 2.7
// *.radioMedium.pathLoss.sigmaGamma = 0.97
// *.radioMedium.pathLoss.sigmaSigma = 0.98
//
// These configuration values are based on Table 1. of [1].
//
// Citation of the following publication is requested by the authors if you use
// this model in a publication of your own:
//
// J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
// layer model for network simulation. SIMULATION January 2011 vol. 87 no. 1-2 82-112.
//
// For more information, see also:
//
// [1] J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
// layer model for network simulation. SIMULATION January 2011 vol. 87 no.
// 1-2 82-112. http://dx.doi.org/10.1177/0037549710377767
//
// [2] J. Rousselot, Ultra Low Power Communication Protocols for UWB
// Impulse Radio Wireless Sensor Networks. EPFL Thesis 4720, 2010.
// http://infoscience.epfl.ch/record/147987
//
// [3]  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// http://portal.acm.org/citation.cfm?id=1537714
//
module UwbIrStochasticPathLoss extends CompoundModule like IPathLoss
{
    parameters:
        double PL0 @unit(dB); // Path loss at reference distance of 1m [dB]
        double muGamma; // Mean path loss exponent
        double muSigma; // Mean standard deviation of shadowing [dB]
        double sigmaGamma; // Standard deviation of path loss exponent
        double sigmaSigma; // Standard deviation of the standard deviation of shadowing
        bool shadowing; // Enable/disable shadowing
        @class(UwbIrStochasticPathLoss);
        @display("i=block/control");
}

