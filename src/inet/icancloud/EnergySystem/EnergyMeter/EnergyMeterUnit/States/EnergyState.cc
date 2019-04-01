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

#include "EnergyState.h"

namespace inet {

namespace icancloud {


EnergyState::EnergyState() {
	// TODO Auto-generated constructor stub
	state = -1;
	cTime = 0.0;
	cValue = 0.0;
}

EnergyState::~EnergyState() {
}

void EnergyState::setState(string newState, double consumptionValue){
	state = newState;
	cValue = consumptionValue;
}

string EnergyState::getState(){
	return state;
}

double EnergyState::getConsumptionValue(){
    return cValue;
}

void EnergyState::setStateTime(simtime_t newConsumptionTime){
	cTime += newConsumptionTime;
}

simtime_t EnergyState::getStateTime(){
	return cTime;
}

void EnergyState::resetStateTime(){
    cTime = 0.0;
}

string EnergyState::toString(){

	std::ostringstream info;

		info << "State: " << state << " [" << cTime << "]" << std::endl;

	return info.str();
}

} // namespace icancloud
} // namespace inet
