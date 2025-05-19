//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_STATISTICS_H_
#define INET_APPLICATIONS_QUIC_STATISTICS_H_

#include <omnetpp/cmodule.h>
#include <omnetpp/cproperties.h>

using namespace omnetpp;

namespace inet {
namespace quic {

class Statistics {
public:
    Statistics(cModule *module, std::string statEntryNamePostfix);
    Statistics(Statistics *other, std::string addStatEntryNamePostfix);
    virtual ~Statistics();

    cModule *getMod() {
        return module;
    }
    simsignal_t createStatisticEntry(std::string name, std::string templateName);
    simsignal_t createStatisticEntry(std::string name);

private:
    cModule *module;
    std::string statEntryNamePostfix;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_STATISTICS_H_ */
