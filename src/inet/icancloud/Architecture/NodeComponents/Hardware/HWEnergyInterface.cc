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

#include "inet/icancloud/Architecture/NodeComponents/Hardware/HWEnergyInterface.h"

namespace inet {

namespace icancloud {


HWEnergyInterface::HWEnergyInterface(){

    meterStates.clear();
    numberOfComponents = 1;
    memorization = false;

}

void HWEnergyInterface::initialize(int stage) {

    // Init the superClass
    icancloud_Base::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        EnergyMeterStates* statesSet;

        meterStates.clear();
        statesSet = new EnergyMeterStates();
        meterStates.push_back(statesSet);

        numberOfComponents = 1;
    }
}

void HWEnergyInterface::finish() {
    icancloud_Base::finish();
    while (!meterStates.empty()) {
        delete (meterStates.back());
        meterStates.pop_back();
    }
    meterStates.clear();
}

HWEnergyInterface::~HWEnergyInterface() {
    // TODO Auto-generated destructor stub
}

void HWEnergyInterface::e_changeState (const string & state, unsigned int componentIndex){

    simtime_t actualSimtime = simTime();
    (*(meterStates.begin() + componentIndex)) -> changeState(state, actualSimtime);

}


string HWEnergyInterface::e_getActualState (unsigned int componentIndex){

    return  (*(meterStates.begin() + componentIndex)) -> getActualState();
}


int HWEnergyInterface::e_getStatesSize (unsigned int componentIndex){

    int res = -1;

    if ((*(meterStates.begin() + componentIndex)) != nullptr){

        res = (*(meterStates.begin() + componentIndex)) -> getStatesSize();

    }

    return res;
}


simtime_t HWEnergyInterface::e_getStateTime (const string & state, unsigned int componentIndex){

    simtime_t res;

    if ((*(meterStates.begin() + componentIndex)) != nullptr){

        res = (*(meterStates.begin() + componentIndex)) -> getStateTime(state);

    }
    return res.dbl();

}

simtime_t HWEnergyInterface::e_getStateTime (int statePosition, unsigned int componentIndex){

    simtime_t res;

    if ((*(meterStates.begin() + componentIndex)) != nullptr){

        res = (*(meterStates.begin() + componentIndex)) -> getStateTime(statePosition);

    }
    return res.dbl();

}

simtime_t HWEnergyInterface::e_getActualStateTime (const string & state, unsigned int componentIndex){

    simtime_t res;

    if ((*(meterStates.begin() + componentIndex)) != nullptr){

        res = (*(meterStates.begin() + componentIndex)) -> getActualStateTime(state);

    }
    return res.dbl();

}


string HWEnergyInterface::e_getStateName (int statePosition,unsigned int componentIndex){

    string res;

    if ((*(meterStates.begin() + componentIndex)) != nullptr){

        res = (*(meterStates.begin() + componentIndex)) -> getStateName (statePosition);

    }
    return res;

}

double HWEnergyInterface::e_getConsumptionValue (const string & state, unsigned int componentIndex){
    double res = -1;

    if ((*(meterStates.begin() + componentIndex)) != nullptr){

        res = (*(meterStates.begin() + componentIndex)) -> getConsumptionValue(state);

    }
    return res;
}

double HWEnergyInterface::e_getConsumptionValue (int statePosition,unsigned int componentIndex){
    double res = -1;

    if ((*(meterStates.begin() + componentIndex)) != nullptr){

        res = (*(meterStates.begin() + componentIndex)) -> getConsumptionValue(statePosition);

    }
    return res;
}

void HWEnergyInterface::setNumberOfComponents(int num){

    EnergyMeterStates* statesSet;

    numberOfComponents = num;

    for (int i = 1; i < num; i++){
        statesSet = new EnergyMeterStates();
        meterStates.push_back(statesSet);
    }
}

void HWEnergyInterface::loadComponent (vector<EnergyState*> states, double consumptionBase, bool activate_memorization, unsigned componentIndex, bool heterogeneousComponents){


    int i, j;

    if (heterogeneousComponents){
        for (i = 0; i < numberOfComponents; i++){
            (*(meterStates.begin()+i))->setConsumptionBase(consumptionBase);
            for (j = 0; j < (int)states.size(); j++)
                (*(meterStates.begin()+i))->loadComponent((*(states.begin()+j)));
        }

    } else {
        (*(meterStates.begin()+componentIndex))->setConsumptionBase(consumptionBase);
        for (j = 0; j < (int)states.size(); j++)
            (*(meterStates.begin()+componentIndex))->loadComponent((*(states.begin()+j)));
    }

    // Memorization..

    memorization = activate_memorization;

    if (memorization){
        // Initialize the accumulated consumption
            if (componentIndex == 0){
                for (i = 0; i < numberOfComponents; i++)  (*(meterStates.begin()+i))->activateMemorization();
            }
            else{
                (*(meterStates.begin()+componentIndex))->activateMemorization();
            }

    }
}

void HWEnergyInterface::resetComponentHistory(unsigned int componentIndex){
    (*(meterStates.begin()+componentIndex))->resetMeterStatesVector();
};

void HWEnergyInterface::resetAllHistory(){

    int i;

    for (i = 0; i < numberOfComponents; i++){
        (*(meterStates.begin()+i))->resetMeterStatesVector();
    }
}

float* HWEnergyInterface::getStatesVector(int componentIndex){

    float* states = nullptr;

    if ((componentIndex >= 0) && (componentIndex < (int)meterStates.size()))
        states = (*(meterStates.begin()+componentIndex))->getStatesVector();

    return states;

}

float* HWEnergyInterface::getTotalVector(){
    // All the components should have the same number of energy states
    int statesSize = ((*(meterStates.begin()))) ->getStatesSize();
    int bytesSize;

    float* nonAccumulatedTimeEntire = new float [statesSize*numberOfComponents*sizeof(float)];
    memset(nonAccumulatedTimeEntire,0,statesSize*numberOfComponents*sizeof(float));

    for (int i = 0; i < numberOfComponents; i++){
        activateMemoVector(i);
        bytesSize = statesSize * sizeof(float);
        memcpy(nonAccumulatedTimeEntire + (i * statesSize), ((*(meterStates.begin() + i)) ->getStatesVector()), bytesSize);
    }

    printf("[%lf] - HWEnergyInterface::getTotalVector()", simTime().dbl());
    printStatesVector(statesSize*numberOfComponents, nonAccumulatedTimeEntire);

    return nonAccumulatedTimeEntire;
}

void HWEnergyInterface::resetTotalVector(){

    for (int i = 0; i < numberOfComponents; i++){

        ((*(meterStates.begin() + i))) ->resetMemorizationVector();

    }
}


float HWEnergyInterface::accumulateConsumption(float energyConsumed, int componentIndex){

    float consumption = 0.0;

    if ((componentIndex >= 0) && (componentIndex < (int)meterStates.size())){
        consumption += (*(meterStates.begin()+componentIndex))->accumulateEnergyConsumed(energyConsumed);
        resetStatesVector(componentIndex);
    } else if ((componentIndex == -1) && (componentIndex < (int)meterStates.size())){


//        consumption += (*(meterStates.begin()))->accumulateEnergyConsumed(energyConsumed);
//        for (int i = 0; i < numberOfComponents; i++){
//
//            resetStatesVector(i);
//        }

                float energy = (energyConsumed / numberOfComponents);
                for (int i = 0; i < numberOfComponents; i++){
                    consumption += (*(meterStates.begin()+i))->accumulateEnergyConsumed(energy);
                    resetStatesVector(i);
                }


    }

    return consumption;

}

void HWEnergyInterface::resetStatesVector(int componentIndex){
    if ((componentIndex >= 0) && (componentIndex < (int) meterStates.size())){
        (*(meterStates.begin()+componentIndex))->resetMemorizationVector();
    }
}

void HWEnergyInterface::resetStatesComponentVector(){
    for (int i = 0; i < numberOfComponents; i++){
        (*(meterStates.begin()+i))->resetMemorizationVector();
    }
}

void HWEnergyInterface::activateMemoVector(int componentIndex){
    if ((componentIndex >= 0) && (componentIndex < (int) meterStates.size())){
        (*(meterStates.begin()+componentIndex))->activateMemoVector();
    }
}

void HWEnergyInterface::printStatesVector(int size, float* vector){
    printf ("BEGIN ");
    for (int i = 0; i < size; i++){
        printf (" %f ", vector[i]);
    }
    printf("END\n");
}

} // namespace icancloud
} // namespace inet
