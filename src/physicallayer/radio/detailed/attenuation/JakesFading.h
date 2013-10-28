//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef JAKESFADING_H_
#define JAKESFADING_H_

#include "INETDefs.h"
#include "AnalogueModel.h"
#include "Mapping.h"

class JakesFading;

/**
 * @brief Mapping used to represent attenuation of a signal by JakesFading.
 *
 * @ingroup analogueModels
 * @ingroup mapping
 */
class INET_API JakesFadingMapping: public SimpleConstMapping {
private:
	/** @brief Assignment operator is not allowed.
	 */
	JakesFadingMapping& operator=(const JakesFadingMapping&);

protected:
	static DimensionSet dimensions;

	/** @brief Pointer to the model.*/
	JakesFading* model;

	/** @brief The relative speed between the two hosts for this attenuation.*/
	double relSpeed;

public:
	/**
	 * @brief Takes the model, the relative speed between two hosts and
	 * the interval in which to create key entries.
	 */
	JakesFadingMapping(JakesFading* model, double relSpeed,
					   const Argument& start,
					   const Argument& interval,
					   const Argument& end)
		: SimpleConstMapping(dimensions, start, end, interval)
		, model(model)
		, relSpeed(relSpeed)
	{}

	JakesFadingMapping(const JakesFadingMapping& o)
		: SimpleConstMapping(o)
		, model(o.model)
		, relSpeed(o.relSpeed)
	{}

	virtual ~JakesFadingMapping() {}

	virtual double getValue(const Argument& pos) const;

	/**
	 * @brief creates a clone of this mapping.
	 *
	 * This method has to be implemented by every subclass.
	 * But most time the implementation will look like the
	 * implementation of this method (except of the class name).
	 */
	ConstMapping* constClone() const
	{
		return new JakesFadingMapping(*this);
	}
};

/**
 * @brief Implements Rayleigh fading after Jakes' model.
 *
 * An example config.xml for this AnalogueModel can be the following:
 * @verbatim
	<AnalogueModel type="JakesFading">
		<!-- Carrier frequency of the signal in Hz
			 If ommited the carrier frequency from the
			 connection manager is taken if available
			 otherwise set to default frequency of 2.412e+9-->
		<parameter name="carrierFrequency" type="double" value="2.412e+9"/>

		<!-- Number of fading paths per host -->
		<parameter name="fadingPaths" type="long" value="3"/>

		<!-- f-selectivity: mean delay spread in seconds -->
		<parameter name="delayRMS" type="double" value="0.0001"/>

		<!-- Interval in which to define attenuation for in seconds -->
		<parameter name="interval" type="double" value="0.001"/>
	</AnalogueModel>
   @endverbatim
 *
 * @ingroup analogueModels
 * @author Hermann S. Lichte, Karl Wessel (port for MiXiM)
 */
class INET_API JakesFading: public AnalogueModel {
private:
	/** @brief Copy constructor is not allowed.
	 */
	JakesFading(const JakesFading&);
	/** @brief Assignment operator is not allowed.
	 */
	JakesFading& operator=(const JakesFading&);

protected:
	friend class JakesFadingMapping;

	/** @brief Number of fading paths used. */
	int fadingPaths;

	/**
	 * @brief Angle of arrival on a fading path used for Doppler shift calculation.
	 **/
	double* angleOfArrival;

	/** @brief Delay on a fading path. */
	simtime_t* delay;

	/** @brief Carrier frequency to be used. */
	double carrierFrequency;

	/** @brief The interval to set attenuation entries in. */
	Argument interval;

public:
	/**
	 * @brief Default constructor for the model, the initialization will be done in initFromMap.
	 */
	JakesFading();

	/** @brief Initialize the analog model from XML map data.
	 *
	 * This method should be defined for generic analog model initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap&);

	virtual ~JakesFading();

	virtual void filterSignal(DetailedRadioFrame*, const Coord&, const Coord&);
};

#endif /* JAKESFADING_H_ */
