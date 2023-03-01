//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.z3;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;

//
// This module provides a gate scheduling algorithm that uses the open source
// z3 SAT solver from Microsoft. In order to be able to use this module, the
// corresponding 'Z3 Gate Scheduling Configurator' feature must be enabled and
// the libz3-dev package must be installed.
//
// This configurator computes the gate schedule by creating SAT variables for
// the transmission start/end times and reception start/end times for each packet
// for each flow in all network interfaces along the path. All communication
// network specific constraints such as transmission duration, propagation time,
// subsequent transmissions along the flow, queueing to avoid reordering packets,
// and taking traffic class priority into consideration are translated into SAT
// constraints among the above variables. All additional user constraints such
// as the maximum end-to-end delay and maximum jitter are also added to the model.
//
// Finally, the SAT solver is used to find a solution that fulfills all constraints,
// and the result values are extracted and configured in the network.
//
// The optimizeSchedule parameter can be used to control whether any solution is
// accepted or it should also be optimal with respect to the total end-to-end delay
// among all packets of all flows in the network.
//
simple Z3GateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
        bool labelAsserts = default(false);
        bool optimizeSchedule = default(true);
        @class(Z3GateScheduleConfigurator);
}
