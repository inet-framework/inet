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

#include "inet/tracker/base/PathTrackerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TrackerTag_m.h"

namespace inet {

namespace tracker {

PathTrackerBase::~PathTrackerBase()
{
    if (trackPaths)
        unsubscribe();
}

void PathTrackerBase::initialize(int stage)
{
    TrackerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *fileName = par("fileName");
        if (!opp_isempty(fileName))
            file.open(fileName);
        trackPaths = par("trackPaths");
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
        packetFilter.setPattern(par("packetFilter"));
        if (trackPaths)
            subscribe();
    }
}

void PathTrackerBase::finish()
{
    if (file.is_open())
        file.close();
}

void PathTrackerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "packetFilter"))
            packetFilter.setPattern(par("packetFilter"));
    }
}

void PathTrackerBase::subscribe()
{
    if (activityLevel == ACTIVITY_LEVEL_SERVICE) {
        trackingSubjectModule->subscribe(packetSentToUpperSignal, this);
        trackingSubjectModule->subscribe(packetReceivedFromUpperSignal, this);
        trackingSubjectModule->subscribe(packetReceivedFromLowerSignal, this);
    }
    else if (activityLevel == ACTIVITY_LEVEL_PEER) {
        trackingSubjectModule->subscribe(packetSentToPeerSignal, this);
        trackingSubjectModule->subscribe(packetReceivedFromPeerSignal, this);
    }
    else if (activityLevel == ACTIVITY_LEVEL_PROTOCOL) {
        trackingSubjectModule->subscribe(packetSentToLowerSignal, this);
        trackingSubjectModule->subscribe(packetReceivedFromLowerSignal, this);
    }
}

void PathTrackerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto trackingSubjectModule = findModuleFromPar<cModule>(par("trackingSubjectModule"), this);
    if (trackingSubjectModule != nullptr) {
        if (activityLevel == ACTIVITY_LEVEL_SERVICE) {
            trackingSubjectModule->unsubscribe(packetSentToUpperSignal, this);
            trackingSubjectModule->unsubscribe(packetReceivedFromUpperSignal, this);
            trackingSubjectModule->unsubscribe(packetReceivedFromLowerSignal, this);
        }
        else if (activityLevel == ACTIVITY_LEVEL_PEER) {
            trackingSubjectModule->unsubscribe(packetSentToPeerSignal, this);
            trackingSubjectModule->unsubscribe(packetReceivedFromPeerSignal, this);
        }
        else if (activityLevel == ACTIVITY_LEVEL_PROTOCOL) {
            trackingSubjectModule->unsubscribe(packetSentToLowerSignal, this);
            trackingSubjectModule->unsubscribe(packetReceivedFromLowerSignal, this);
        }
    }
}

const std::vector<int> *PathTrackerBase::getIncompletePath(int chunkId)
{
    auto it = incompletePaths.find(chunkId);
    if (it == incompletePaths.end())
        return nullptr;
    else
        return &it->second;
}

void PathTrackerBase::addToIncompletePath(int chunkId, cModule *module)
{
    auto& moduleIds = incompletePaths[chunkId];
    auto moduleId = module->getId();
    if (moduleIds.size() == 0 || moduleIds[moduleIds.size() - 1] != moduleId)
        moduleIds.push_back(moduleId);
}

void PathTrackerBase::removeIncompletePath(int chunkId)
{
    incompletePaths.erase(incompletePaths.find(chunkId));
}

void PathTrackerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if ((activityLevel == ACTIVITY_LEVEL_SERVICE && signal == packetReceivedFromUpperSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PEER && signal == packetSentToPeerSignal) ||
        (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == packetSentToLowerSignal))
    {
        auto module = check_and_cast<cModule *>(source);
        if (isPathStart(module)) {
            auto networkNode = getContainingNode(module);
            auto packet = check_and_cast<Packet *>(object);
            if (nodeFilter.matches(networkNode) && packetFilter.matches(packet)) {
                mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) {
                    auto path = getIncompletePath(id);
                    if (path != nullptr)
                        removeIncompletePath(id);
                    addToIncompletePath(id, module);
                });
            }
        }
    }
    else if (signal == packetReceivedFromLowerSignal) {
        auto module = check_and_cast<cModule *>(source);
        if (isPathElement(module)) {
            auto packet = check_and_cast<Packet *>(object);
            if (packetFilter.matches(packet)) {
                mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) {
                    auto path = getIncompletePath(id);
                    if (path != nullptr)
                        addToIncompletePath(id, module);
                });
            }
        }
    }
    else if ((activityLevel == ACTIVITY_LEVEL_SERVICE && signal == packetSentToUpperSignal) ||
             (activityLevel == ACTIVITY_LEVEL_PEER && signal == packetReceivedFromPeerSignal) ||
             (activityLevel == ACTIVITY_LEVEL_PROTOCOL && signal == packetReceivedFromLowerSignal))
    {
        auto receiverModule = check_and_cast<cModule *>(source);
        if (isPathEnd(receiverModule)) {
            auto receiverNetworkNode = getContainingNode(receiverModule);
            auto packet = check_and_cast<Packet *>(object);
            if (nodeFilter.matches(receiverNetworkNode) && packetFilter.matches(packet)) {
                mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int id) {
                    auto path = getIncompletePath(id);
                    if (path != nullptr) {
                        addToIncompletePath(id, receiverModule);
                        if (path->size() > 1) {
                            auto senderModule = check_and_cast<cModule *>(getSimulation()->getComponent(path->at(0)));
                            auto senderNetworkNode = getContainingNode(senderModule);
                            auto trackedPacket = packet->dup();
                            trackedPacket->trim();
                            auto trackerTag = trackedPacket->addTag<TrackerTag>();
                            trackerTag->setTags(getTags());
                            trackPacketSend(trackedPacket, senderNetworkNode, senderModule, receiverNetworkNode, receiverModule, path);
                            delete trackedPacket;
                        }
                        removeIncompletePath(id);
                    }
                });
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

void PathTrackerBase::trackPacketSend(Packet *packet, cModule *senderNetworkNode, cModule *senderModule, cModule *receiverNetworkNode, cModule *receiverModule, const std::vector<int> *path)
{
    EV_INFO << "Recording virtual packet send" << EV_FIELD(senderModule) << EV_FIELD(receiverModule) << EV_FIELD(packet) << EV_ENDL;
//    // KLUDGE TODO: this gate may not even exist
//    auto senderGate = senderModule->hasGate("transportIn") ? senderModule->gate("transportIn") : senderModule->gate("appIn"); // TODO:
//    packet->setSentFrom(senderModule, senderGate->getId(), simTime());
//    auto envir = getEnvir();
////    envir->beginSend(packet, SendOptions().tags(getTags()));
//    envir->beginSend(packet, SendOptions());
//    // KLUDGE TODO: this gate may not even exist
//    auto arrivalGate = receiverModule->hasGate("transportOut") ? receiverModule->gate("transportOut") : receiverModule->gate("appOut");
//    envir->messageSendDirect(packet, arrivalGate, ChannelResult());
//    envir->endSend(packet);
    const char *moduleMode = par("moduleMode");
    cModule *sender = nullptr;
    cModule *receiver = nullptr;
    if (!strcmp(moduleMode, "networkNode")) {
        sender = senderNetworkNode;
        receiver = receiverNetworkNode;
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

