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

#include "inet/icancloud/EnergySystem/EnergyMeter/EnergyMeterUnit/AbstractMeterUnit.h"

namespace inet {

namespace icancloud {


AbstractMeterUnit::~AbstractMeterUnit() {

}

void AbstractMeterUnit::initialize(){

    EnergyState* state;
    double value = 0.0;
    string name;

    componentName = par("componentName").stringValue();
    type = getParentModule()->par("componentType").stringValue();

    // Get all the states
    consumptionBase = 0.0;
    consumptionBase = getParentModule()->par("consumptionBase").doubleValue();
    numEnergyStates = getParentModule()->par("numEnergyStates").intValue();

    states.clear();

    for (int i = 0; i < numEnergyStates; i++){
        state = new EnergyState();
        name = getParentModule()->getSubmodule("state",i)->par("stateName").stringValue();
        value = getParentModule()->getSubmodule("state",i)->par("value").doubleValue();
        state->setState(name,value);
        states.push_back(state);
    }

    lastEnergyRead = 0.0;
    memorization = false;

}

void AbstractMeterUnit::finish(){}

void AbstractMeterUnit::resetConsumptionHistory(){
    e_internal->resetAllHistory();
}

double AbstractMeterUnit::getEnergyConsumed(int partIndex)
{
    double energy = -1;
    double *r ;
    float* statesVector;

    if (memorization)
    {
        ostringstream data;
        int bytesEnergyStates ;

        bytesEnergyStates  = numEnergyStates * sizeof(float) ;

        if (partIndex != -1)
        {
            statesVector = e_internal->getStatesVector(partIndex);
            r = memo->find(bytesEnergyStates, statesVector) ;
        }
        else
        {
            int NumberOfComponents = e_internal->getNumberOfComponents() ;
            int bytesEnergyStates  = numEnergyStates * sizeof(float) ;

            r = memo->find(NumberOfComponents * bytesEnergyStates, e_internal->getTotalVector()) ;

            if (r != nullptr){

                energy = 0;

                for (int i = 0; i < NumberOfComponents; i++){
                    r = memo->find(bytesEnergyStates, e_internal->getStatesVector(i)) ;

                    // DEBUG //
                    printStateVector("FIND-ENTRY",numEnergyStates, e_internal->getStatesVector(i), *r);
                    // DEBUG //
                    energy += e_internal->accumulateConsumption(*r, i) ;
                }

                e_internal->resetTotalVector();
            }
        }
    }

    return  energy;
}

double AbstractMeterUnit::postEnergyConsumedCalculus (int partIndex, vector<double> consumption)
{
    float consumptionTotal = 0;

     if (partIndex == -1)
         {
                 // Next lines add the consumption base to the consumption calculated from each component.
         float time = (simTime() - lastEnergyRead).dbl();
         float consum = e_internal->getConsumptionBase() * time;
         double *r ;

                 lastEnergyRead = simTime();

                 if (memorization)
                 {
                     int NumberOfComponents = e_internal->getNumberOfComponents() ;
                     int bytesEnergyStates  = numEnergyStates * sizeof(float) ;
                     bool memoAdd = false;

                     float *statesVectors = (float*)malloc(NumberOfComponents * bytesEnergyStates) ;

                     if (nullptr == statesVectors)
                         cerr << "ERROR in malloc at AbstractMeterUnit::postEnergyConsumedCalculus" ;

                     for (int i = 0; i < NumberOfComponents; i++)
                     {

                         // get the states vector
                         float *statesVector = e_internal->getStatesVector(i);

                         // Memoization...
                         r = memo->find(bytesEnergyStates, statesVector) ;

                         if (nullptr == r){
                               printStateVector("ADD",numEnergyStates, statesVector, consumption[i]);
                               memo->add(bytesEnergyStates, statesVector, consumption[i]) ;
                               memoAdd = true;
                         }
                         else{
                             printStateVector("FIND",numEnergyStates, statesVector, *r);
                         }

                         // Add the stateVector to the stateVectors...
                         memcpy(statesVectors + (i * numEnergyStates), statesVector, bytesEnergyStates) ;

                         // reset del vector de estados
                         e_internal->accumulateConsumption(consumption[i], i);
                     }

                     if (consumptionTotal != 0)
                         consumptionTotal = e_internal->accumulateConsumption(consum, -1);

                     if ((memoAdd) && (NumberOfComponents > 1)){
                         printStateVector("ADD",NumberOfComponents * numEnergyStates, statesVectors, consumptionTotal );
                         memo->add(NumberOfComponents * bytesEnergyStates, statesVectors, consumptionTotal) ;
                     }else {
                         printStateVector("FIND",NumberOfComponents * numEnergyStates, statesVectors, consumptionTotal);
                     }

                     free(statesVectors) ;
                 }
                 // If not memorization and it is required to calculate the entire componentes tu y
                 else {

                     // Add the component consumption
                     for (int i = 0; i < (int)consumption.size(); i++){
                         consumptionTotal += consumption[i];
                     }

                     if (consumptionTotal != 0)
                         consumptionTotal += consum * time;
                 }
         }
     else
     {
         consumptionTotal =consumption[partIndex];

         if (memorization) {
             float *statesVector = e_internal->getStatesVector(partIndex);
             memo->add(numEnergyStates * sizeof(float), statesVector, consumptionTotal) ;
         }

         e_internal->accumulateConsumption(consumptionTotal,partIndex);
     }

     return consumptionTotal;
}

void AbstractMeterUnit::activateMeter(unsigned componentIndex, bool heterogeneousComponents){

    cModule* mod;
    cModule* psuMod;
    AbstractPSU* psu;

    // Create the memory consumption calculator
    try{

        if (strcmp(type.c_str(), "cpu") == 0){
            mod = getParentModule()->getParentModule()->getParentModule()->getSubmodule("cpuModule")->getSubmodule("eController");
        }
        else if (strcmp(type.c_str(), "memory") == 0){
            mod = getParentModule()->getParentModule()->getParentModule()->getSubmodule("memory");
        }
        else if (strcmp(type.c_str(), "storage") == 0){
            mod = getParentModule()->getParentModule()->getParentModule()->getSubmodule("storageSystem",0)->getSubmodule("eController");
        }
        else if (strcmp(type.c_str(), "network") == 0){
           mod = getParentModule()->getParentModule()->getParentModule()->getSubmodule("hypervisor")->getSubmodule("networkService");
        }
        else {
            throw cRuntimeError ("AbstractMeterUnit::activateMeter --> unknown component type.\n");
        }

        e_internal = dynamic_cast <HWEnergyInterface*>  (mod);

        e_internal->loadComponent(states, consumptionBase, memorization, componentIndex, heterogeneousComponents);

        psuMod = getParentModule()->getParentModule()->getParentModule()->getSubmodule("psu");
        psu = dynamic_cast <AbstractPSU*> (psuMod);
        psu -> registerComponent(this);

    }catch (std::exception& e){
        throw cRuntimeError(" AbstractMeterUnit::activateMeter: cannot get an initialization of the %s component type", type.c_str());
    }

}

void AbstractMeterUnit::resetAccumulatedEnergy(int componentIndex){
    if (componentIndex != -1)
        e_internal->resetComponentHistory(componentIndex);
    else
        e_internal->resetAllHistory();
}

double AbstractMeterUnit::getEnergyAccumulated_and_reset(int componentIndex) {
    double accumEnergy = getEnergyConsumed(componentIndex);
    resetAccumulatedEnergy (componentIndex);

    return accumEnergy;
}

string AbstractMeterUnit::getActualState(int partIndex){
    string actualState="";

    if (partIndex != -1)
        actualState = e_internal->e_getActualState(partIndex);
    else
        actualState = e_internal->e_getActualState();

    return actualState;
}

void AbstractMeterUnit::printStateVector(string operation, int size, float * statesVectors, float energy){

    ostringstream st;
    st <<  "guasabi3 "<< operation << " - "<< simTime().dbl() << " - " << getParentModule()->getFullName() << " - " << getParentModule()->getParentModule()->getParentModule()->getFullName() << " - " ;

    for (int m=0; m<size; m++)
        st <<  statesVectors[m] << "-" ;

    st << "energy: "<< energy << std::endl;

    printf ("%s\n",st.str().c_str());

}

} // namespace icancloud
} // namespace inet
