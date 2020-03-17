//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/physicallayer/contract/ethernet/EtherPhyCmd_m.h"
#include "inet/physicallayer/ethernet/EtherPhy.h"

namespace inet {
namespace physicallayer {

Define_Module(EtherPhy);

simsignal_t EtherPhy::txStateChangedSignal = registerSignal("txStateChanged");
simsignal_t EtherPhy::txFinishedSignal = registerSignal("txFinished");
simsignal_t EtherPhy::txAbortedSignal = registerSignal("txAborted");
simsignal_t EtherPhy::rxStateChangedSignal = registerSignal("rxStateChanged");
simsignal_t EtherPhy::connectionStateChangedSignal = registerSignal("connectionStateChanged");

std::ostream& operator <<(std::ostream& o, EtherPhy::RxState s)
{
    switch (s) {
        case EtherPhy::RX_OFF_STATE: o << "OFF"; break;
        case EtherPhy::RX_IDLE_STATE: o << "IDLE"; break;
        case EtherPhy::RX_RECEIVING_STATE: o << "RX"; break;
        default: o << (int)s;
    }
    return o;
}

std::ostream& operator <<(std::ostream& o, EtherPhy::TxState s)
{
    switch (s) {
        case EtherPhy::TX_OFF_STATE: o << "OFF"; break;
        case EtherPhy::TX_IDLE_STATE: o << "IDLE"; break;
        case EtherPhy::TX_TRANSMITTING_STATE: o << "TX"; break;
        default: o << (int)s;
    }
    return o;
}

EtherPhy::~EtherPhy()
{
    cancelAndDelete(endTxMsg);
    cancelAndDelete(scheduledTxModifier);
}

void EtherPhy::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        physOutGate = gate("phys$o");
        upperLayerInGate = gate("upperLayerIn");
        handleParameterChange(nullptr);

        // initialize connected flag
        connected = false;
        txTransmissionChannel = rxTransmissionChannel = nullptr;

        // initialize states
        txState = TX_INVALID_STATE;
        rxState = RX_INVALID_STATE;
        lastRxStateChangeTime = simTime();

        // initialize self messages
        endTxMsg = new cMessage("EndTransmission", ENDTRANSMISSION);

        for (int i = 0; i <= RX_LAST; i++)
            totalRxStateTime[i] = SIMTIME_ZERO;

        WATCH(rxState);
        WATCH(txState);

        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        interfaceEntry = getContainingNicModule(this);
        bitrate = interfaceEntry->par("bitrate");   //TODO add a NED parameter reference when will be supported in OMNeT++
        duplexMode = interfaceEntry->par("duplexMode");   //TODO add a NED parameter reference when will be supported in OMNeT++
        if (!duplexMode)
            throw cRuntimeError("half-duplex currently not supported.");
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        if (checkConnected())
            handleConnected();
        else
            handleDisconnected();
        if (!connected)
            EV_WARN << "PHY not connected to a network.\n";
    }
}

void EtherPhy::finish()
{
    changeRxState(RX_OFF_STATE);
    simtime_t t(SIMTIME_ZERO);
    for (int i = 0; i <= RX_LAST; i++)
        t += totalRxStateTime[i];
    if (t > SIMTIME_ZERO) {
        recordScalar("rx channel idle (%)", 100 * (totalRxStateTime[RX_IDLE_STATE] / t));
        recordScalar("rx channel utilization (%)", 100 * (totalRxStateTime[RX_RECEIVING_STATE] / t));
        recordScalar("rx channel off (%)", 100 * (totalRxStateTime[RX_OFF_STATE] / t));
    }
}

void EtherPhy::handleParameterChange(const char *parname)
{
    upperLayerInGate = gate("upperLayerIn");
    displayStringTextFormat = par("displayStringTextFormat");
    sendRawBytes = par("sendRawBytes");
}

void EtherPhy::changeTxState(TxState newState)
{
    if (newState != txState) {
        txState = newState;
        emit(txStateChangedSignal, static_cast<intval_t>(newState));
    }
}

void EtherPhy::changeRxState(RxState newState)
{
    simtime_t t = simTime();
    totalRxStateTime[rxState] += t - lastRxStateChangeTime;
    lastRxStateChangeTime = t;
    if (newState != rxState) {
        rxState = newState;
        emit(rxStateChangedSignal, static_cast<intval_t>(newState));
    }
}

