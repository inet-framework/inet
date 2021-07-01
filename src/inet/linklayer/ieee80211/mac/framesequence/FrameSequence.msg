//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
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

