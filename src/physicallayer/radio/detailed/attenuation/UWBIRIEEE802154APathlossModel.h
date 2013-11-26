/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRIEEE802154APathlossModel.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008-2009 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
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
 * description: this AnalogueModel implements the IEEE 802.15.4A Channel Model
 ***************************************************************************/

#ifndef _UWBIRIEEE802154APATHLOSSMODEL_H
#define	_UWBIRIEEE802154APATHLOSSMODEL_H

#include "MiXiMDefs.h"
#include "AnalogueModel.h"
#include "SimpleTimeConstMapping.h"

/**
 * @brief This class implements the IEEE 802.15.4A Channel Model[1] in the MiXiM
 * omnet++ modeling framework.
 *
 *  "IEEE 802.15.4a channel model - final report", 2005, Andreas F. Molisch,
 *  Kannan Balakrishnan, Dajana Cassioli, Chia-Chin Chong, Shahriar Emami,
 *  Andrew Fort, Johan Karedal, Juergen Kunisch, Hans Schantz, Ulrich Schuster, Kai Siwiak
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
 * @ingroup analogueModels
 * @ingroup ieee802154a
 */
class MIXIM_API UWBIRIEEE802154APathlossModel : public AnalogueModel {
private:
	/** @brief Copy constructor is not allowed.
	 */
	UWBIRIEEE802154APathlossModel(const UWBIRIEEE802154APathlossModel&);
	/** @brief Assignment operator is not allowed.
	 */
	UWBIRIEEE802154APathlossModel& operator=(const UWBIRIEEE802154APathlossModel&);

public:

	/*
	 * @brief Default constructor.
	 */
	UWBIRIEEE802154APathlossModel()
		: AnalogueModel()
		, channelModel(0)
		, cfg()
		, tapThreshold(0)
		, doShadowing(false)
 		, doSmallScaleShadowing(false)
		, newTxPower(NULL)
		, txPower(NULL)
		, arg()
		, pulsesIter(NULL)
		, L(0)
		, S(0)
		, clusterStart()
		, gamma_l()
		, Mcluster(0)
		, Omega_l(0)
		, averagePower(0)
		, nbCalls(0)
		, averagePowers()
		, pathlosses()
    {
    	averagePowers.setName("averagePower");
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

    virtual ~UWBIRIEEE802154APathlossModel() {}

    /*
     * @brief Applies the model to an incoming AirFrame's Signal.
     */
    void filterSignal(airframe_ptr_t, const Coord&, const Coord&);

    /*@brief Utility function to use a Rayleigh random variable
     *
     */
    double Rayleigh(double param);

    /* @brief Lists implemented channel models.
     * */
    static const bool implemented_CMs[];

    struct CMconfig {
    	double PL0; 				// pathloss at 1 m distance (*not* in dB)
    	double n;					// pathloss exponent
    	double sigma_s;				// shadowing standard deviation (unused)
    	double Aant;				// antenna loss (always 3 dB)
    	double kappa;				// frequency dependence of the pathloss
		double Lmean;				// mean number of clusters
    	double Lambda;					// inter-cluster arrival rate (clusters *per second*)
		// ray arrival rates (mixed Poisson model parameters, in seconds)
    	double lambda_1, lambda_2, Beta;
    	double Gamma;				// inter-cluster decay constant
    	double k_gamma, gamma_0; 	// intra-cluster decay time constant parameters
    	double sigma_cluster;		// cluster shadowing variance (*not* in dB)
    	double m_0, k_m;			// Nakagami m factor mean
    	double var_m_0, var_k_m;	// Nakagami m factor variance
    	double strong_m_0;			// Nakagami m factor for strong components
    	// parameters for alternative PDP shape
    	double gamma_rise; double gamma_1; double xi;
    };

    /* @brief Known models configuration
     **/
    static const CMconfig CMconfigs[];


protected:

	/** @brief selected channel model
	 **/
	int channelModel;

	/* @brief Selected configuration
	     **/
	    CMconfig cfg;

    // configure cluster threshold
    double tapThreshold; // Only generates taps with at most 10 dB attenuation

    double doShadowing; //activates block shadowing
    double doSmallScaleShadowing; // activates small-scale shadowing

    // channel statistical characterization parameters
    // (should be xml-ized)
    // First environment: Residential LOS
    static const double PL0; // -43.9 dB
    static const double pathloss_exponent;
    static const double meanL;
    static const double Lambda;
    static const double lambda1;
    static const double lambda2;
    static const double Beta;
    static const double Gamma;
    static const double k_gamma;
    static const double gamma_0;
    static const double sigma_cluster; // 2.75 dB

    static const double fc; // mandatory band 3, center frequency, Hz
    static const double BW;  //  mandatory band 3, bandwidth, Hz
    static const double fcMHz; // mandatory band 3, center frequency, MHz

    static const double d0;

    // antenna parameters
    static const double ntx;
    static const double nrx;

    TimeMapping<Linear>* newTxPower;
    ConstMapping* txPower;
    Argument arg;
    MappingIterator* pulsesIter;
    // number of clusters
    double L;
    // block shadowing
    double S;
    // start time of cluster number "cluster"
    simtime_t clusterStart;
    simtime_t gamma_l;
    double Mcluster;
    // cluster integrated energy
    double Omega_l;

    double averagePower; // statistics counter (useful for model validation, should converges towards 1)
    long nbCalls;
    cOutVector averagePowers;
    cOutVector pathlosses;  // outputs computed pathlosses. Allows to compute Eb = Epulse*pathloss for Eb/N0 computations. (N0 is the noise sampled by the receiver)

    /*
     * Generates taps for the considered pulse, with the current channel parameters
     */
    void addEchoes(simtime_t_cref pulseStart);

    /*
     * @brief Computes the pathloss as a function of center frequency and bandwidth given in MHz
     */
    double getPathloss(double fc, double BW, double distance) const;

};

#endif	/* _UWBIRIEEE802154APATHLOSSMODEL_H */