void EtherPhy::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        handleSelfMessage(message);
    }
    else if (connected) {
        if (message->getArrivalGate() == upperLayerInGate) {
            processMsgFromUpperLayer(message);
        }
        else if (message->getArrivalGate() == physInGate) {
            receiveFromMedium(message);
        }
        else
            throw cRuntimeError("Received unknown message");
    }
    else {
        EV_ERROR << "Message " << message << " arrived when PHY disconnected, dropped\n";
        delete message;
    }
}

void EtherPhy::handleSelfMessage(cMessage *message)
{
    switch(message->getKind()) {
        case ENDTRANSMISSION:
            ASSERT(message == endTxMsg);
            endTx();
            break;
        case MODIFYTRANSMISSION:
            ASSERT(message == scheduledTxModifier);
            modifyTxProgress(message);
            break;
        default:
            throw cRuntimeError("Unknown self message received! kind = %d", message->getKind());
    }
}

void EtherPhy::processMsgFromUpperLayer(cMessage *message)
{
    switch (message->getKind()) {
        case ETH_CMD_SEND:
        case ETH_CMD_SEND_PREEMPTABLE:
        {
            auto packet = check_and_cast<Packet *>(message);
            auto signal = encapsulate(packet);
            startTx(signal);
            break;
        }
        case ETH_CMD_MODIFY_CURRENT_PREEMPTABLE:
            modifyCurrentPreemptableTx(message);
            break;
        default:
            throw cRuntimeError("Invalid message kind: %d", message->getKind());
    }
}

void EtherPhy::modifyCurrentPreemptableTx(cMessage *message)
{
    if (curTx == nullptr)
        throw cRuntimeError("Module doesn't have a modifiable transmission");

    if (auto signal = dynamic_cast<EthernetSignal*>(curTx)) {
        auto oldPacket = check_and_cast<Packet*>(signal->getEncapsulatedPacket());
        const auto& phyHeader = oldPacket->peekAtFront<EthernetPhyHeader>();
        auto type = phyHeader->getPreambleType();
        if (type == SMD_Sx || type == SMD_Cx) {
            auto newPacket = check_and_cast<Packet *>(message);
            auto req = newPacket->getTag<PreemptionModifyReq>();
            b signalFirstChangedBitPosition = PREAMBLE_BYTES + SFD_BYTES + req->getFirstChangedOffset();
            simtime_t timePosition = calculateDuration(signalFirstChangedBitPosition, curTx->getBitrate());
            if (curTxStartTime + timePosition < simTime())
                throw cRuntimeError("CMD_MODIFY_CURRENT_PREEMPTABLE message arrived too later");

            simtime_t scheduleTo = curTxStartTime + timePosition;
            newPacket->setKind(MODIFYTRANSMISSION);
            if (scheduledTxModifier != nullptr) {
                ASSERT(scheduledTxModifier->isScheduled());
                if (scheduleTo > scheduledTxModifier->getArrivalTime())
                    scheduleTo = scheduledTxModifier->getArrivalTime();
                cancelAndDelete(scheduledTxModifier);
            }
            scheduleAt(scheduleTo, newPacket);
            scheduledTxModifier = newPacket;
            return;
        }
    }
    throw cRuntimeError("current Tx EthernetSignal doesn't preemptable");
}

void EtherPhy::modifyTxProgress(cMessage *message)
{
    ASSERT(scheduledTxModifier == message);
    scheduledTxModifier = nullptr;
    auto newPacket = check_and_cast<Packet *>(message);
    auto req = newPacket->removeTag<PreemptionModifyReq>();
    b signalFirstChangedBitPosition = PREAMBLE_BYTES + SFD_BYTES + req->getFirstChangedOffset();
    simtime_t timePosition = calculateDuration(signalFirstChangedBitPosition, curTx->getBitrate());
    ASSERT(curTxStartTime + timePosition == simTime());

    auto oldPacket = curTx->decapsulate();
    //FIXME when will be deleted the oldPacket?

    curTx->encapsulate(newPacket);
    auto duration = calculateDuration(curTx);
    sendPacketProgress(curTx, physOutGate, duration, signalFirstChangedBitPosition.get(), timePosition);
}

