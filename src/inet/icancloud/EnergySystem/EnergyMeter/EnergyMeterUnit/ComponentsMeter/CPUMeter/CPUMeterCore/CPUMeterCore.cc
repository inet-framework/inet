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

#include "CPUMeterCore.h"

namespace inet {

namespace icancloud {



Define_Module(CPUMeterCore);


void CPUMeterCore::initialize(){

    type = "cpu";
    AbstractMeterUnit::initialize();

 }

CPUMeterCore::~CPUMeterCore() {
    AbstractMeterUnit::finish();
}

double CPUMeterCore::getInstantConsumption(const string & state, int partIndex){

    // Define ..
        int componentsQuantity;
        double consumptionValue;
        double accumConsumption;
        string actualState;
        int i;

    // Initialize ..
        componentsQuantity = e_internal->getNumberOfComponents();
        accumConsumption = 0.0;
        actualState = "";

    // Initialize ..

        if (partIndex == -1){

            for (i= 0; i < componentsQuantity; i++){
                // Get the state
                    actualState = e_internal -> e_getActualState(i);

                // Get the consumption of the state
                    consumptionValue = e_internal->e_getConsumptionValue(actualState);
                    accumConsumption +=  consumptionValue;
            }

            if (accumConsumption != 0) accumConsumption+= e_internal->getConsumptionBase();
        }

        else {

            // Get the state
            if (strcmp (state.c_str(), NULL_STATE) == 0)
                actualState = e_internal -> e_getActualState(partIndex);
            else
                actualState = state;

            // Get the consumption of the state
                consumptionValue = e_internal->e_getConsumptionValue(actualState, partIndex);

               accumConsumption =  consumptionValue;

        }

        return accumConsumption;
}


double CPUMeterCore::getEnergyConsumed (int partIndex){

    // Define ..
        double consumption;
        int componentsQuantity, statesSize;
        int i, j;
        double consumptionValue;
        double componentConsumption;
        double timeState;
        vector<double> consumptionVector;

    // Initialize ..
        consumption = AbstractMeterUnit::getEnergyConsumed(partIndex);
        consumptionVector.clear();

    if (consumption == -1){

        // Initialize ..
            componentsQuantity = e_internal->getNumberOfComponents();
            consumption = 0.0;
            componentConsumption = 0.0;
            timeState = 0.0;

        // Initialize ..

            if (partIndex == -1){

                for (i= 0; i < componentsQuantity; i++){
                    // Get the states size
                       statesSize = e_internal -> e_getStatesSize(i);

                    for (j = 0; j < statesSize ; j++){
                        // Get the time at the state
                            timeState = e_internal->e_getStateTime(j).dbl();

                        // Get the consumption of the state
                            consumptionValue = e_internal->e_getConsumptionValue(j);

                        // For the memorization process
                            componentConsumption += consumptionValue * timeState;
                    }

                    consumptionVector.push_back(componentConsumption);
                    componentConsumption = 0;
                }

                consumption = postEnergyConsumedCalculus(partIndex, consumptionVector);
            }

            // The calculus of a concrete component do not include the consumption base.
            else{

             // Get the states size
                 statesSize = e_internal -> e_getStatesSize(partIndex);

                  for (j = 0; j < statesSize ; j++){
                      // Get the consumption of the state
                          timeState = e_internal->e_getStateTime(j, partIndex).dbl();

                      // Get the consumption of the state
                          consumptionValue = e_internal->e_getConsumptionValue(j, partIndex);
                          consumption +=  consumptionValue * timeState;
                  }

            }
    }

    return consumption;

}


} // namespace icancloud
} // namespace inet
