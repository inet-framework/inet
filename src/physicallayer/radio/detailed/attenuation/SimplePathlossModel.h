#ifndef PATHLOSSMODEL_H_
#define PATHLOSSMODEL_H_

#include <cstdlib>

#include "INETDefs.h"
#include "AnalogueModel.h"
#include "Mapping.h"

class SimplePathlossModel;

/**
 * @brief Mapping that represents a Pathloss-function.
 *
 * SimplePathlossConstMapping is subclassed from SimpleConstMapping for
 * convenience.
 * In this simple pathloss implementation, we assume one attenuation value
 * being constant over the signals duration.
 *
 * @ingroup analogueModels
 * @ingroup mapping
 */
class INET_API SimplePathlossConstMapping : public SimpleConstMapping
{
private:
	/** @brief Assignment operator is not allowed.
	 */
	SimplePathlossConstMapping& operator=(const SimplePathlossConstMapping&);

protected:
	/** @brief The factor dependent on the distance of the transmission.*/
	const double distFactor;
	/** @brief Pointer to the model.*/
	SimplePathlossModel* model;
	/** @brief Is the Signal to attenuate defined over frequency?*/
	bool hasFrequency;

public:
	/**
	 * @brief initializes the PathlossMapping for the passed dimensions,
	 * its model and the distance factor for the signal to attenuate.
	 */
	SimplePathlossConstMapping(const DimensionSet& dimensions,
							   SimplePathlossModel* model,
							   const double distFactor);

	/** @brief Copy constructor.
	 */
	SimplePathlossConstMapping(const SimplePathlossConstMapping&);
	/**
	 * @brief Calculates attenuation from the distance factor and
	 * the current positions frequency (or the models carrier frequency
	 * if we are using using signals without frequency domain.
	 */
	virtual double getValue(const Argument& pos) const;

	/**
	 * @brief creates a clone of this mapping. This method has to be implemented
	 * by every subclass. But most time the implementation will look like the
	 * implementation of this method (except of the class name).
	 */
	ConstMapping* constClone() const
	{
		return new SimplePathlossConstMapping(*this);
	}

};


/**
 * @brief Basic implementation of a SimplePathlossModel that uses
 * SimplePathlossConstMapping (that is subclassed from SimpleConstMapping) as
 * attenuation-Mapping.
 *
 * An example config.xml for this AnalogueModel can be the following:
 * @verbatim
	<AnalogueModel type="SimplePathlossModel">
		<!-- Environment parameter of the pathloss formula
			 If ommited default value is 3.5-->
		<parameter name="alpha" type="double" value="3.5"/>

		<!-- Carrier frequency of the signal in Hz
			 If ommited the carrier frequency from the
			 connection manager is taken if available
			 otherwise set to default frequency of 2.412e+9-->
		<parameter name="carrierFrequency" type="double" value="2.412e+9"/>
	</AnalogueModel>
   @endverbatim
 *
 * @ingroup analogueModels
 */
class INET_API SimplePathlossModel : public AnalogueModel
{
protected:
	friend class SimplePathlossConstMapping;

	/** @brief Path loss coefficient. **/
	double pathLossAlphaHalf;

	/** @brief carrier frequency needed for calculation */
	double carrierFrequency;

	/** @brief Information needed about the playground */
	bool useTorus;

	/** @brief The size of the playground.*/
	Coord playgroundSize;

public:
	/**
	 * @brief Initializes the analogue model. playgroundSize
	 * need to be valid as long as this instance exists.
	 *
	 * The constructor needs some specific knowledge in order to create
	 * its mapping properly:
	 *
	 * @param alpha the coefficient alpha (specified e.g. in config.xml and
	 * 				passed in constructor call)
	 * @param carrierFrequency the carrier frequency
	 * @param useTorus information about the playground the host is moving in
	 * @param playgroundSize information about the playground the host is
	 * 						 moving in
	 */
	SimplePathlossModel()
	    : pathLossAlphaHalf(0)
	    , carrierFrequency(0)
	    , useTorus(false)
	    , playgroundSize()
	{ }

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

	/**
	 * @brief Method to calculate the attenuation value for pathloss.
	 *
	 * Functionality is similar to pathloss-calculation in BasicSnrEval from
	 * Mobility-frame work.
	 *
	 * Note: This method is not used directly anymore. Instead the actual
	 * pathloss is calculated in SimplePathlossConstMappings "getValue()"
	 * method.
	 */
	virtual double calcPathloss(const Coord& receiverPos, const Coord& sendersPos);
};

#endif /*PATHLOSSMODEL_H_*/
