//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/lifecycle/OperationalMixinImpl.h"

#include "inet/protocolelement/transceiver/base/PacketReceiverBase.h"
#include "inet/protocolelement/transceiver/base/PacketTransmitterBase.h"

namespace inet {

template class OperationalMixin<cSimpleModule>;

template class OperationalMixin<PacketProcessorBase>;

} // namespace inet

