#ifndef PER_MODEL_H
#define PER_MODEL_H

#include <cassert>

#include "INETDefs.h"
#include "AnalogueModel.h"

/**
 * @brief This class applies a parameterized packet error rate
 * to incoming packets. This allows the user to easily
 * study the robustness of its system to packet loss.
 *
 * @ingroup analogueModels
 *
 * @author Jérôme Rousselot <jerome.rousselot@csem.ch>
 */
class INET_API PERModel : public AnalogueModel {
protected:
	double packetErrorRate;
public:
	/** @brief The PERModel constructor takes as argument the packet error rate to apply (must be between 0 and 1). */
	PERModel() : packetErrorRate(0) { }

	/** @brief Initialize the analog model from XML map data.
	 *
	 * This method should be defined for generic analog model initialization.
	 *
	 * @param params The parameter map which was filled by XML reader.
	 *
	 * @return true if the initialization was successfully.
	 */
	virtual bool initFromMap(const ParameterMap&);

	virtual void filterSignal(DetailedRadioFrame*, const Coord&, const Coord&);

};

#endif
