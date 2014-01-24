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

#ifndef LOGNORMALSHADOWING_H_
#define LOGNORMALSHADOWING_H_

#include "INETDefs.h"
#include "AnalogueModel.h"

/**
 * @brief Channel state implementing log-normal shadowing.
 *
 * An example config.xml for this AnalogueModel can be the following:
 * @verbatim
	<AnalogueModel type="LogNormalShadowing">
		<!-- Mean attenuation in dB -->
		<parameter name="mean" type="double" value="0.5"/>

		<!-- Standart deviation of the attenuation in dB -->
		<parameter name="stdDev" type="double" value="0.25"/>

		<!-- Interval in which to define attenuation for in seconds -->
		<parameter name="interval" type="double" value="0.001"/>
	</AnalogueModel>
   @endverbatim
 *
 * @ingroup analogueModels
 * @author Hermann S. Lichte, Karl Wessel (port for MiXiM)
 * @date 2007-08-15
 **/
class INET_API LogNormalShadowing: public AnalogueModel {
protected:
	/** @brief Mean of the random attenuation in dB */
	double mean;

	/** @brief Standart deviation of the random attenuation in dB */
	double stdDev;

	/** @brief The interval to set attenuation entries in. */
	simtime_t interval;

protected:
	/**
	 * @brief Returns a random log normal distributed gain factor.
	 *
	 * The gain factor is below 1.0 so its an actual attenuation.
	 */
	double randomLogNormalGain() const;

public:
	/**
	 * @brief Takes the mean and standard deviation of the log normal
	 * distributed attenuation values as well as the inteval in which
	 * to define key entries in (accuracy of the model).
	 */
	LogNormalShadowing();

	/** @brief Initialize the analog model from XML map data.
	 *
	 * This method should be defined for generic analog model initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap&);

	virtual ~LogNormalShadowing();

	/**
	 * @brief Calculates shadowing loss based on a normal gaussian function.
	 */
	virtual void filterSignal(DetailedRadioFrame*, const Coord&, const Coord&);
};

#endif /* LOGNORMALSHADOWING_H_ */
