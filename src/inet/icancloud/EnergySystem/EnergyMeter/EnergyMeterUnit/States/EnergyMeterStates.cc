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

#include "inet/icancloud/EnergySystem/EnergyMeter/EnergyMeterUnit/States/EnergyMeterStates.h"

namespace inet {

namespace icancloud {


EnergyMeterStates::EnergyMeterStates() {
	// TODO Auto-generated constructor stub
	states.clear();
	lastEvent = 0.0;
	actualState = "";
	consumptionBase = 0.0;
	memorization = false;
	nonAccumulatedTimeStates = nullptr;
	accumulatedConsumption = 0.0;
	actualStatePos = 0;
}

EnergyMeterStates::~EnergyMeterStates() {
	// TODO Auto-generated destructor stub
}

void EnergyMeterStates::setLastEvent (simtime_t event){
	lastEvent = event;
}

void EnergyMeterStates::setNewState (string newState,  double consumption){
	EnergyState* newEnergyState;
	vector<EnergyState*>::iterator stateIt;
	std::ostringstream msgLine;
	bool found;
	string getstate;

	found = false;

	for (stateIt = states.begin() ; stateIt < states.end() ; stateIt++){
		getstate = (*stateIt)->getState();
		if (strcmp (getstate.c_str(), newState.c_str()) == 0 ){ // it exists another state with the same reference (newState!)
			found = true;
			break;
		}
	}

	if (!found){
		newEnergyState = new EnergyState();
		newEnergyState->setState(newState, consumption);
		states.insert(states.end(), newEnergyState);
	} else {
		msgLine << "The state ["<< newState <<"] is repeated. Aborting simulation..";
		throw cRuntimeError(msgLine.str().c_str());
	}

}

void EnergyMeterStates::initActualState(string actual_state){
	actualState = actual_state;

	int counter;
	bool found = false;

	for (counter = 0; (counter < (int)states.size()) && (!found); counter++){
	    if (strcmp ((*(states.begin() + counter))->getState().c_str() , actualState.c_str()) == 0 )
	        found = true;;
	}

	actualStatePos = counter;
	lastEvent = simTime();

}

void EnergyMeterStates::changeState (string state, simtime_t actualSimtime){
    simtime_t aux = actualSimtime;
    if (memorization){
//        double entera = actualSimtime.trunc(0).dbl();
//        double decimal = (actualSimtime - entera).trunc(-1).dbl();
//        if ( (decimal < 0.3 ) ){
//           aux = entera;
//        }
//        else if ((decimal > 0.7)){
//           aux = entera + 1;
//
//        }
//        else {
//           aux = entera + 0.5;
//        }
        aux = actualSimtime.trunc(SimTimeUnit(-1));
    }
    else {
        aux = actualSimtime;
    }

	if (actualState.empty()){
		initActualState(state);
		lastEvent = aux;
	}
	else if (strcmp (actualState.c_str() , state.c_str()) != 0 ){

		vector<EnergyState*>::iterator stateIt;
		std::ostringstream msgLine;
		simtime_t time = aux - lastEvent;
		bool found;
		string getstate;
		int counter = 0;
		found = false;

		for (counter = 0; (counter < (int)states.size()) && (!found); counter++){

		    stateIt = (states.begin() + counter);

			getstate = (*stateIt)->getState();
			if (strcmp (getstate.c_str() , state.c_str()) == 0 ){

			    // Change the last time stamp
                if (memorization)
                    nonAccumulatedTimeStates[actualStatePos]+= time.dbl();

			    // Set the new time-event at structure
				(*stateIt)->setStateTime(time);
				actualState = state;
				actualStatePos = counter;
				lastEvent = aux;

				found = true;

				if (memorization)
                    nonAccumulatedTimeStates[counter]+= time.dbl();

			}
		}

		if (!found){
			msgLine << "The state ["<< state <<"] has not been found at the operation of changing state." << std::endl;
			msgLine << "Maybe the component does not support the energy states. Aborting simulation.." << std::endl;
			throw cRuntimeError(msgLine.str().c_str());
		}
	}
}

simtime_t EnergyMeterStates::getStateTime (string state){

    vector<EnergyState*>::iterator stateIt;
    int counter = 0;
	string getstate;
	simtime_t acumulated_time = 0.0;
	bool found = false;

	for (stateIt = states.begin() ; (stateIt < states.end()) && (!found) ; stateIt++){

		getstate = (*stateIt)->getState();
		if (strcmp (getstate.c_str() , state.c_str()) == 0 ){
		    if (memorization)
		        acumulated_time = (*stateIt)->getStateTime();
		    else
		        acumulated_time = nonAccumulatedTimeStates[counter];
			found = true;
		}
		counter++;
	}

	return acumulated_time;
}

simtime_t EnergyMeterStates::getStateTime (int statePosition){

    vector<EnergyState*>::iterator stateIt;
    simtime_t stateTime;
    int statesSize = states.size();

    if (!memorization){
        if ((statePosition < statesSize) || (statePosition >= 0)){
            stateIt = states.begin() + statePosition;
            stateTime = (*stateIt)->getStateTime();

            if (strcmp (actualState.c_str() , (*stateIt)->getState().c_str()) == 0 ){
                stateTime += getActualStateTime (actualState);
            }

        }
    } else {
        stateTime = nonAccumulatedTimeStates[statePosition];
    }
    return stateTime;
}

simtime_t EnergyMeterStates::getActualStateTime (string state){

    simtime_t accumulated_time;

    if (strcmp (actualState.c_str() , state.c_str()) == 0 ){
        accumulated_time += simTime() - lastEvent;
    } else {
        throw cRuntimeError("actual state is different than checked it. Try with method EnergyMeterStates::getStateTime (string state)");
    }

    return accumulated_time;
}

string EnergyMeterStates::getStateName (int statePosition){
	vector<EnergyState*>::iterator stateIt;
	string getstate;
    int statesSize = states.size();

	if ((statePosition < statesSize) || (statePosition >= 0)){
		stateIt = states.begin() + statePosition;
		getstate = (*stateIt)->getState();
	}

	return getstate;
}

string EnergyMeterStates::getActualState(){
    return actualState;
}

double EnergyMeterStates::getConsumptionValue (string state){
    vector<EnergyState*>::iterator stateIt;
    EnergyState* energyState;
    string getstate;
    bool found;

    energyState = nullptr;
    found = false;

    for (stateIt = states.begin() ; (stateIt < states.end()) && (!found) ; stateIt++){

        getstate = (*stateIt)->getState();
        if (strcmp (getstate.c_str() , state.c_str()) == 0 ){
            energyState = (*stateIt);
            found = true;
        }
    }

    if (found) return energyState->getConsumptionValue();
    else throw cRuntimeError("EnergyMeterStates::getConsumptionValue-> state (%s) not exists..", state.c_str());
}

double EnergyMeterStates::getConsumptionValue (int statePosition){
    vector<EnergyState*>::iterator stateIt;
    double consumption = 0;
    int statesSize = states.size();

    if ((statePosition < statesSize) || (statePosition >= 0)){
        stateIt = states.begin() + statePosition;
        consumption = (*stateIt)->getConsumptionValue();
    }
    else
        throw cRuntimeError("EnergyMeterStates::getConsumptionValue-> state number %i not exist ..",statePosition);

    return consumption;
}

int EnergyMeterStates::getStatesSize (){
	return states.size();
}

double EnergyMeterStates::getConsumptionBase(){
    return consumptionBase;
}

string EnergyMeterStates::toString(){
	std::ostringstream info;
	vector<EnergyState*>::iterator stateIt;

	for (stateIt = states.begin() ; stateIt < states.end() ; stateIt++){
		info << (*stateIt)->toString() << std::endl;
	}

	return info.str().c_str();

}

void EnergyMeterStates::loadComponent (EnergyState* state){

    EnergyState* cell;
    string empty = "";
    cell = new EnergyState();
    cell->setState(state->getState(),state->getConsumptionValue());
    states.push_back (cell);

      if (strcmp(actualState.c_str(),empty.c_str()) == 0)
          actualState = (*(states.begin()))->getState();
}

void EnergyMeterStates::resetMemorizationVector (){
    memset (nonAccumulatedTimeStates, 0, (int)states.size()*sizeof(float));
}

void EnergyMeterStates::resetMeterStatesVector (){
    vector<EnergyState*>::iterator stateIt;
    simtime_t gettime;
    int i;
    int statesSize = states.size();

    if (!actualState.empty())
        lastEvent = simTime();


    for (i = 0; i < statesSize; i++){
        stateIt = states.begin() + i;
        (*stateIt)->resetStateTime();
    }

}

void EnergyMeterStates::setConsumptionBase(double newValue){
    consumptionBase = newValue;
}

void EnergyMeterStates::activateMemorization(){

    memorization = true;

    // Set the size
        nonAccumulatedTimeStates = new float [(int)states.size()*sizeof(float)];

    // Initialize the array
        memset (nonAccumulatedTimeStates, 0 ,(int)states.size()*sizeof(float));

}

void EnergyMeterStates::activateMemoVector(){
    if (simTime().dbl() != lastEvent.dbl()){
        double t = simTime().trunc(SimTimeUnit(-3)).dbl();
        nonAccumulatedTimeStates[actualStatePos]= t - lastEvent.trunc(SimTimeUnit(-3)).dbl();
        lastEvent = t;
    }
}

float* EnergyMeterStates::getStatesVector(){
    return  nonAccumulatedTimeStates;
}

float EnergyMeterStates::accumulateEnergyConsumed(float value){
  return accumulatedConsumption += value;
}

void EnergyMeterStates::PrintStates(){

    printf("energy states: ");

    for (int m=0; m<(int)states.size(); m++)
        printf("%lf", nonAccumulatedTimeStates[m]);

    printf("\n");
}


} // namespace icancloud
} // namespace inet
