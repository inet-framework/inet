//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_TRANSPORTPARAMETER_H_
#define INET_APPLICATIONS_QUIC_TRANSPORTPARAMETER_H_

#include "Quic.h"

namespace inet {
namespace quic {

class TransportParameters {
public:
    TransportParameters() {}
    virtual ~TransportParameters() {}

    void readParameters(Quic *quicSimpleMod);
    void readExtension(Ptr<const TransportParametersExtension> transportParametersExt);

    omnetpp::simtime_t maxAckDelay = omnetpp::SimTime(25, omnetpp::SimTimeUnit::SIMTIME_MS); //TODO make it as parameter in ini
    int ackDelayExponent = 3;

    uint64_t initialMaxData = 0;
    uint64_t initialMaxStreamData = 0;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_TRANSPORTPARAMETER_H_ */
