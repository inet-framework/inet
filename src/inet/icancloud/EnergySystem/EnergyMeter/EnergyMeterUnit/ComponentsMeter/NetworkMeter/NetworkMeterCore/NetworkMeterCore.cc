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

#include "NetworkMeterCore.h"

namespace inet {

namespace icancloud {


Define_Module(NetworkMeterCore);


void NetworkMeterCore::initialize(){

    type = "network";
    AbstractMeterUnit::initialize();

 }

NetworkMeterCore::~NetworkMeterCore() {
    AbstractMeterUnit::finish();
}

double NetworkMeterCore::getInstantConsumption(const string &state,int partIndex){

    // Define ..

        int componentsQuantity, i;
        double consumptionValue;
        double instantConsumption;
        string actualState;

    // Initialize ..
        componentsQuantity = e_internal->getNumberOfComponents();
        instantConsumption = 0.0;
        actualState = "";

    // Initialize ..

        if (partIndex == -1){

            for (i= 0; i < componentsQuantity; i++){
                // Get the state
                    actualState = e_internal -> e_getActualState();

                // Get the consumption of the state
                    consumptionValue = e_internal->e_getConsumptionValue(actualState);

                    instantConsumption +=  consumptionValue;
            }
        }
        else {

            // Get the state
            if (strcmp (state.c_str(), NULL_STATE) == 0)
                actualState = e_internal -> e_getActualState(partIndex);
            else
                actualState = state;

            // Get the consumption of the state
                consumptionValue = e_internal->e_getConsumptionValue(actualState, partIndex);

                instantConsumption =  consumptionValue;

        }

        if (instantConsumption != 0) instantConsumption += e_internal->getConsumptionBase();

        return instantConsumption;
}

double NetworkMeterCore::getEnergyConsumed (int partIndex){

    // Define ..
        int componentsQuantity, statesSize, i, j;
        double consumptionValue;
        double timeState;
        string actualState;
        double consumption;
        vector<double> totalConsumption;

    // Initialize
        consumption = AbstractMeterUnit::getEnergyConsumed(partIndex);
        totalConsumption.clear();

    if (consumption == -1){

        // Initialize ..
            componentsQuantity = e_internal->getNumberOfComponents();
            consumption = 0.0;
            timeState = 0.0;
            actualState = "";

        // Initialize ..

            if (partIndex == -1){

                for (i= 0; i < componentsQuantity; i++){
                    // Get the states size
                       statesSize = e_internal -> e_getStatesSize();

                    for (j = 0; j < statesSize ; j++){
                        // Get the consumption of the state
                            timeState = e_internal->e_getStateTime(j).dbl();

                        // Get the consumption of the state
                            consumptionValue = e_internal->e_getConsumptionValue(j);

                            consumption +=  consumptionValue * timeState;

                    }
                    totalConsumption.push_back(consumption);
                    consumption = 0;
                }

                consumption = postEnergyConsumedCalculus(partIndex, totalConsumption);

            }
            else {


             // Get the states size
                 statesSize = e_internal -> e_getStatesSize(partIndex);

              for (j = 0; j < statesSize ; j++){
                  // Get the consumption of the state
                      timeState = e_internal->e_getStateTime(j, partIndex).dbl();

                  // Get the consumption of the state
                      consumptionValue = e_internal->e_getConsumptionValue(j, partIndex);

                      consumption +=  consumptionValue * timeState;

              }
              totalConsumption.push_back(consumption);
              consumption = postEnergyConsumedCalculus(partIndex, totalConsumption);
            }
    }


    return consumption;
}


} // namespace icancloud
} // namespace inet
