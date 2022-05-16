//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/clock/ClockUserModuleMixinImpl.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/lifecycle/OperationalMixin.h"

#include "inet/queueing/base/ActivePacketSinkBase.h"
#include "inet/queueing/base/ActivePacketSourceBase.h"
#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/queueing/base/PacketGateBase.h"
#include "inet/queueing/base/PacketMeterBase.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/base/PacketPusherBase.h"
#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/queueing/base/PacketServerBase.h"
#include "inet/queueing/base/PassivePacketSinkBase.h"
#include "inet/queueing/base/PassivePacketSourceBase.h"
#include "inet/queueing/base/TokenGeneratorBase.h"

namespace inet {

template class ClockUserModuleMixin<cSimpleModule>;
template class ClockUserModuleMixin<ApplicationBase>;

template class ClockUserModuleMixin<OperationalMixin<queueing::PacketProcessorBase>>;
template class ClockUserModuleMixin<queueing::ActivePacketSinkBase>;
template class ClockUserModuleMixin<queueing::ActivePacketSourceBase>;
template class ClockUserModuleMixin<queueing::PacketClassifierBase>;
template class ClockUserModuleMixin<queueing::PacketFilterBase>;
template class ClockUserModuleMixin<queueing::PacketGateBase>;
template class ClockUserModuleMixin<queueing::PacketMeterBase>;
template class ClockUserModuleMixin<queueing::PacketProcessorBase>;
template class ClockUserModuleMixin<queueing::PacketPusherBase>;
template class ClockUserModuleMixin<queueing::PacketSchedulerBase>;
template class ClockUserModuleMixin<queueing::PacketServerBase>;
template class ClockUserModuleMixin<queueing::PassivePacketSinkBase>;
template class ClockUserModuleMixin<queueing::PassivePacketSourceBase>;
template class ClockUserModuleMixin<queueing::TokenGeneratorBase>;

} // namespace inet

