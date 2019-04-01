//
// Module that implements a cpu meter core
//
// CPU meter core is responsible for performing energy measurements.
// The core supply operations to get cpu energy consumed (Joules) and cpu power consumption(Watts)
// 
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//


#ifndef CPUMETERCORE_H_
#define CPUMETERCORE_H_

#include "inet/icancloud/Architecture/NodeComponents/Hardware/CPUs/CPUController/CPUController.h"
#include "inet/icancloud/EnergySystem/EnergyMeter/EnergyMeterUnit/AbstractMeterUnit.h"

namespace inet {

namespace icancloud {


class CPUMeterCore : public AbstractMeterUnit {

public:

    virtual void initialize() override;
    

	virtual ~CPUMeterCore();

    // This method returns the value of the instant power consumed by CPU
    double getInstantConsumption(const string & state, int partIndex) override;

    double getInstantConsumption() override {return getInstantConsumption(NULL_STATE, -1);}

    double getInstantConsumption(const string & state) override {return getInstantConsumption(state, -1);}

    // This method returns the quantity of jules that
    double getEnergyConsumed(int partIndex) override;

    double getEnergyConsumed() override {return getEnergyConsumed(-1);}

};

} // namespace icancloud
} // namespace inet

#endif /* CPUMETERCORE_H_ */




