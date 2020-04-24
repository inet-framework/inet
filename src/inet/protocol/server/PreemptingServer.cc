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

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/protocol/contract/IProtocol.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"
#include "inet/protocol/ordering/SequenceNumberHeader_m.h"
#include "inet/protocol/server/PreemptingServer.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {

Define_Module(PreemptingServer);

void PreemptingServer::initialize(int stage)
{
    PacketServerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minPacketLength = b(par("minPacketLength"));
        roundingLength = b(par("roundingLength"));
        preemptedOutputGate = gate("preemptedOut");
        preemptedConsumer = getConnectedModule<IPassivePacketSink>(preemptedOutputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(preemptedOutputGate);
    }
}

void PreemptingServer::startSendingPacket()
{
    packet = provider->pullPacketStart(inputGate->getPathStartGate(), datarate);
    take(packet);
    auto fragmentTag = packet->findTag<FragmentTag>();
    if (fragmentTag == nullptr) {
        fragmentTag = packet->addTag<FragmentTag>();
        fragmentTag->setFirstFragment(true);
        fragmentTag->setLastFragment(true);
        fragmentTag->setFragmentNumber(0);
        fragmentTag->setNumFragments(-1);
    }
    packet->setArrival(getId(), inputGate->getId(), simTime());
    EV_INFO << "Sending packet " << packet->getName() << " started." << endl;
    animateSend(packet, outputGate);
    consumer->pushPacketStart(packet->dup(), outputGate->getPathEndGate());
    processedTotalLength += packet->getDataLength();
    numProcessedPackets++;
    updateDisplayString();
}

void PreemptingServer::endSendingPacket()
{
    EV_INFO << "Sending packet " << packet->getName() << " ended.\n";
    consumer->pushPacketEnd(packet, outputGate->getPathEndGate());
    packet = nullptr;
}

int PreemptingServer::getPriority(Packet *packet) const
{
    // TODO: make this a parameter
    auto dscpInd = packet->findTag<DscpInd>();
    return dscpInd != nullptr ? dscpInd->getDifferentiatedServicesCodePoint() : 0;
//    return packet->getTag<UserPriorityReq>()->getUserPriority();
}

void PreemptingServer::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (packet != nullptr)
        endSendingPacket();
    if (provider->canPullSomePacket(inputGate->getPathStartGate()))
        startSendingPacket();
}

void PreemptingServer::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (consumer->canPushSomePacket(outputGate->getPathEndGate())) {
        if (provider->canPullSomePacket(inputGate->getPathStartGate()))
            startSendingPacket();
    }
    else {
        auto nextPacket = provider->canPullPacket(inputGate->getPathStartGate());
        if (packet != nullptr && nextPacket != nullptr && getPriority(nextPacket) > getPriority(packet)) {
            b confirmedLength = consumer->getPushPacketProcessedLength(packet, outputGate->getPathEndGate());
            b preemptedLength = roundingLength * ((confirmedLength + roundingLength - b(1)) / roundingLength);
            if (preemptedLength < minPacketLength)
                preemptedLength = minPacketLength;
            if (preemptedLength + minPacketLength <= packet->getTotalLength()) {
                std::string name = std::string(packet->getName()) + "-frag";
                // confirmed part
                packet->setName(name.c_str());
                const auto& remainingData = packet->removeAtBack(packet->getTotalLength() - preemptedLength);
                FragmentTag *fragmentTag = packet->getTag<FragmentTag>();
                fragmentTag->setLastFragment(false);
                // remaining part
                Packet *remainingPart = new Packet(name.c_str(), remainingData);
                remainingPart->copyTags(*packet);
                FragmentTag *remainingPartFragmentTag = remainingPart->getTag<FragmentTag>();
                remainingPartFragmentTag->setFirstFragment(false);
                remainingPartFragmentTag->setLastFragment(true);
                remainingPartFragmentTag->setFragmentNumber(fragmentTag->getFragmentNumber() + 1);
                // send parts
                animateSend(packet, outputGate);
                auto l = packet->getTotalLength() - preemptedLength;
                consumer->pushPacketProgress(packet, outputGate->getPathEndGate(), preemptedLength, l);
                packet = nullptr;
                pushOrSendPacket(remainingPart, preemptedOutputGate, preemptedConsumer);
            }
        }
    }
}

} // namespace inet
