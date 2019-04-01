//
// Module that implements an energy state.
//
// A state is defined by an id and the energy consumed when device is performing operations at that state.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

#ifndef ENERGYSTATE_H_
#define ENERGYSTATE_H_

#include <omnetpp.h>
#include <sstream>
#include <vector>
#include <string>
#include <map>

namespace inet {

namespace icancloud {

using std::string;
using std::pair;
using std::vector;


using namespace omnetpp;

class EnergyState {

private:

	string state;         // The id for the state
	simtime_t cTime;      // The amount of time that the component has been acummulated
	double cValue;        // The value of consumption of the component for the state.

public:
	EnergyState();
	virtual ~EnergyState();

	void setState(string newState, double consumptionValue);

	string getState();
	double getConsumptionValue();

	void setStateTime(simtime_t newConsumptionTime);
	simtime_t getStateTime();
	void resetStateTime();

	string toString();
};

} // namespace icancloud
} // namespace inet

#endif /* ENERGYSTATE_H_ */
