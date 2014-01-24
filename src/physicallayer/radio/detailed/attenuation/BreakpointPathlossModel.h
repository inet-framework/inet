#ifndef BREAKPOINTPATHLOSSMODEL_H_
#define BREAKPOINTPATHLOSSMODEL_H_

#include <cstdlib>

#include "INETDefs.h"
#include "AnalogueModel.h"

/**
 * @brief Basic implementation of a BreakpointPathlossModel.
 * This class can be used to implement the ieee802154 path loss model.
 *
 * @ingroup analogueModels
 */
class INET_API BreakpointPathlossModel : public AnalogueModel
{
protected:

//	/** @brief Model to use for distances below breakpoint distance */
//	SimplePathlossModel closeRangeModel;
//	/** @brief Model to use for distances larger than the breakpoint distance */
//	SimplePathlossModel farRangeModel;

    /** @brief initial path loss in dB */
    double PL01, PL02;
    /** @brief initial path loss */
    double PL01_real, PL02_real;

    /** @brief pathloss exponents */
    double alpha1, alpha2;

    /** @brief Breakpoint distance squared. */
    double breakpointDistance;

    /** @brief Carrier frequency in Hz */
    double carrierFrequency;

    /** @brief Information needed about the playground */
    bool useTorus;

    /** @brief The size of the playground.*/
    Coord playgroundSize;

    /** logs computed pathlosses. */
    cOutVector pathlosses;

public:
	/**
	 * @brief Initializes the analogue model. playgroundSize
	 * need to be valid as long as this instance exists.
	 *
	 * The constructor needs some specific knowledge in order to create
	 * its mapping properly:
	 *
	 * @param alpha the coefficient alpha (specified e.g. in config.xml and passed
	 *    			in constructor call)
	 * @param carrierFrequency the carrier frequency
	 * @param useTorus information about the playground the host is moving in
	 * @param playgroundSize information about the playground the host is moving in
	 */
	BreakpointPathlossModel()
		: AnalogueModel()
		, PL01(0)
		, PL02(0)
		, PL01_real(0)
		, PL02_real(0)
		, alpha1(alpha1)
		, alpha2(alpha2)
		, breakpointDistance(breakpointDistance)
		, carrierFrequency(carrierFrequency)
		, useTorus(useTorus)
		, playgroundSize(playgroundSize)
		, pathlosses()
	{
		pathlosses.setName("pathlosses");
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

	/**
	 * @brief Filters a specified AirFrame's Signal by adding an attenuation
	 * over time to the Signal.
	 */
	virtual void filterSignal(DetailedRadioFrame*, const Coord&, const Coord&);

	virtual bool isActiveAtDestination() { return true; }

	virtual bool isActiveAtOrigin() { return false; }

};

#endif /*BREAKPOINTPATHLOSSMODEL_H_*/
