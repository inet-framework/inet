//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_CONGESTIONCONTROLFACTORY_H_
#define INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_CONGESTIONCONTROLFACTORY_H_

#include "ICongestionController.h"
#include "../Statistics.h"

namespace inet {
namespace quic {

class CongestionControlFactory {
public:
    ~CongestionControlFactory() { }

    static CongestionControlFactory *getInstance() {
        static CongestionControlFactory instance;
        return &instance;
    }

    ICongestionController *createCongestionController(const std::string &name);

private:
    CongestionControlFactory() { }

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONGESTIONCONTROL_CONGESTIONCONTROLFACTORY_H_ */
