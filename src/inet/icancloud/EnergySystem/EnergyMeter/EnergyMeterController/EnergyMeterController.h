/**
 * @class EnergyMeterController EnergyMeterController.h EnergyMeterController.cc
 *
 * This class models a energy controller. This device links each single component controller.
 * From this device it is possible to access to the data of the different energy data consumed by main devices.
 *
 * @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2013-11-07
 */

#ifndef ENERGY_METER_H_
#define ENERGY_METER_H_

#include "inet/icancloud/EnergySystem/PSU/AbstractPSU.h"
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

namespace inet {

namespace icancloud {


class AbstractMeterUnit;
class AbstractPSU;

class EnergyMeterController : public cSimpleModule {

    bool activeEnergyMeter;

    // Components
    AbstractMeterUnit* cpu;
    AbstractMeterUnit* memory;
    AbstractMeterUnit* storage;
    AbstractMeterUnit* network;
    AbstractPSU* psu;

    bool memorization;

public:

	virtual ~EnergyMeterController();

    virtual void initialize() override;
    

	/*
	*  to be initialized by node module..
	*/
	void init();
	void activateMeters();
   /**
	* Module ending.
	*/
	void finish() override;

	double cpuInstantConsumption(string state = NULL_STATE, int partIndex = -1);
    double getCPUEnergyConsumed(int partIndex = -1);
    string getCurrentCPUState (int partIndex = -1){return cpu->getActualState(partIndex);};

    double getMemoryInstantConsumption(string state = NULL_STATE, int partIndex = -1);
    double getMemoryEnergyConsumed(int partIndex);
    string getCurrentMemoryState (int partIndex = -1){return memory->getActualState(partIndex);};

    double getNICInstantConsumption(string state = NULL_STATE, int partIndex = -1);
    double getNICEnergyConsumed(int partIndex = -1);
    string getCurrentNICState (int partIndex = -1){return network->getActualState(partIndex);};

    double getStorageInstantConsumption(string state = NULL_STATE, int partIndex = -1);
    double getStorageEnergyConsumed(int partIndex = -1);
    string getCurrentStorageState (int partIndex = -1){return storage->getActualState(partIndex);};

	double getPSUConsumptionLoss();
	double getPSUEnergyLoss();
	double getNodeInstantConsumption();
    double getNodeEnergyConsumed();
    double getNodeSubsystemsConsumption();

    string getCPUName(){return cpu->getComponentName();};
    string getMemoryName(){return memory->getComponentName();};
    string getStorageName(){return network->getComponentName();};
    string getNetworkName(){return storage->getComponentName();};

    void resetNodeEnergy (){psu->resetNodeEnergyConsumed();};
   /*
    * This method activates an alarm for getting the consumption each 'scale' interval
    * The consumption will be accumulated at nodeEnergyConsumed
    */
   void switchOnPSU ();

   /*
   * This method deactivates the alarm.
   */
   void switchOffPSU ();

   void registerMemorization(bool memo);
   void loadMemo(MemoSupport* c,MemoSupport* m,MemoSupport* s,MemoSupport* n);

};

} // namespace icancloud
} // namespace inet

#endif /* ENERGYMETER_H_ */


