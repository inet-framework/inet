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

#include "Statistics.h"

namespace inet {
namespace quic {

Statistics::Statistics(cModule *module, std::string statEntryNamePostfix) {
    this->module = module;
    this->statEntryNamePostfix = statEntryNamePostfix;
}

Statistics::Statistics(Statistics *other, std::string addStatEntryNamePostfix) {
    this->module = other->module;
    this->statEntryNamePostfix = other->statEntryNamePostfix + addStatEntryNamePostfix;

}

Statistics::~Statistics() { }

simsignal_t Statistics::createStatisticEntry(std::string name, std::string templateName) {
    std::string fullName = name + statEntryNamePostfix;
    cProperty *statisticTemplate;
    simsignal_t signalId;

    signalId = module->registerSignal(fullName.c_str());
    statisticTemplate = module->getProperties()->get("statisticTemplate", templateName.c_str());

    getEnvir()->addResultRecorders(module, signalId, fullName.c_str(), statisticTemplate);

    return signalId;
}

simsignal_t Statistics::createStatisticEntry(std::string name) {
    return createStatisticEntry(name, name);
}

} /* namespace quic */
} /* namespace inet */
