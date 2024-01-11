//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/routing/leach/Leach.h"

namespace inet {
namespace leach {

Register_Enum(inet::leach::Leach, (Leach::ch, Leach::nch));
Define_Module(Leach);

Leach::Leach() {

}

Leach::~Leach() {
    cancelAndDelete(event);
    stop();
}

void Leach::initialize(int stage) {
    RoutingProtocolBase::initialize(stage);

    //reads from omnetpp.ini
    if (stage == INITSTAGE_LOCAL) {
        sequencenumber = 0;
        host = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"),
                this);

        clusterHeadPercentage = par("clusterHeadPercentage");
        numNodes = par("numNodes");
        netInterface = par("netInterface");

        roundDuration = dblrand(0) * 10;
        TDMADelayCounter = 1;
        event = new cMessage("event");
        roundDuration = dblrand(0) * 10;
        wasCH = false;

        vector<nodeMemoryObject> nodeMemory(numNodes); // Localized NCH node memory with CH data
        vector<TDMAScheduleEntry> nodeCHMemory(numNodes); // Localized CH memory
        vector<TDMAScheduleEntry> extractedTDMASchedule(numNodes); // Localized NCH node memory with extracted TDMA data

    } else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerService(Protocol::manet, nullptr, gate("ipIn"));
        registerProtocol(Protocol::manet, gate("ipOut"), nullptr);
    }
}

void Leach::start() {
    // Search the 802154 interface
    int num_802154 = 0;
    NetworkInterface *ie;
    NetworkInterface *i_face;
    const char *name;

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        ie = ift->getInterface(i);
        name = ie->getInterfaceName();
        if (strstr(name, netInterface) != nullptr) {
            i_face = ie;
            num_802154++;
            interfaceId = i;
        }
    }

    // One enabled network interface (in total)
    if (num_802154 == 1)
        interface80211ptr = i_face;
    else
        throw cRuntimeError("DSDV has found %i 80211 interfaces", num_802154);
    CHK(interface80211ptr->getProtocolDataForUpdate<Ipv4InterfaceData>())->joinMulticastGroup(
            Ipv4Address::LL_MANET_ROUTERS);

    // schedules a random periodic event
    event->setKind(SELF);
    scheduleAt(simTime() + uniform(0.0, par("maxVariance").doubleValue()),
            event);
}

void Leach::stop() {

    nodeMemory.clear();
    nodeCHMemory.clear();
    extractedTDMASchedule.clear();
    TDMADelayCounter = 1;
}

void Leach::handleMessageWhenUp(cMessage *msg) {
    // if node is sending message
    if (msg->isSelfMessage()) {
        double randNo = dblrand(1);
        double threshold = generateThresholdValue(round);

        if (randNo < threshold && wasCH == false) {
            setLeachState(ch);               // Set state for GUI visualization
            wasCH = true;
            handleSelfMessage(msg);
        }

        round += 1;
        int intervalLength = 1.0 / clusterHeadPercentage;
        if (fmod(round, intervalLength) == 0) { // reset values at end of number of subintervals
            wasCH = false;
            nodeMemory.clear();
            nodeCHMemory.clear();
            extractedTDMASchedule.clear();
            TDMADelayCounter = 1;
        }

        // schedule another self message every time new one is received by node
        event->setKind(SELF);
        scheduleAt(simTime() + roundDuration, event);
        // if node is receiving message
    } else if (check_and_cast<Packet*>(msg)->getTag<PacketProtocolTag>()->getProtocol()
            == &Protocol::manet) {
        processMessage(msg);
    } else {
        throw cRuntimeError("Message not supported %s", msg->getName());
    }
}

void Leach::handleMessageWhenDown(cMessage *msg) {
    delete msg;
    OperationalBase::handleMessageWhenDown(msg);
}

