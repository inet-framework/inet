//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
