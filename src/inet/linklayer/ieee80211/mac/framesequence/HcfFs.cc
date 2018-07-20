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

#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"
#include "inet/linklayer/ieee80211/mac/framesequence/TxOpFs.h"

namespace inet {
namespace ieee80211 {

HcfFs::HcfFs() :
    // G.3 EDCA and HCCA sequences
    // hcf-sequence =
    //   ( [ CTS ] 1{( Data + group [+ QoS ] ) | Management + broadcast ) +pifs} |
    //   ( [ CTS ] 1{txop-sequence} ) |
    //   (* HC only, polled TXOP delivery *)
    //   ( [ RTS CTS ] non-cf-ack-piggybacked-qos-poll-sequence )
    //   (* HC only, polled TXOP delivery *)
    //   cf-ack-piggybacked-qos-poll-sequence |
    //   (* HC only, self TXOP delivery or termination *)
    //   Data + self + null + CF-Poll + QoS;
    AlternativesFs({new SequentialFs({new OptionalFs(new SelfCtsFs(), OPTIONALFS_PREDICATE(isSelfCtsNeeded)),
                                      new RepeatingFs(new AlternativesFs({new DataFs(), new ManagementFs()}, ALTERNATIVESFS_SELECTOR(selectDataOrManagementSequence)),
                                                      REPEATINGFS_PREDICATE(hasMoreTxOpsAndMulticast))}),
                    new SequentialFs({new OptionalFs(new SelfCtsFs(), OPTIONALFS_PREDICATE(isSelfCtsNeeded)),
                                      new RepeatingFs(new TxOpFs(), REPEATINGFS_PREDICATE(hasMoreTxOps))})},
                   ALTERNATIVESFS_SELECTOR(selectHcfSequence))
{
}

int HcfFs::selectHcfSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    auto frameToTransmit = context->getInProgressFrames()->getFrameToTransmit();
    return frameToTransmit->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress().isMulticast() ? 0 : 1;
}

int HcfFs::selectDataOrManagementSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    auto frameToTransmit = context->getInProgressFrames()->getFrameToTransmit();
    const auto& header = frameToTransmit->peekAtFront<Ieee80211MacHeader>();
    if (dynamicPtrCast<const Ieee80211DataHeader>(header))
        return 0;
    else if (dynamicPtrCast<const Ieee80211MgmtHeader>(header))
        return 1;
    else
        throw cRuntimeError("frameToTransmit must be either a Data or a Management frame");
}

bool HcfFs::isSelfCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    return false;
}

bool HcfFs::hasMoreTxOps(RepeatingFs *frameSequence, FrameSequenceContext *context)
{
    bool hasFrameToTransmit = context->getInProgressFrames()->hasInProgressFrames();
    if (hasFrameToTransmit) {
        auto nextFrameToTransmit = context->getInProgressFrames()->getFrameToTransmit();
        const auto& nextHeader = nextFrameToTransmit->peekAtFront<Ieee80211MacHeader>();
        return frameSequence->getCount() == 0 || (!nextHeader->getReceiverAddress().isMulticast() && context->getQoSContext()->txopProcedure->getRemaining() > 0);
    }
    return false;
}

bool HcfFs::hasMoreTxOpsAndMulticast(RepeatingFs *frameSequence, FrameSequenceContext *context)
{
    return hasMoreTxOps(frameSequence, context) && context->getInProgressFrames()->getFrameToTransmit()->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress().isMulticast();
}

} // namespace ieee80211
} // namespace inet
