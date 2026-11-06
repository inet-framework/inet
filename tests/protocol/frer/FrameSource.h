// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_FRERTEST_FRAMESOURCE_H
#define __INET_FRERTEST_FRAMESOURCE_H

#include "inet/common/INETDefs.h"
#include "inet/common/SequenceNumberTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {
namespace frertest {

// Sends the declared trace to the production merger. It contains no recovery logic.
class FrameSource : public cSimpleModule
{
  public:
    FrameSource() : cSimpleModule(65536) {}

  protected:
    virtual void activity() override
    {
        auto frames = check_and_cast<cValueArray *>(par("frames").objectValue());
        for (int i = 0; i < frames->size(); i++) {
            cStringTokenizer fields(frames->get(i).stringValue(), ",");
            int64_t microseconds = std::stoll(fields.nextToken());
            std::string stream = fields.nextToken();
            int sequence = std::stoi(fields.nextToken());
            std::string name = fields.nextToken();
            simtime_t when = SimTime(microseconds, SIMTIME_US);
            if (when < simTime())
                throw cRuntimeError("Frame trace must be ordered by time");
            wait(when - simTime());
            auto packet = new Packet(name.c_str(), makeShared<ByteCountChunk>(B(64)));
            packet->addTag<StreamInd>()->setStreamName(stream.c_str());
            if (sequence >= 0)
                packet->addTag<SequenceNumberInd>()->setSequenceNumber(sequence);
            send(packet, "out");
        }
    }
};
Define_Module(FrameSource);

// Check the externally observed packet and time, not any private recovery state.
inline bool frameMatches(const Packet *packet, simtime_t time, const char *name,
                         const char *stream, int sequence, int64_t microseconds)
{
    if (packet == nullptr || strcmp(packet->getName(), name) || time != SimTime(microseconds, SIMTIME_US))
        return false;
    auto streamTag = packet->findTag<StreamInd>();
    auto sequenceTag = packet->findTag<SequenceNumberInd>();
    return streamTag != nullptr && !strcmp(streamTag->getStreamName(), stream)
           && (sequence < 0 ? sequenceTag == nullptr :
               sequenceTag != nullptr && sequenceTag->getSequenceNumber() == sequence);
}

} // namespace frertest
} // namespace inet
#endif