bool EtherPhy::checkConnected()
{
    bool newConn = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

    if (newConn) {
        auto outChannel = physOutGate->findTransmissionChannel();
        auto inChannel = physInGate->findIncomingTransmissionChannel();
        newConn = inChannel && outChannel && !inChannel->isDisabled() && !outChannel->isDisabled();
    }
    return newConn;
}

void EtherPhy::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

//    MacProtocolBase::receiveSignal(source, signalID, obj, details);

    if (signalID == PRE_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPrePathCutNotification *>(obj)) {
            if (connected && ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))) {
                handleDisconnected();
            }
        }
        else if (auto gcobj = dynamic_cast<cPreParameterChangeNotification *>(obj)) {
            if (connected
                    && (gcobj->par->getOwner() == txTransmissionChannel || gcobj->par->getOwner() == rxTransmissionChannel)
                    && gcobj->par->getType() == cPar::BOOL
                    && strcmp(gcobj->par->getName(), "disabled") == 0
                    /* && gcobj->newValue == true */ //TODO the new value of parameter currently unavailable
                    ) {
                handleDisconnected();
            }
        }
    }
    else if (signalID == POST_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPostPathCreateNotification *>(obj)) {
            if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate)) {
                if (checkConnected())
                    handleConnected();
            }
        }
        else if (auto gcobj = dynamic_cast<cPostParameterChangeNotification *>(obj)) {
            (void)gcobj;
            if (checkConnected())
                handleConnected();
            else
                handleDisconnected();
        }
    }
}

