//
// This abstract class is the main base that each hardware component inherits from.
// It supply all the methods to control the different energy states for the component.
//
// @author Gabriel Gonz&aacute;lez Casta;&ntilde;&eacute;
// @date 2012-23-11

#ifndef _HWENERGYINTERFACE_H_
#define _HWENERGYINTERFACE_H_

#include <omnetpp.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdexcept>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include <sstream>
#include <vector>
#include <string>
#include "inet/icancloud/EnergySystem/EnergyMeter/EnergyMeterUnit/States/EnergyMeterStates.h"
#include "inet/icancloud/Base/icancloud_Base.h"

namespace inet {

namespace icancloud {

using std::string;
using std::pair;
using std::vector;

class HWEnergyInterface : public icancloud_Base{

    /** Vector with the states and the times of execution to calculate the energy*/
    vector<EnergyMeterStates*> meterStates;
    vector<string> actualState;
    int numberOfComponents;

    /** Memorization parameters */
    bool memorization;

protected:

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    
    virtual void finish() override;

    HWEnergyInterface();

    virtual ~HWEnergyInterface();

    /**
     * Get the out Gate to the module that sent <b>msg</b>.
     * @param msg Arrived message.
     * @return Gate (out) to module that sent <b>msg</b> or nullptr if gate not found.
     */
     virtual cGate* getOutGate (cMessage *msg) override = 0;

    /**
     * Process a self message.
     * @param msg Self message.
     */
     virtual void processSelfMessage (cMessage *msg) override = 0;

    /**
     * Process a request message.
     * @param sm Request message.
     */
     virtual void processRequestMessage (Packet *) override = 0;

    /**
     * Process a response message.
     * @param sm Request message.
     */
     virtual void processResponseMessage (Packet *) override = 0;


public:

     /*
      * Change the energy state of the memory given by node state
      */
     virtual void changeDeviceState (const string &state,unsigned int componentIndex) = 0;
     virtual void changeDeviceState (const string &state) {changeDeviceState(state,0);}

     /*
      * Change the energy state of the component
      */
     virtual void changeState (const string &energyState,unsigned int componentIndex) = 0;
     virtual void changeState (const string &energyState) {changeState (energyState, 0);}

    /**
     * Change the actual state to the new given state in the instant actualSimtime
     */
    virtual void e_changeState (const string & state, unsigned int componentIndex);
    virtual void e_changeState (const string & state) {e_changeState(state, 0);}

    /*
     * Return the actual state of the component. The parameter gives if there are several components
     */
    virtual string e_getActualState (unsigned int componentIndex = 0);

    /*
     * This method return the size of the states vector.
     */
    virtual int e_getStatesSize (unsigned int componentIndex);

    virtual int e_getStatesSize () {return e_getStatesSize(0);}

    /*
     * Return the simtime associated to a given as parameter state
     */
    virtual simtime_t e_getStateTime (const string & state, unsigned int componentIndex = 0);
    simtime_t e_getStateTime (int statePosition, unsigned int componentIndex = 0);
    virtual simtime_t e_getActualStateTime (const string & state, unsigned int componentIndex = 0);

    /*
     * This method returns the name of the state at position "statePosition" given as parameter
     */
    virtual string e_getStateName (int statePosition,unsigned int componentIndex = 0);

    /*
     * This method returns the consumption value associated to a given state
     */
    virtual double e_getConsumptionValue (const string & state, unsigned int componentIndex = 0);
    virtual double e_getConsumptionValue (int statePosition,unsigned int componentIndex = 0);

    /*
     * This method returns the number of conponents that conform the hw structure
     */
    virtual int getNumberOfComponents() {return numberOfComponents;};

    /*
     * The method parses the energy data for the component from the omnetpp.ini
     */
    virtual void loadComponent (vector<EnergyState*> states, double consumptionBase, bool activate_memorization, unsigned componentIndex = 0, bool heterogeneousComponents = true);

    /*
     *  This method returns the value attribute
     */
    double getConsumptionBase(unsigned int componentIndex = 0) { return (*(meterStates.begin()+componentIndex))->getConsumptionBase();};

    /*
     * This method reset all the states history
     */
    void resetComponentHistory(unsigned int componentIndex);

    /*
     * This method reset all the states history
     */
    void resetAllHistory();

    /*
     * This method allows to set the number of different "devices" that will be contrelled by this interface.
     */
    virtual void setNumberOfComponents(int num);

    /*
     * Memorization methods
     */
    float* getStatesVector(int componentIndex = 0);
    float* getTotalVector();
    void resetTotalVector();
    float accumulateConsumption(float energyConsumed, int componentIndex = 0);
    float getEnergyAccumulated(int componentIndex){return (*(meterStates.begin() + componentIndex))->getEnergyAccumulated();};

private:
    void resetStatesVector(int componentIndex = 0);
    void resetStatesComponentVector();
    void printStatesVector(int size, float* vector);
    void activateMemoVector(int componentIndex);
};

} // namespace icancloud
} // namespace inet

#endif /* HWENERGYINTERFACE_H_ */
