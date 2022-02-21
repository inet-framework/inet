//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.Packet;

cplusplus {{
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"
}}

namespace inet::ieee80211;

class FrameSequenceContext extends cObject
{
    @existingClass;
    @descriptor(readonly);
    IFrameSequenceStep *steps[] @getter(getStep) @sizeGetter(getNumSteps);
}

class IFrameSequenceStep
{
    @existingClass;
    @descriptor(readonly);
}

class TransmitStep extends IFrameSequenceStep
{
    @existingClass;
    @descriptor(readonly);
    Packet *frameToTransmit;
    simtime_t ifs;
}

class ReceiveStep extends IFrameSequenceStep
{
    @existingClass;
    @descriptor(readonly);
    Packet *receivedFrame;
}