void EtherPhy::handleConnected()
{
    if (!connected) {
        connected = true;
        txTransmissionChannel = physOutGate->getTransmissionChannel();
        if (!txTransmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
            txTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        rxTransmissionChannel = physInGate->getIncomingTransmissionChannel();
        if (!rxTransmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
            rxTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);

        //TODO Should accep asymmetric settings - changed on tx channel first, than changed on rx channel or vica versa
        if (rxTransmissionChannel->hasPar("delay") && txTransmissionChannel->hasPar("delay")) {
            double rxDelay = rxTransmissionChannel->par("delay");
            double txDelay = txTransmissionChannel->par("delay");
            if (rxDelay != txDelay)
                throw cRuntimeError("The delay parameters on tx and rx transmission channels are differ %g vs %g", txDelay, rxDelay);
        }
        else
            throw cRuntimeError("the tx/rx transmission channels doesn't have delay parameter");    // TODO delay needed both tx/rx

        if (rxTransmissionChannel->hasPar("datarate") && txTransmissionChannel->hasPar("datarate")) {
            bitrate = txTransmissionChannel->par("datarate");
            double rxBitrate = rxTransmissionChannel->par("datarate");
            if (bitrate != rxBitrate)
                throw cRuntimeError("The datarate parameters on tx and rx transmission channels are differ %g vs %g", bitrate, rxBitrate);
            interfaceEntry->par("bitrate").setDoubleValue(bitrate);
        }
        else if (!rxTransmissionChannel->hasPar("datarate") && !txTransmissionChannel->hasPar("datarate")) {
            // channels doesn't have datarate parameters
        }
        else
            throw cRuntimeError("asymmetric settings: only one channel has datarate parameter on tx/rx transmission channels");

        changeTxState(TX_IDLE_STATE);
        changeRxState(RX_IDLE_STATE);
        emit(connectionStateChangedSignal, 1);
        interfaceEntry->setCarrier(true);
    }
}

void EtherPhy::handleDisconnected()
{
    if (connected) {
        abortTx();
        abortRx();
        connected = false;
        txTransmissionChannel = physOutGate->findTransmissionChannel();
        changeTxState(TX_OFF_STATE);
        rxTransmissionChannel = physInGate->findIncomingTransmissionChannel();
        changeRxState(RX_OFF_STATE);
        emit(connectionStateChangedSignal, 0);
        interfaceEntry->setCarrier(false);
    }
}

EthernetSignal *EtherPhy::encapsulate(Packet *packet)
{
    auto phyHeader = makeShared<EthernetPhyHeader>();
    PreemptionReq *req = nullptr;
    switch (packet->getKind()) {
        case ETH_CMD_SEND:
            phyHeader->setPreambleType(SFD);
            break;
        case ETH_CMD_SEND_PREEMPTABLE:
            req = packet->removeTag<PreemptionReq>();
            phyHeader->setPreambleType(req->isFirst() ? SMD_Sx : SMD_Cx);
            phyHeader->setFragId(req->getFragId());
            phyHeader->setFragCount(req->getFragCount());
            break;
        case ETH_CMD_MODIFY_CURRENT_PREEMPTABLE:
            throw cRuntimeError("Model Error");
        default:
            throw cRuntimeError("Invalid message kind: %d", packet->getKind());
    }
    packet->insertAtFront(phyHeader);
    packet->clearTags();
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
    packet->setKind(0);
    auto signal = new EthernetSignal(packet->getName());
    signal->setSrcMacFullDuplex(duplexMode);
    signal->setBitrate(bitrate);
    signal->encapsulate(packet);
    return signal;
}

simtime_t EtherPhy::calculateDuration(EthernetSignalBase *signal) const
{
    return signal->getBitLength() / signal->getBitrate();
}

simtime_t EtherPhy::calculateDuration(b length, double bitrate)
{
    return length.get() / bitrate;
}

void EtherPhy::startTx(EthernetSignalBase *signal)
{
    ASSERT(txState == TX_IDLE_STATE);
    ASSERT(curTx == nullptr);

    curTx = signal;
    curTxStartTime = simTime();
    auto duration = calculateDuration(curTx);
    sendPacketStart(curTx, physOutGate, duration);
    ASSERT(txTransmissionChannel->getTransmissionFinishTime() == simTime() + duration);
    scheduleAt(simTime() + duration, endTxMsg);
    changeTxState(TX_TRANSMITTING_STATE);
}

void EtherPhy::endTx()
{
    ASSERT(txState == TX_TRANSMITTING_STATE);
    ASSERT(curTx != nullptr);
    auto duration = calculateDuration(curTx);
    sendPacketEnd(curTx, physOutGate, duration);
    emit(txFinishedSignal, 1);   //TODO
    curTx = nullptr;
    changeTxState(TX_IDLE_STATE);
}

void EtherPhy::abortTx()
{
    if (txState == TX_TRANSMITTING_STATE) {
        ASSERT(curTx != nullptr);
        ASSERT(endTxMsg->isScheduled());
        auto abortTime = simTime();
        txTransmissionChannel->forceTransmissionFinishTime(abortTime);
        cancelEvent(endTxMsg);
        emit(txAbortedSignal, 1);   //TODO
        curTx = nullptr;
    }
    else {
        ASSERT(curTx == nullptr);
        ASSERT(!endTxMsg->isScheduled());
    }
}

EtherPhy::FcsCheckResult EtherPhy::verifyFcs(Packet *packet)
{
    EV_STATICCONTEXT;

    const auto& ethTrailer = packet->peekAtBack<EthernetFcs>(ETHER_FCS_BYTES);          //FIXME can I use any flags?

    switch(ethTrailer->getFcsMode()) {
        case FCS_DECLARED_CORRECT:
            return ethTrailer->getFcs() == 0x0000FFFFL ? MCRC_OK : FCS_OK;
        case FCS_DECLARED_INCORRECT:
            EV_ERROR << "incorrect FCS in ethernet frame\n";
            return FCS_BAD;
        case FCS_COMPUTED: {
            // check the FCS
            auto ethBytes = packet->peekDataAt<BytesChunk>(B(0), packet->getDataLength() - ethTrailer->getChunkLength());
            auto bufferLength = B(ethBytes->getChunkLength()).get();
            auto buffer = new uint8_t[bufferLength];
            // 1. fill in the data
            ethBytes->copyToBuffer(buffer, bufferLength);
            // 2. compute the FCS
            auto computedFcs = ethernetCRC(buffer, bufferLength);
            delete [] buffer;
            if (computedFcs == ethTrailer->getFcs())
                return FCS_OK;
            if ((computedFcs ^ 0x0000FFFFL) == ethTrailer->getFcs())
                return MCRC_OK;
            return FCS_BAD;
        }
        default:
            throw cRuntimeError("invalid FCS mode in ethernet frame");
    }
}

Packet *EtherPhy::decapsulate(EthernetSignal *signal)
{
    auto packet = check_and_cast<Packet *>(signal->decapsulate());
    delete signal;
    auto phyHeader = packet->popAtFront<EthernetPhyHeader>();
    auto fcsCheckResult = verifyFcs(packet);
    if (fcsCheckResult == FCS_BAD) {
        //TODO drop packet
        delete packet;
        return nullptr;
    }

    short int kind = 0;
    switch (phyHeader->getPreambleType()) {
        case SFD: // or SMD_E
            kind = ETH_IND_RECV;
            break;
        case SMD_Verify:
            kind = ETH_IND_RECV_PREEMPTION_VERIFY;
            break;
        case SMD_Respond:
            kind = ETH_IND_RECV_PREEMPTION_RESPONSE;
            break;
        case SMD_Cx:
        case SMD_Sx: {
            auto ind = packet->addTag<PreemptionInd>();
            ind->setIsFirst(phyHeader->getPreambleType() == SMD_Sx);
            ind->setIsLast(true);   //FIXME get from CRC
            ind->setFragId(phyHeader->getFragId());
            ind->setFragCount(phyHeader->getFragCount());
            kind = ETH_IND_RECV_PREEMPTABLE;
            break;
        }
        default:
            //TODO error
            //TODO drop packet
            delete packet;
            return nullptr;
    }

    //FIXME
    // add PreemptionInd if needed, set kind to EtherPhyIndCode


    packet->setKind(kind);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    return packet;
}

void EtherPhy::startRx(EthernetSignalBase *signal)
{
    // only the rx end received in full duplex mode
    if (rxState == RX_IDLE_STATE)
        changeRxState(RX_RECEIVING_STATE);
}

void EtherPhy::endRx(EthernetSignalBase *signal)
{
    if (signal->getSrcMacFullDuplex() != duplexMode)
        throw cRuntimeError("Ethernet misconfiguration: MACs on the same link must be all in full duplex mode, or all in half-duplex mode");
    if (signal->getBitrate() != bitrate)
        throw cRuntimeError("Ethernet misconfiguration: MACs on the same link must be same bitrate %f Mbps (sender:%s, %f Mbps)", bitrate/1e6, signal->getSenderModule()->getFullPath().c_str(), signal->getBitrate()/1e6);

    if (signal->hasBitError())
        ; //TODO

    if (rxState == RX_RECEIVING_STATE) {
        if (auto packet = decapsulate(check_and_cast<EthernetSignal*>(signal)))
            send(packet, "upperLayerOut");
        changeRxState(RX_IDLE_STATE);
    }
    else {
        //TODO
        delete signal;
    }
}

void EtherPhy::abortRx()
{
}

void EtherPhy::receivePacketStart(cPacket *packet)
{
    startRx(check_and_cast<EthernetSignalBase *>(packet));
}

void EtherPhy::receivePacketProgress(cPacket *packet, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration)
{
    throw cRuntimeError("receivePacketProgress not implemented");
}

void EtherPhy::receivePacketEnd(cPacket *packet)
{
    endRx(check_and_cast<EthernetSignalBase *>(packet));
}

bool EtherPhy::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    int stage = operation->getCurrentStage();
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (stage == ModuleStartOperation::STAGE_PHYSICAL_LAYER) {
            handleParameterChange(nullptr);
        }
        else if (stage == ModuleStartOperation::STAGE_LINK_LAYER) {
            if (checkConnected())
                handleConnected();
            else
                handleDisconnected();
            return true;
        }
    }
    else if (dynamic_cast<ModuleStopOperation *>(operation)) {
        if (stage == ModuleStopOperation::STAGE_PHYSICAL_LAYER) {
            handleDisconnected();
            return true;
        }
    }
    else if (dynamic_cast<ModuleCrashOperation *>(operation)) {
        if (stage == ModuleCrashOperation::STAGE_CRASH) {
            handleDisconnected();
            return true;
        }
    }
    else
        throw cRuntimeError("unaccepted Lifecycle operation: (%s)%s", operation->getClassName(), operation->getName());
    return true;
}

} // namespace physicallayer
} // namespace inet

