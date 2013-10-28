#ifndef ANALOGUEMODEL_
#define ANALOGUEMODEL_

#include "INETDefs.h"
#include "Coord.h"

class DetailedRadioFrame;

/**
 * @brief Interface for the analogue models of the physical layer.
 *
 * An analogue model is a filter responsible for changing
 * the attenuation value of a Signal to simulate things like
 * shadowing, fading, pathloss or obstacles.
 *
 * Note: The Mapping this an AnalogeuModel adds to a signal has
 * to define absolute time positions not relative.
 * Meaning the position zero refers to the simulation start not
 * the signal start.
 *
 * @ingroup analogueModels
 */
class INET_API AnalogueModel {

public:
	/**
	 * @brief Used at initialization to pass the parameters
	 *        to the AnalogueModel and Decider
	 */
	typedef std::map<std::string, cMsgPar> ParameterMap;

public:
	virtual ~AnalogueModel() {}

	/** @brief Initialize the analog model from XML map data.
	 *
	 * This method should be defined for generic analog model initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap&) { return true; }

	/**
	 * @brief Has to be overriden by every implementation.
	 *
	 * Filters a specified AirFrame's Signal by adding an attenuation
	 * over time to the Signal.
	 *
	 * @param frame			The incomming frame.
	 * @param sendersPos	The position of the frame sender.
	 * @param receiverPos	The position of frame receiver.
	 */
	virtual void filterSignal(DetailedRadioFrame* frame, const Coord& sendersPos, const Coord& receiverPos) = 0;
};

#endif /*ANALOGUEMODEL_*/
