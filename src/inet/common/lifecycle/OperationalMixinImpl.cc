//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/lifecycle/OperationalMixinImpl.h"

#include "inet/protocolelement/transceiver/base/PacketReceiverBase.h"
#include "inet/protocolelement/transceiver/base/PacketTransmitterBase.h"

#ifdef INET_WITH_QUEUEING
#include "inet/queueing/base/PacketFlowBase.h"
#endif

namespace inet {

template class OperationalMixin<cSimpleModule>;

template class OperationalMixin<PacketProcessorBase>;

#ifdef INET_WITH_QUEUEING
template class OperationalMixin<queueing::PacketFlowBase>;
#endif

} // namespace inet

