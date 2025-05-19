//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "CongestionControlFactory.h"

#include "NoCongestionController.h"
#include "NewRenoCongestionController.h"

namespace inet {
namespace quic {

ICongestionController *CongestionControlFactory::createCongestionController(const std::string &name)
{
    if (name == "No") {
        EV_DEBUG << "CongestionControlFactory: create NoCongestionController" << endl;
        return new NoCongestionController();
    } else if (name == "NewReno") {
        EV_DEBUG << "CongestionControlFactory: create NewRenoCongestionController" << endl;
        return new NewRenoCongestionController();
    } else {
        throw cRuntimeError("Unknown congestion controller name");
    }
}

} /* namespace quic */
} /* namespace inet */
