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

#include "MemoryMeterCore.h"

namespace inet {

namespace icancloud {


Define_Module(MemoryMeterCore);

void MemoryMeterCore::initialize(){

    type = "memory";
    numModules = par ("numModules");
    numDRAMChips = par ("numDRAMChips");
    AbstractMeterUnit::initialize();
}

MemoryMeterCore::~MemoryMeterCore() {

}


double MemoryMeterCore::getInstantConsumption(const string & state, int partIndex){

    // Define ..
          int componentsQuantity, i;
          double consumptionValue;
          double powerMemoryConsumption, powerDimmConsumption;
          string actualState;
          int numRamChipsActived;
          int numRamChipsNotActived;
          RAMmemory* memory;

          memory = check_and_cast<RAMmemory*>(e_internal);
          float percentLoaded = memory->getMemoryOccupation();

      // Initialize ..

          if (percentLoaded == 0.0) {
              numRamChipsActived = 1;
          }
          else{
              numRamChipsActived = ceil((numDRAMChips * percentLoaded) /100);
          }

          numRamChipsNotActived = numDRAMChips - numRamChipsActived;

          consumptionBase = AbstractMeterUnit::getConsumptionBase();
          componentsQuantity = e_internal->getNumberOfComponents();
          powerMemoryConsumption = 0.0;
          powerDimmConsumption = 0.0;
          actualState = "";

      // Initialize ..

          if (partIndex == -1){

              for (i= 0; i < componentsQuantity; i++){
                  // Get the state
                      actualState = e_internal -> e_getActualState();

                  // Get the consumption of the state
                      consumptionValue = e_internal->e_getConsumptionValue(actualState);

                      // Calculate the power consumption of a module (Voltage * current * time * num dram chips)
                      powerDimmConsumption += (consumptionValue * numRamChipsActived);
                      if (powerDimmConsumption != 0) powerDimmConsumption+= (consumptionBase * numRamChipsNotActived);

                      // Calculate the power consumption the set of modules
                      powerMemoryConsumption += powerDimmConsumption * numModules;

                      // Reset for the next operation
                      powerDimmConsumption = 0.0;

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

              // Calculate the power consumption of a module (Voltage * current * time * num dram chips)
                  powerDimmConsumption += (consumptionValue * numRamChipsActived);
                  if (powerDimmConsumption != 0) powerDimmConsumption+= (consumptionBase * numRamChipsNotActived);

              // Calculate the power consumption the set of modules
                  powerMemoryConsumption = powerDimmConsumption * numModules;

               // Reset for the next operation
                  powerDimmConsumption = 0.0;

          }

          if (powerMemoryConsumption != 0.0) powerMemoryConsumption += e_internal->getConsumptionBase();

          return powerMemoryConsumption;
}


double MemoryMeterCore::getEnergyConsumed(int partIndex){

    // Define ..
        double powerMemoryConsumption;
        int componentsQuantity,statesSize;
        double consumptionValue, i, j;
        double timeState, powerDimmConsumption;
        int numRamChipsActived;
        string actualState;
        vector<double> totalConsumption;
        RAMmemory* memory;

        memory = check_and_cast<RAMmemory*>(e_internal);
        float percentLoaded = memory->getMemoryOccupation();

    // Initialize ..
        powerMemoryConsumption = AbstractMeterUnit::getEnergyConsumed(partIndex);
        totalConsumption.clear();
        if (percentLoaded == 0.0) {
            numRamChipsActived = 1;
        }
        else{
            numRamChipsActived = ceil((numDRAMChips * percentLoaded) /100);
        }

    if (powerMemoryConsumption == -1){

        // Initialize ..
            componentsQuantity = e_internal->getNumberOfComponents();
            consumptionValue = 0.0;
            powerMemoryConsumption = 0.0;
            powerDimmConsumption = 0.0;
            timeState = 0.0;
            actualState = "";

           if (partIndex == -1){

                for (i= 0; i < componentsQuantity; i++){
                    // Get the states size
                       statesSize = e_internal -> e_getStatesSize();

                    for (j = 0; j < statesSize ; j++){
                        // Get the consumption of the state
                            timeState = e_internal->e_getStateTime(j).dbl();

                        // Get the consumption of the state
                            consumptionValue = e_internal->e_getConsumptionValue(j);

                        // Calculate the power consumption of a module (Voltage * current * time * num dram chips)
                            powerDimmConsumption += (consumptionValue * timeState * (numRamChipsActived));

                            if (powerDimmConsumption != 0) powerDimmConsumption+= (AbstractMeterUnit::getConsumptionBase() * timeState * (numDRAMChips - numRamChipsActived));

                        // Calculate the power consumption the set of modules
                            powerMemoryConsumption += powerDimmConsumption * numModules;

                        // Reset for the next operation
                            powerDimmConsumption = 0.0;

                    }

                    if (powerMemoryConsumption != 0.0) powerMemoryConsumption += e_internal->getConsumptionBase();

                    // reset the value
                    totalConsumption.push_back(powerMemoryConsumption);
                    powerMemoryConsumption = 0.0;

                }
                powerMemoryConsumption = postEnergyConsumedCalculus(partIndex, totalConsumption);
            }
            else{

             // Get the states size
                 statesSize = e_internal -> e_getStatesSize(partIndex);

              for (j = 0; j < statesSize ; j++){
                  // Get the consumption of the state
                      timeState = e_internal->e_getStateTime(j, partIndex).dbl();

                  // Get the consumption of the state
                      consumptionValue = e_internal->e_getConsumptionValue(j, partIndex);

                  // Calculate the power consumption of a module (Voltage * current * time * num dram chips)
                      powerDimmConsumption += (consumptionValue * timeState * (numRamChipsActived));
                      if (powerDimmConsumption != 0) powerDimmConsumption+= (AbstractMeterUnit::getConsumptionBase() * timeState * (numDRAMChips - numRamChipsActived));

                  // Calculate the power consumption the set of modules
                      powerMemoryConsumption = powerDimmConsumption * numModules;

                  // Reset for the next operation
                      powerDimmConsumption = 0.0;

              }

              totalConsumption.push_back(powerMemoryConsumption);
              powerMemoryConsumption = postEnergyConsumedCalculus(partIndex, totalConsumption);

            }
       }

       return powerMemoryConsumption;
}

} // namespace icancloud
} // namespace inet
