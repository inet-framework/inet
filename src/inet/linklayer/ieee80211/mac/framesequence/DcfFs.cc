//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/linklayer/ieee80211/mac/framesequence/DcfFs.h"
#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

DcfFs::DcfFs() :
    // Excerpt from G.2 Basic sequences (p. 2309)
    // frame-sequence =
    //   ( [ CTS ] ( Management + broadcast | Data + group ) ) |
    //   ( [ CTS | RTS CTS] {frag-frame ACK } last-frame ACK )
    //
    // frag-frame = ( Data | Management ) + individual + frag;
    // last-frame = ( Data | Management ) + individual + last;
    //
    AlternativesFs({new SequentialFs({new OptionalFs(new SelfCtsFs(), OPTIONALFS_PREDICATE(isSelfCtsNeeded)),
                                      new AlternativesFs({new ManagementFs(), new DataFs()}, ALTERNATIVESFS_SELECTOR(selectMulticastDataOrMgmt))}),
                    new SequentialFs({new OptionalFs(new AlternativesFs({new SelfCtsFs(), new SequentialFs({new RtsFs(), new CtsFs()})},
                                                                        ALTERNATIVESFS_SELECTOR(selectSelfCtsOrRtsCts)),
                                                     OPTIONALFS_PREDICATE(isCtsOrRtsCtsNeeded)),
                                      new RepeatingFs(new FragFrameAckFs(), REPEATINGFS_PREDICATE(hasMoreFragments)),
                                      new LastFrameAckFs()})},
                   ALTERNATIVESFS_SELECTOR(selectDcfSequence))
{
}

int DcfFs::selectDcfSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    bool multicastMgmtOrDataSequence = isBroadcastManagementOrGroupDataSequenceNeeded(frameSequence, context);
    bool fragFrameSequence = isFragFrameSequenceNeeded(frameSequence, context);
    if (multicastMgmtOrDataSequence) return 0;
    else if (fragFrameSequence) return 1;
    else throw cRuntimeError("One alternative must be chosen");
}

int DcfFs::selectSelfCtsOrRtsCts(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    bool selfCts = isSelfCtsNeeded(nullptr, context); // TODO
    bool rtsCts = isRtsCtsNeeded(nullptr, context); // TODO
    if (selfCts) return 0;
    else if (rtsCts) return 1;
    else throw cRuntimeError("One alternative must be chosen");
}

bool DcfFs::hasMoreFragments(RepeatingFs *frameSequence, FrameSequenceContext *context)
{
    return context->getInProgressFrames()->hasInProgressFrames() && context->getInProgressFrames()->getFrameToTransmit()->peekAtFront<Ieee80211MacHeader>()->getMoreFragments();
}

bool DcfFs::isSelfCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    return false; // TODO: Implement
}

bool DcfFs::isRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    auto protectedFrame = context->getInProgressFrames()->getFrameToTransmit();
    return context->getRtsPolicy()->isRtsNeeded(protectedFrame, protectedFrame->peekAtFront<Ieee80211MacHeader>());
}

bool DcfFs::isCtsOrRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    bool selfCts = isSelfCtsNeeded(frameSequence, context);
    bool rtsCts = isRtsCtsNeeded(frameSequence, context);
    return selfCts || rtsCts;
}

bool DcfFs::isBroadcastManagementOrGroupDataSequenceNeeded(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    if (context->getInProgressFrames()->hasInProgressFrames()) {
        auto frameToTransmit = context->getInProgressFrames()->getFrameToTransmit();
        return frameToTransmit->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress().isMulticast();
    }
    else
        return false;
}

int DcfFs::selectMulticastDataOrMgmt(AlternativesFs* frameSequence, FrameSequenceContext* context)
{
    auto frameToTransmit = context->getInProgressFrames()->getFrameToTransmit();
    return dynamicPtrCast<const Ieee80211MgmtHeader>(frameToTransmit->peekAtFront<Ieee80211MacHeader>()) ? 0 : 1;
}

bool DcfFs::isFragFrameSequenceNeeded(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    return context->getInProgressFrames()->hasInProgressFrames() && dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(context->getInProgressFrames()->getFrameToTransmit()->peekAtFront<Ieee80211MacHeader>());
}

} // namespace ieee80211
} // namespace inet
