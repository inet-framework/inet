//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This tag specifies the creation time. It can be attached to a specific
// region of a packet in order to store the time when that region was created.
//
class CreationTimeTag extends TagBase
{
    simtime_t creationTime = simTime();
}

//
// This tag specifies the network node residence start and end times. It can be
// attached to a specific region of a packet in order to store when that region
// entered and left the network node.
//
class ResidenceTimeTag extends TagBase
{
    simtime_t startTime = -1;
    simtime_t endTime = -1;
}

//
// This is a base class for various time measurement tags.
//
class TimeTagBase extends TagBase
{
    string flowNames[]; // flow names for which the times belong
    simtime_t bitTotalTimes[]; // total bit time summed up for the relevant modules
    simtime_t packetTotalTimes[]; // total packet time summed up for the relevant modules
}

//
// This tag specifies the elapsed time. It can be attached to a specific
// region of a packet in order to measure the elapsed time.
//
class ElapsedTimeTag extends TimeTagBase
{
}

//
// This tag specifies the delaying time. It can be attached to a specific
// region of a packet in order to sum up the delaying time spent in each
// packet delayer module.
//
class DelayingTimeTag extends TimeTagBase
{
}

//
// This tag specifies the queueing time. It can be attached to a specific
// region of a packet in order to sum up the queueing time spent in each
// queueing module.
//
class QueueingTimeTag extends TimeTagBase
{
}

//
// This tag specifies the processing time. It can be attached to a specific
// region of a packet in order to sum up the processing time spent in each
// packet processor module.
//
class ProcessingTimeTag extends TimeTagBase
{
}

//
// This tag specifies the transmission time. It can be attached to a specific
// region of a packet in order to sum up the transmission time spent in each
// transmitter.
//
class TransmissionTimeTag extends TimeTagBase
{
}

//
// This tag specifies the propagation time. It can be attached to a specific
// region of a packet in order to sum up the propagation time spent in each
// transmission channel.
//
class PropagationTimeTag extends TimeTagBase
{
}
