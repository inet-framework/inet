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

#include "inet/icancloud/Applications/Base/Management/JobResults.h"

namespace inet {

namespace icancloud {


JobResults::JobResults() {
	value.clear();
	name = "";
}

JobResults::~JobResults() {
    value.clear();
    name = "";
}

string JobResults::getName(){
	return name;
}

void JobResults::setName(string newName){
	name = newName;
}

int JobResults::getValuesSize(){
    return value.size();
}


string JobResults::getValue(int index){
    string result;

    result = (*(value.begin() + index));

    return  result;
}

void JobResults::setValue(string newValue){
    value.push_back(newValue);
}

JobResults* JobResults::dup(){
    JobResults* result;
    result = new JobResults();

    result->setName(name);

    for (int i = 0; i < (int)value.size();i++){
        result->setValue(getValue(i));
    }

    return result;
}

string JobResults::toString(){

	std::ostringstream info;

	info << "[  " << name << " : " ;
	for (int i = 0; i < (int)value.size(); i++) (info << (*(value.begin()+ i))) ;

	info << " ] ";

	return info.str();
}


} // namespace icancloud
} // namespace inet
