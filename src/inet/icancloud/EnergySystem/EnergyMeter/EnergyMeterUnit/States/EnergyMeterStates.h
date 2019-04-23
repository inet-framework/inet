//
// Class that implements a set of energy states and its controlling.
//
// This set also controls the actual state.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

#ifndef ENERGYMETERSTATES_H_
#define ENERGYMETERSTATES_H_

#include "EnergyState.h"
#include <omnetpp.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "inet/icancloud/Base/include/icancloud_types.h"

namespace inet {

namespace icancloud {


using std::string;
using std::vector;

class EnergyMeterStates {

	vector <EnergyState*> states;

	simtime_t lastEvent;                            // The time when the last even occurs.
	string actualState;				                // The initial state always is 0
	int actualStatePos;                             // The actual energy state

    double consumptionBase;                         // The consumption base of the components independently of the actual state

    /*
     * Memorization parameters. Memorization is a technique under developement in order to accelerate the calculus of energy measurements.
     */
    bool memorization;                              // If memorization is active or not
    float *nonAccumulatedTimeStates;                // Array to allocate the accumulated time since last read
    float accumulatedConsumption;                   // the quantity of joules consumed by component from the beggining of the simulation until last event.


public:

    /*
     * Constructor
     */
	EnergyMeterStates();

	/*
	 * Destructor
	 */
	virtual ~EnergyMeterStates();

	// To know when is the last record of an energy time event
	void setLastEvent (simtime_t event);

	// To record the states of a component
	void setNewState (string newState, double consumption);

	// These methods returns the amount of time that the component has been at the given state as parameter
	simtime_t getStateTime (string state);
	simtime_t getStateTime (int statePosition);
	simtime_t getActualStateTime (string state);
	string getStateName (int statePosition);

	// Returns the actual state
	string getActualState();

	// These methods returns the consumption value that the component has loaded to the state
	double getConsumptionValue (string state);
	double getConsumptionValue (int statePosition);

	// Initialize the actual state
	void initActualState(string actual_state);

	// To change the actual state
	void changeState (string state, simtime_t actualSimtime);

	// To get the quantity of states
	int getStatesSize ();

    // This method returns the consumption base value
    double getConsumptionBase();

	// to print all the states
	string toString();

	// The method parses the energy data for the component from the omnetpp.ini
	void loadComponent(EnergyState* state);

	/*
	 * This method reset the bytes associated to the float array of memory positions that represents the
	 * energy measured by components
	 */
	void resetMemorizationVector ();
    void resetMeterStatesVector ();

    /*
     * If the component has parts, all of them may share a common consumption
     */
    void setConsumptionBase(double newValue);


    /*
     *  Methods for memorization operations
     */
    // This method activates the memorization structure (Reset the vector and creates it)
    void activateMemorization();

    // This method has to be invoked each time that a meter wants to acces to the memorization vector. This is due to
    // when a read is performed it is needed to set at vector the time from the last checking event until the moment when the
    // read is done (now).
    void activateMemoVector();

    // This method returns the states vector associated to the memorization techniques
    float* getStatesVector();

    // This method accumulates the value given as parameter and returns the total amount of energy wasted by device
    float accumulateEnergyConsumed(float value);

    // This method returns the amount of energy wasted by the device
    float getEnergyAccumulated(){return accumulatedConsumption;};

    // This method prints the set of states (States vector) and the times accumulated at each state by standard output.
    void PrintStates();
};

} // namespace icancloud
} // namespace inet

#endif /* ENERGYSTATESSET_H_ */
