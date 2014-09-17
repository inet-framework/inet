/* -*- mode:c++ -*- ********************************************************
 * author:      Jerome Rousselot
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this AnalogueModel models free-space pathloss
 ***************************************************************************/

#ifndef __INET_UWBIRSTOCHASTICPATHLOSS_H
#define __INET_UWBIRSTOCHASTICPATHLOSS_H

#include "inet/physicallayer/contract/IPathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the Ghassmezadeh stochastic UWB channel path loss model.
 *
 * Citation of the following publication is appreciated if you use the MiXiM UWB PHY model
 * for a publication of your own.
 * J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
 * layer model for network simulation. SIMULATION January 2011 vol. 87 no. 1-2 82-112.
 *
 * For more information, see also:
 *
 * [1] J. Rousselot, J.-D. Decotignie, An ultra-wideband impulse radio PHY
 * layer model for network simulation. SIMULATION January 2011 vol. 87 no.
 * 1-2 82-112. http://dx.doi.org/10.1177/0037549710377767
 * [2] J. Rousselot, Ultra Low Power Communication Protocols for UWB
 * Impulse Radio Wireless Sensor Networks. EPFL Thesis 4720, 2010.
 * http://infoscience.epfl.ch/record/147987
 * [3]  A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
 * for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
 * Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
 * http://portal.acm.org/citation.cfm?id=1537714
 *
 * @ingroup ieee802154a
 * @ingroup analogueModels
 */
class INET_API UWBIRStochasticPathLoss : public cModule, public IPathLoss
{
  protected:
    static const Hz fc; // mandatory band 3, center frequency
    static const m d0;
    static const double n1_limit;
    static const double n2_limit;
    static const double n3_limit;
    static const double s_mu;
    static const double s_sigma;
    static const double kappa;

    double PL0; // 0.000008913; // -50.5 dB   0.000019953
    double muGamma;
    double muSigma;
    double sigmaGamma; //1.7, 0.3
    double sigmaSigma;
    double pathloss_exponent;
    bool shadowing;

  protected:
    virtual void initialize(int stage);
    virtual double getFDPathLoss(Hz frequency, m distance) const;
    virtual double getGhassemzadehPathLoss(double gamma, double S, m distance) const;
    virtual double getNarrowBandFreeSpacePathLoss(Hz frequency, m distance) const;
    virtual double simtruncnormal(double mean, double stddev, double a, int rng) const;

  public:
    UWBIRStochasticPathLoss();
    virtual void printToStream(std::ostream& stream) const;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const;
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const { return m(NaN); }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_UWBIRSTOCHASTICPATHLOSS_H

