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

#include "MainPSU.h"

namespace inet {

namespace icancloud {


Define_Module(MainPSU);

MainPSU::~MainPSU(){

}

void MainPSU::initialize(){

    AbstractPSU::initialize();

	psuState = PSU_OFF;
    wattage = par("wattage").doubleValue();
    psu_20 = par("psu_20").doubleValue();
    psu_50 = par("psu_50").doubleValue();
    psu_100 = par("psu_100").doubleValue();

}

double MainPSU::getConsumptionLoss(){

    // Define ..
        double instantConsumption;

    // Initialize ..
        instantConsumption = 0.0;

    // Begin..
        instantConsumption = getNodeSubsystemsConsumption();

    return calculateConsumptionLoss(instantConsumption);
}

double MainPSU::getNodeConsumption(){

    // Define ..
        double instantConsumption;

    // Initialize ..
        instantConsumption = 0.0;

    // Begin..

        instantConsumption = getNodeSubsystemsConsumption();

        instantConsumption += calculateConsumptionLoss(instantConsumption);

        return instantConsumption;
}

double MainPSU::calculateConsumptionLoss(double instantConsumption){

    double percent;
    double consumptionLoss;

    percent = 0.0;
    consumptionLoss = 0.0;

    percent = (instantConsumption * 100 / wattage);

       if (percent > 100) {
           consumptionLoss = -1;
       } else if ((percent >= 0.0) && (percent < psu_20)){
           consumptionLoss = ((instantConsumption * 100) / psu_20) - instantConsumption;
       } else if ((percent >= psu_20) && (percent < psu_50)){
           consumptionLoss = ((instantConsumption * 100) / psu_50) - instantConsumption;
       } else if ((percent >= psu_50) && (percent <= psu_100)){
           consumptionLoss = ((instantConsumption * 100) / psu_100) - instantConsumption;
       } else {
           throw cRuntimeError("MainPSU::getInstantConsumption error. Calculus are not correct -> percent: %lf",percent);
       }

       if ((consumptionLoss+instantConsumption) > wattage){
           consumptionLoss = -1;
       }

       if (DEBUG_ENERGY) printf("%s MainPSU calculateConsumptionLoss [ %lf ] %lf\n", getParentModule()->getFullName(), instantConsumption, consumptionLoss);
       return consumptionLoss;
}

} // namespace icancloud
} // namespace inet
