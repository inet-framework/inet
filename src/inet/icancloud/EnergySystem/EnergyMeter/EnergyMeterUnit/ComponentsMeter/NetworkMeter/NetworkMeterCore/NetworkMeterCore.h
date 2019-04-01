//
// Module that implements a network interface card (NIC) meter core
//
// NIC meter core is responsible for performing energy measurements.
// The core supply operations to get NIC energy consumed (Joules) and NIC power consumption(Watts)
// 
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

#ifndef NETWORK_METER_CORE_H_
#define NETWORK_METER_CORE_H_

#include "inet/icancloud/EnergySystem/EnergyMeter/EnergyMeterUnit/AbstractMeterUnit.h"

namespace inet {

namespace icancloud {


class NetworkMeterCore : public AbstractMeterUnit {

public:

    virtual void initialize() override;
    

    virtual ~NetworkMeterCore();

    // This method returns the value of the instant power consumed by CPU
    double getInstantConsumption(const string &state = NULL_STATE, int partIndex = -1) override;

    // This method returns the quantity of jules that
    double getEnergyConsumed(int partIndex = -1) override;

};

} // namespace icancloud
} // namespace inet

#endif /* NETWORK_CONSUMPTION_CALCULATOR_H_ */




