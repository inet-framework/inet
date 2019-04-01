//
// This class defines a generic PSU main parameters.
//
// The set of parameters are defined as quantity percents.
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

#ifndef MAINPSU_H_
#define MAINPSU_H_

#include "inet/icancloud/EnergySystem/PSU/AbstractPSU.h"

namespace inet {

namespace icancloud {


class MainPSU : public AbstractPSU{

protected:

    string psuState;            // Power supply unit state (on/off)
    double wattage;             // Rated output power in watts
    double psu_20;              // Consumption in watts at 20 percent load
    double psu_50;              // Consumption in watts at 50 percent load
    double psu_100;             // Consumption in watts at 100 percent load

	virtual ~MainPSU();

	/*
	*  Module initialization.
	*/
    virtual void initialize() override;
    

public:

    /*
     *  This method returns the value of the instant power consumed by all components of the node and the
     *  consumption loss by psu
     */
    double getConsumptionLoss() override;

    /*
     *  This method returns the value of the instant power consumed by all components of the node and the
     *  consumption loss by psu
     */
    double getNodeConsumption() override;

    /*
     * This method returns the value of the instant consumption depending on the instant consumption of node components
     */
    double calculateConsumptionLoss(double instantConsumption) override;

};

} // namespace icancloud
} // namespace inet

#endif /* MAINPSU_H_ */
