/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRPathGainModel.h
 *
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

#ifndef _UWBIRPATHLOSSMODEL_H
#define	_UWBIRPATHLOSSMODEL_H

#include <string>

#include "MiXiMDefs.h"
#include "AnalogueModel.h"
#include "Mapping.h"

/**
 * @brief This AnalogueModel models implements the Ghassmezadeh stochastic UWB channel models.
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
class MIXIM_API UWBIRStochasticPathlossModel : public AnalogueModel {

public:
    virtual ~UWBIRStochasticPathlossModel() {}

    static const double Gtx;
    static const double Grx;
    static const double ntx;
    static const double nrx;
    double PL0; // 0.000008913; // -50.5 dB   0.000019953
    static const double fc; // mandatory band 3, center frequency, MHz
    static const double d0;
    double mu_gamma, sigma_gamma; //1.7, 0.3
    double mu_sigma, sigma_sigma;
    double gamma, S, sigma;
    double n1, n2, n3;
    static double n1_limit;
    static double n2_limit;

    static const double s_mu;
    static const double s_sigma;
    static const double kappa;

    bool isEnabled, shadowing;

    cOutVector distances;
    cOutVector srcPosX, srcPosY, dstPosX, dstPosY;

    int myIndex;
    std::string myName;

    UWBIRStochasticPathlossModel()
    	: AnalogueModel()
    	, PL0(0)
    	, mu_gamma(0), sigma_gamma(0)
    	, mu_sigma(0), sigma_sigma(0)
    	, gamma(0.0), S(0.0), sigma(0.0)
    	, n1(0.0), n2(0.0), n3(0.0)
    	, isEnabled(false)
    	, shadowing(true)
    	, distances()
    	, srcPosX(), srcPosY(), dstPosX(), dstPosY()
    	, myIndex(-1)
    	, myName()
    	, pathloss_exponent(0.0)
    	, fading(0.0)
    	, frequency("frequency")
    	, pathlosses()
    {
    	distances.setName("distances");
    	srcPosX.setName("srcPosX");
    	srcPosY.setName("srcPosY");
    	dstPosX.setName("dstPosX");
    	dstPosY.setName("dstPosY");
    	pathlosses.setName("pathloss");
    }

    /** @brief Initialize the analog model from XML map data.
     *
     * This method should be defined for generic analog model initialization.
     *
     * @param params The parameter map which was filled by XML reader.
     *
     * @return true if the initialization was successfully.
     */
    virtual bool initFromMap(const ParameterMap&);

    void filterSignal(airframe_ptr_t, const Coord&, const Coord&);

protected:
    double pathloss_exponent;
    double fading;
    Dimension frequency;
    cOutVector pathlosses;  // outputs computed pathlosses. Allows to compute Eb = Epulse*pathloss for Eb/N0 computations. (N0 is the noise sampled by the receiver)

    static double getNarrowBandFreeSpacePathloss(double fc, double distance);
    double getGhassemzadehPathloss(double distance) const;
    double getFDPathloss(double freq, double distance) const;
    static double simtruncnormal(double mean, double stddev, double a, int rng);

};

#endif	/* _UWBIRPATHLOSSMODEL_H */