void Leach::handleSelfMessage(cMessage *msg) {
    if (msg == event) {
        if (event->getKind() == SELF) {
            auto ctrlPkt = makeShared<LeachControlPkt>();

            // Filling the LeachControlPkt fields
            ctrlPkt->setPacketType(CH);
            Ipv4Address source = (interface80211ptr->getProtocolData<
                    Ipv4InterfaceData>()->getIPAddress());
            ctrlPkt->setChunkLength(b(128)); ///size of Hello message in bits
            ctrlPkt->setSrcAddress(source);

            //new control info for LeachControlPkt
            auto packet = new Packet("LEACHControlPkt", ctrlPkt);
            auto addressReq = packet->addTag<L3AddressReq>();
            addressReq->setDestAddress(Ipv4Address(255, 255, 255, 255));
            addressReq->setSrcAddress(source); //let's try the limited broadcast
            packet->addTag<InterfaceReq>()->setInterfaceId(
                    interface80211ptr->getInterfaceId());
            packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
            packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

            //broadcast to other nodes the hello message
            send(packet, "ipOut");
            packet = nullptr;
            ctrlPkt = nullptr;
        }
    } else {
        delete msg;
    }
}

void Leach::processMessage(cMessage *msg) {
    Ipv4Address selfAddr =
            (interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
    auto receivedCtrlPkt =
            staticPtrCast<LeachControlPkt>(
                    check_and_cast<Packet*>(msg)->peekData<LeachControlPkt>()->dupShared());
    Packet *receivedPkt = check_and_cast<Packet*>(msg);
    auto &leachControlPkt = receivedPkt->popAtFront<LeachControlPkt>();
    auto packetType = leachControlPkt->getPacketType();

    // filter packet based on type and run specific functions
    if (msg->arrivedOn("ipIn")) {
        // first broadcast from CH to NCH nodes
        if (packetType == CH) {
            Ipv4Address CHAddr = receivedCtrlPkt->getSrcAddress();

            auto signalPowerInd = receivedPkt->getTag<SignalPowerInd>();
            double rxPower = signalPowerInd->getPower().get();

            addToNodeMemory(selfAddr, CHAddr, rxPower);
            sendAckToCH(selfAddr, CHAddr);

            // ACK packet from NCH node to CH
        } else if (packetType == ACK && leachState == ch) {
            Ipv4Address nodeAddr = receivedCtrlPkt->getSrcAddress();

            addToNodeCHMemory(nodeAddr);
            if (nodeCHMemory.size() > 2) {
                sendSchToNCH(selfAddr);
            }

            // TDMA schedule from CH to NCH
        } else if (packetType == SCH) {
            Ipv4Address CHAddr = receivedCtrlPkt->getSrcAddress();

            int scheduleArraySize = receivedCtrlPkt->getScheduleArraySize();
            // Traverses through schedule array in packets and sets values into a vector in local node memory
            for (int counter = 0; counter < scheduleArraySize; counter++) {
                ScheduleEntry tempScheduleEntry = receivedCtrlPkt->getSchedule(
                        counter);
                TDMAScheduleEntry extractedTDMAScheduleEntry;
                extractedTDMAScheduleEntry.nodeAddress =
                        tempScheduleEntry.getNodeAddress();
                extractedTDMAScheduleEntry.TDMAdelay =
                        tempScheduleEntry.getTDMAdelay();
                extractedTDMASchedule.push_back(extractedTDMAScheduleEntry);
            }

            // Finds TDMA slot for self by traversing through earlier generated vector
            double receivedTDMADelay = -1;
            for (auto &it : extractedTDMASchedule) {
                if (it.nodeAddress == selfAddr) {
                    receivedTDMADelay = it.TDMAdelay;
                    break;
                }
            }

            if (receivedTDMADelay > -1) { // Only sends data to CH if self address is included in schedule
                sendDataToCH(selfAddr, CHAddr, receivedTDMADelay);
            }
            // Data packet from NCH to CH
        } else if (packetType == DATA) {
            Ipv4Address NCHAddr = receivedCtrlPkt->getSrcAddress();

            sendDataToBS(selfAddr);

            // BS packet from CH to BS - deleted in the case of standard nodes
        } else if (packetType == BS) {
            delete msg;
        }
    } else {
        throw cRuntimeError("Message arrived on unknown gate %s",
                msg->getArrivalGate()->getName());
    }
}

// Runs during node shutdown events
void Leach::handleStopOperation(LifecycleOperation *operation) {
    cancelAndDelete(event);

}

// Runs during node crash events
void Leach::handleCrashOperation(LifecycleOperation *operation) {
    cancelAndDelete(event);
}

// Threshold value for CH selection
double Leach::Leach::generateThresholdValue(int round) {
    int intervalLength = 1.0 / clusterHeadPercentage;
    double threshold = (clusterHeadPercentage
            / (1 - clusterHeadPercentage * (fmod(round, intervalLength))));

    return threshold;
}

// Add CH to non CH node memory
void Leach::addToNodeMemory(Ipv4Address nodeAddr, Ipv4Address CHAddr,
        double energy) {
    if (!isCHAddedInMemory(CHAddr)) {
        nodeMemoryObject node;
        node.nodeAddr = nodeAddr;
        node.CHAddr = CHAddr;
        node.energy = energy;
        nodeMemory.push_back(node);
    }
}

// Add non CH to CH node memory for TDMA schedule generation
void Leach::addToNodeCHMemory(Ipv4Address NCHAddr) {
    if (!isNCHAddedInCHMemory(NCHAddr)) {
        TDMAScheduleEntry scheduleEntry;
        scheduleEntry.nodeAddress = NCHAddr;
        scheduleEntry.TDMAdelay = TDMADelayCounter;
        nodeCHMemory.push_back(scheduleEntry);
        TDMADelayCounter++; // Counter increases and sets slots for NCH transmission time
    }
}

// Checks non CH nodes memory for CH records
bool Leach::isCHAddedInMemory(Ipv4Address CHAddr) {
    for (auto &it : nodeMemory) {
        if (it.CHAddr == CHAddr) {
            return true;
        }
    }
    return false;
}

// Checks CH nodes memory to generate TDMA schedule
bool Leach::isNCHAddedInCHMemory(Ipv4Address NCHAddr) {
    for (auto &it : nodeCHMemory) {
        if (it.nodeAddress == NCHAddr) {
            return true;
        }
    }
    return false;
}

void Leach::generateTDMASchedule() {
    for (auto &it : nodeCHMemory) {
        ScheduleEntry scheduleEntry;
        scheduleEntry.setNodeAddress(it.nodeAddress);
        scheduleEntry.setTDMAdelay(it.TDMAdelay);
    }
}

// Set CH/non CH states for GUI visualization - optional
void Leach::setLeachState(LeachState ls) {
    leachState = ls;
}

// Send broadcast acknowledgement to CH from non CH nodes
void Leach::sendAckToCH(Ipv4Address nodeAddr, Ipv4Address CHAddr) {
    auto ackPkt = makeShared<LeachAckPkt>();
    ackPkt->setPacketType(ACK);
    ackPkt->setChunkLength(b(128)); ///size of Hello message in bits
    ackPkt->setSrcAddress(nodeAddr);

    auto ackPacket = new Packet("LeachAckPkt", ackPkt);
    auto addressReq = ackPacket->addTag<L3AddressReq>();

    addressReq->setDestAddress(getIdealCH(nodeAddr, CHAddr));
    addressReq->setSrcAddress(nodeAddr);
    ackPacket->addTag<InterfaceReq>()->setInterfaceId(
            interface80211ptr->getInterfaceId());
    ackPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    ackPacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

    send(ackPacket, "ipOut");
}

// Sends TDMA schedule to non CH nodes
void Leach::sendSchToNCH(Ipv4Address selfAddr) {
    auto schedulePkt = makeShared<LeachSchedulePkt>();
    schedulePkt->setPacketType(SCH);
    schedulePkt->setChunkLength(b(128)); ///size of Hello message in bits
    schedulePkt->setSrcAddress(selfAddr);

    for (auto &it : nodeCHMemory) {
        ScheduleEntry scheduleEntry;
        scheduleEntry.setNodeAddress(it.nodeAddress);
        scheduleEntry.setTDMAdelay(it.TDMAdelay);
        schedulePkt->insertSchedule(scheduleEntry);
    }

    auto schedulePacket = new Packet("LeachSchedulePkt", schedulePkt);
    auto scheduleReq = schedulePacket->addTag<L3AddressReq>();

    scheduleReq->setDestAddress(Ipv4Address(255, 255, 255, 255));
    scheduleReq->setSrcAddress(selfAddr);
    schedulePacket->addTag<InterfaceReq>()->setInterfaceId(
            interface80211ptr->getInterfaceId());
    schedulePacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    schedulePacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

    send(schedulePacket, "ipOut");
}

// Sends data to CH node
void Leach::sendDataToCH(Ipv4Address nodeAddr, Ipv4Address CHAddr,
        double TDMAslot) {
    auto dataPkt = makeShared<LeachDataPkt>();
    dataPkt->setPacketType(DATA);
    double temperature = (double) rand() / RAND_MAX;
    double humidity = (double) rand() / RAND_MAX;

    dataPkt->setChunkLength(b(128));
    dataPkt->setTemperature(temperature);
    dataPkt->setHumidity(humidity);
    dataPkt->setSrcAddress(nodeAddr);

    auto dataPacket = new Packet("LEACHDataPkt", dataPkt);
    auto addressReq = dataPacket->addTag<L3AddressReq>();

    addressReq->setDestAddress(getIdealCH(nodeAddr, CHAddr));
    addressReq->setSrcAddress(nodeAddr);
    dataPacket->addTag<InterfaceReq>()->setInterfaceId(
            interface80211ptr->getInterfaceId());
    dataPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    dataPacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

    sendDelayed(dataPacket, TDMAslot, "ipOut");
}

// CH sends data to BS
void Leach::sendDataToBS(Ipv4Address CHAddr) {
    auto bsPkt = makeShared<LeachBSPkt>();
    bsPkt->setPacketType(BS);

    bsPkt->setChunkLength(b(128));
    bsPkt->setCHAddr(CHAddr);

    auto bsPacket = new Packet("LEACHBsPkt", bsPkt);
    auto addressReq = bsPacket->addTag<L3AddressReq>();

    addressReq->setDestAddress(Ipv4Address(10, 0, 0, 1));
    addressReq->setSrcAddress(CHAddr);
    bsPacket->addTag<InterfaceReq>()->setInterfaceId(
            interface80211ptr->getInterfaceId());
    bsPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    bsPacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);

    sendDelayed(bsPacket, TDMADelayCounter, "ipOut");
    setLeachState(nch);     // Set state for GUI visualization
}

// Selects the ideal CH based on RSSI signal
Ipv4Address Leach::getIdealCH(Ipv4Address nodeAddr, Ipv4Address CHAddr) {
    Ipv4Address tempIdealCHAddr = CHAddr;
    double tempRxPower = 0.0;
    if (nodeMemory.size() > 0) {
        for (auto &it : nodeMemory) {
            if (it.nodeAddr == nodeAddr) {
                if (it.energy > tempRxPower) {
                    tempRxPower = it.energy;
                    tempIdealCHAddr = it.CHAddr;
                    continue;
                }
            } else {
                continue;
            }
        }
    }
    return tempIdealCHAddr;
}

void Leach::refreshDisplay() const {
    const char *icon;
    switch (leachState) {
    case nch:
        icon = "";
        break;
    case ch:
        icon = "status/green";
        break;
    default:
        throw cRuntimeError("Unknown LEACH status");
    }
    auto &displayString = getDisplayString();
    if (*icon) {
        displayString.setTagArg("i2", 0, icon);
        host->getDisplayString().setTagArg("i2", 0, icon);
    } else {
        displayString.removeTag("i2");
        host->getDisplayString().removeTag("i2");
    }
}
} /* namespace leach */
} /* namespace inet */
