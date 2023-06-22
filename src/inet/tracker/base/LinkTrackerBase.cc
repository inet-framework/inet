//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/tracker/base/LinkTrackerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TrackerTag_m.h"
#include "inet/physicallayer/common/Signal.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {

namespace tracker {

LinkTrackerBase::~LinkTrackerBase()
{
    if (trackLinks)
        unsubscribe();
}

void LinkTrackerBase::initialize(int stage)
{
    TrackerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *fileName = par("fileName");
        if (!opp_isempty(fileName))
            file.open(fileName);
        trackLinks = par("trackLinks");
        const char *activityLevelString = par("activityLevel");
        if (!strcmp(activityLevelString, "service"))
            activityLevel = ACTIVITY_LEVEL_SERVICE;
        else if (!strcmp(activityLevelString, "peer"))
            activityLevel = ACTIVITY_LEVEL_PEER;
        else if (!strcmp(activityLevelString, "protocol"))
            activityLevel = ACTIVITY_LEVEL_PROTOCOL;
        else
            throw cRuntimeError("Unknown activity level: %s", activityLevelString);
        nodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        packetFilter.setExpression(par("packetFilter").objectValue());
        if (trackLinks)
            subscribe();
    }
}

void LinkTrackerBase::finish()
{
    if (file.is_open())
        file.close();
}

void LinkTrackerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "interfaceFilter"))
            interfaceFilter.setPattern(par("interfaceFilter"));
        else if (!strcmp(name, "packetFilter"))
            packetFilter.setPattern(par("packetFilter"));
    }
}

void LinkTrackerBase::subscribe()
{
    if (activityLevel == ACTIVITY_LEVEL_SERVICE) {
        trackingSubjectModule->subscribe(packetSentToUpperSignal, this);
        trackingSubjectModule->subscribe(packetReceivedFromUpperSignal, this);
    }
    else if (activityLevel == ACTIVITY_LEVEL_PEER) {
        trackingSubjectModule->subscribe(packetSentToPeerSignal, this);
        trackingSubjectModule->subscribe(packetReceivedFromPeerSignal, this);
    }
    else if (activityLevel == ACTIVITY_LEVEL_PROTOCOL) {
        trackingSubjectModule->subscribe(packetSentToLowerSignal, this);
        trackingSubjectModule->subscribe(packetReceivedFromLowerSignal, this);
        trackingSubjectModule->subscribe(transmissionEndedSignal, this);
        trackingSubjectModule->subscribe(receptionEndedSignal, this);
    }
}

void LinkTrackerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto trackingSubjectModule = findModuleFromPar<cModule>(par("trackingSubjectModule"), this);
    if (trackingSubjectModule != nullptr) {
        if (activityLevel == ACTIVITY_LEVEL_SERVICE) {
            trackingSubjectModule->unsubscribe(packetSentToUpperSignal, this);
            trackingSubjectModule->unsubscribe(packetReceivedFromUpperSignal, this);
        }
        else if (activityLevel == ACTIVITY_LEVEL_PEER) {
            trackingSubjectModule->unsubscribe(packetSentToPeerSignal, this);
            trackingSubjectModule->unsubscribe(packetReceivedFromPeerSignal, this);
        }
        else if (activityLevel == ACTIVITY_LEVEL_PROTOCOL) {
            trackingSubjectModule->unsubscribe(packetSentToLowerSignal, this);
            trackingSubjectModule->unsubscribe(packetReceivedFromLowerSignal, this);
            trackingSubjectModule->unsubscribe(transmissionEndedSignal, this);
            trackingSubjectModule->unsubscribe(receptionEndedSignal, this);
        }
    }
}

cModule *LinkTrackerBase::getLastModule(int treeId)
{
    auto it = lastModules.find(treeId);
    if (it == lastModules.end())
        return nullptr;
    else
        return getSimulation()->getModule(it->second);
}

void LinkTrackerBase::setLastModule(int treeId, cModule *module)
{
    lastModules[treeId] = module->getId();
}

void LinkTrackerBase::removeLastModule(int treeId)
{
    lastModules.erase(lastModules.find(treeId));
}

void LinkTrackerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if ((activityLevel == ACTIVITY_LEVEL_SERVICE && signal == packetReceivedFromUpperSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PEER && signal == packetSentToPeerSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == packetSentToLowerSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == transmissionEndedSignal))
    {
        auto module = check_and_cast<cModule *>(source);
        if (isLinkStart(module)) {
            const Packet *packet = nullptr;
            auto signal = dynamic_cast<physicallayer::Signal *>(object);
            if (signal != nullptr)
                packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
            auto transmission = dynamic_cast<physicallayer::ITransmission *>(object);
            if (transmission != nullptr)
                packet = transmission->getPacket();
            if (packet == nullptr)
                packet = check_and_cast<Packet *>(object);
            mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) { if (getLastModule(id) != nullptr) removeLastModule(id); });
            auto networkNode = getContainingNode(module);
            auto networkInterface = getContainingNicModule(module);
            if (nodeFilter.matches(networkNode) && interfaceFilter.matches(networkInterface) && packetFilter.matches(packet)) {
                mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) { setLastModule(id, module); });
            }
        }
    }
    else if ((activityLevel == ACTIVITY_LEVEL_SERVICE && signal == packetSentToUpperSignal) ||
             (activityLevel == ACTIVITY_LEVEL_PEER && signal == packetReceivedFromPeerSignal) ||
             (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == packetReceivedFromLowerSignal) ||
             (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == receptionEndedSignal))
    {
        auto receiverModule = check_and_cast<cModule *>(source);
        if (isLinkEnd(receiverModule)) {
            const Packet *packet = nullptr;
            auto signal = dynamic_cast<physicallayer::Signal *>(object);
            if (signal != nullptr)
                packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
            auto reception = dynamic_cast<physicallayer::IReception *>(object);
            if (reception != nullptr)
                packet = reception->getTransmission()->getPacket();
            if (packet == nullptr)
                packet = check_and_cast<Packet *>(object);
            auto receiverNetworkNode = getContainingNode(receiverModule);
            auto receiverNetworkInterface = getContainingNicModule(receiverModule);
            if (nodeFilter.matches(receiverNetworkNode) && interfaceFilter.matches(receiverNetworkInterface) && packetFilter.matches(packet)) {
                cModule *senderModule = nullptr;
                mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) {
                    auto lastModule = getLastModule(id);
                    if (lastModule != nullptr)
                        senderModule = lastModule;
                    // NOTE: don't call removeLastModule(treeId) because other network nodes may still receive this packet
                });
                if (senderModule != nullptr) {
                    auto senderNetworkNode = getContainingNode(senderModule);
                    auto senderNetworkInterface = getContainingNicModule(senderModule);
                    auto trackedPacket = packet->dup();
                    trackedPacket->trim();
                    auto trackerTag = trackedPacket->addTag<TrackerTag>();
                    trackerTag->setTags(getTags());
                    trackPacketSend(trackedPacket, senderNetworkNode, senderNetworkInterface, senderModule, receiverNetworkNode, receiverNetworkInterface, receiverModule);
                    delete trackedPacket;
                }
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

void LinkTrackerBase::trackPacketSend(Packet *packet, cModule *senderNetworkNode, cModule *senderNetworkInterface, cModule *senderModule, cModule *receiverNetworkNode, cModule *receiverNetworkInterface, cModule *receiverModule)
{
    EV_INFO << "Recording virtual packet send" << EV_FIELD(senderModule) << EV_FIELD(receiverModule) << EV_FIELD(packet) << EV_ENDL;
//    // KLUDGE TODO: this gate may not even exist
//    auto senderGate = senderModule->gate("upperLayerIn");
//    packet->setSentFrom(senderModule, senderGate->getId(), simTime());
//    auto envir = getEnvir();
////    envir->beginSend(packet, SendOptions().tags(getTags()));
//    envir->beginSend(packet, SendOptions());
//    // KLUDGE TODO: this gate may not even exist
//    auto arrivalGate = receiverModule->gate("upperLayerOut");
//    envir->messageSendDirect(packet, arrivalGate, ChannelResult());
//    envir->endSend(packet);
    const char *moduleMode = par("moduleMode");
    cModule *sender = nullptr;
    cModule *receiver = nullptr;
    if (!strcmp(moduleMode, "networkNode")) {
        sender = senderNetworkNode;
        receiver = receiverNetworkNode;
    }
    else if (!strcmp(moduleMode, "networkInterface")) {
        sender = senderNetworkInterface;
        receiver = receiverNetworkInterface;
    }
    else if (!strcmp(moduleMode, "module")) {
        sender = senderModule;
        receiver = receiverModule;
    }
    else
        throw cRuntimeError("Unknown moduleMode parameter value");
    file << sender->getFullPath() << "\t" << receiver->getFullPath() << "\t" << packet->getFullName() << "\t" << packet->getTag<TrackerTag>()->getTags() << std::endl;
}

} // namespace tracker

} // namespace inet

