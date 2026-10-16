//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2011 Thomas Reschka
// Copyright (C) 2011 OpenSim Ltd.
// Copyright (C) 2015 Martin Becke
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <string.h>

#include <algorithm> // min,max

#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/IcmpType_m.h"
#include "inet/networklayer/common/Icmpv4ErrorTag_m.h"
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/networklayer/common/Icmpv6Type_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/contract/tcp/TcpSendEorTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpSendMoreTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpTimestampingTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpZerocopyTag_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"
#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

//
// helper functions
//

// A queue of sequence numbers (the record boundaries of MSG_EOR, the PSH marks) is in sequence order.
// It is a queue, not a std::set, because the sequence numbers wrap around.
static void removeSeqNumsUpTo(std::deque<uint32_t>& seqNums, uint32_t seq)
{
    while (!seqNums.empty() && seqLE(seqNums.front(), seq))
        seqNums.pop_front();
}

// the first sequence number of the queue after seq, or the end of the queue
static std::deque<uint32_t>::const_iterator findSeqNumAfter(const std::deque<uint32_t>& seqNums, uint32_t seq)
{
    return std::partition_point(seqNums.begin(), seqNums.end(), [&] (uint32_t s) { return seqLE(s, seq); });
}

static void insertSeqNum(std::deque<uint32_t>& seqNums, uint32_t seq)
{
    auto it = findSeqNumAfter(seqNums, seq - 1); // the first at seq or after it
    if (it == seqNums.end() || *it != seq)
        seqNums.insert(it, seq);
}

static bool containsSeqNum(const std::deque<uint32_t>& seqNums, uint32_t seq)
{
    auto it = findSeqNumAfter(seqNums, seq - 1);
    return it != seqNums.end() && *it == seq;
}

const char *TcpConnection::stateName(int state)
{
#define CASE(x)    case x: \
        s = (char *)#x + 6; break
    const char *s = "unknown";
    switch (state) {
        CASE(TCP_S_INIT);
        CASE(TCP_S_CLOSED);
        CASE(TCP_S_LISTEN);
        CASE(TCP_S_SYN_SENT);
        CASE(TCP_S_SYN_RCVD);
        CASE(TCP_S_ESTABLISHED);
        CASE(TCP_S_CLOSE_WAIT);
        CASE(TCP_S_LAST_ACK);
        CASE(TCP_S_FIN_WAIT_1);
        CASE(TCP_S_FIN_WAIT_2);
        CASE(TCP_S_CLOSING);
        CASE(TCP_S_TIME_WAIT);
    }
    return s;
#undef CASE
}

const char *TcpConnection::eventName(int event)
{
#define CASE(x)    case x: \
        s = (char *)#x + 6; break
    const char *s = "unknown";
    switch (event) {
        CASE(TCP_E_IGNORE);
        CASE(TCP_E_OPEN_ACTIVE);
        CASE(TCP_E_OPEN_PASSIVE);
        CASE(TCP_E_ACCEPT);
        CASE(TCP_E_SEND);
        CASE(TCP_E_CLOSE);
        CASE(TCP_E_ABORT);
        CASE(TCP_E_DESTROY);
        CASE(TCP_E_STATUS);
        CASE(TCP_E_QUEUE_BYTES_LIMIT);
        CASE(TCP_E_READ);
        CASE(TCP_E_SETOPTION);
        CASE(TCP_E_RCV_DATA);
        CASE(TCP_E_RCV_ACK);
        CASE(TCP_E_RCV_SYN);
        CASE(TCP_E_RCV_SYN_ACK);
        CASE(TCP_E_RCV_FIN);
        CASE(TCP_E_RCV_FIN_ACK);
        CASE(TCP_E_RCV_RST);
        CASE(TCP_E_RCV_UNEXP_SYN);
        CASE(TCP_E_TIMEOUT_2MSL);
        CASE(TCP_E_TIMEOUT_CONN_ESTAB);
        CASE(TCP_E_TIMEOUT_FIN_WAIT_2);
    }
    return s;
#undef CASE
}

const char *TcpConnection::indicationName(int code)
{
#define CASE(x)    case x: \
        s = (char *)#x + 6; break
    const char *s = "unknown";
    switch (code) {
        CASE(TCP_I_DATA);
        CASE(TCP_I_URGENT_DATA);
        CASE(TCP_I_AVAILABLE);
        CASE(TCP_I_ESTABLISHED);
        CASE(TCP_I_PEER_CLOSED);
        CASE(TCP_I_CLOSED);
        CASE(TCP_I_CONNECTION_REFUSED);
        CASE(TCP_I_CONNECTION_RESET);
        CASE(TCP_I_TIMED_OUT);
        CASE(TCP_I_STATUS);
        CASE(TCP_I_SEND_MSG);
        CASE(TCP_I_ICMPv4_ERROR);
        CASE(TCP_I_ICMPv6_ERROR);
    }
    return s;
#undef CASE
}

const char *TcpConnection::optionName(int option)
{
    switch (option) {
        case TCPOPTION_END_OF_OPTION_LIST:
            return "EOL";

        case TCPOPTION_NO_OPERATION:
            return "NOP";

        case TCPOPTION_MAXIMUM_SEGMENT_SIZE:
            return "MSS";

        case TCPOPTION_WINDOW_SCALE:
            return "WS";

        case TCPOPTION_SACK_PERMITTED:
            return "SACK_PERMITTED";

        case TCPOPTION_SACK:
            return "SACK";

        case TCPOPTION_TIMESTAMP:
            return "TS";

        case TCPOPTION_TCP_FASTOPEN:
            return "FAST_OPEN";

        case TCPOPTION_ACCECN0:
            return "ACCECN0";

        case TCPOPTION_ACCECN1:
            return "ACCECN1";

        default:
            return "unknown";
    }
}

void TcpConnection::printConnBrief() const
{
    EV_DETAIL << "Connection "
              << localAddr << ":" << localPort << " to " << remoteAddr << ":" << remotePort
              << "  on socketId=" << socketId
              << "  in " << stateName(fsm.getState())
              << "\n";
}

void TcpConnection::printSegmentBrief(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader)
{
    EV_STATICCONTEXT;
    EV_INFO << "." << tcpHeader->getSrcPort() << " > ";
    EV_INFO << "." << tcpHeader->getDestPort() << ": ";

    if (tcpHeader->getSynBit())
        EV_INFO << (tcpHeader->getAckBit() ? "SYN+ACK " : "SYN ");

    if (tcpHeader->getFinBit())
        EV_INFO << "FIN(+ACK) ";

    if (tcpHeader->getRstBit())
        EV_INFO << (tcpHeader->getAckBit() ? "RST+ACK " : "RST ");

    if (tcpHeader->getPshBit())
        EV_INFO << "PSH ";

    auto payloadLength = tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>();
    if (payloadLength > 0 || tcpHeader->getSynBit()) {
        EV_INFO << "[" << tcpHeader->getSequenceNo() << ".." << (tcpHeader->getSequenceNo() + payloadLength) << ") ";
        EV_INFO << "(l=" << payloadLength << ") ";
    }
    else
        EV_INFO << "seq " << tcpHeader->getSequenceNo() << " ";

    if (tcpHeader->getAckBit())
        EV_INFO << "ack " << tcpHeader->getAckNo() << " ";

    EV_INFO << "win " << tcpHeader->getWindow() << " ";

    if (tcpHeader->getUrgBit())
        EV_INFO << "urg " << tcpHeader->getUrgentPointer() << " ";

    if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH) { // Header options present?
        EV_INFO << "options ";

        for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
            const TcpOption *option = tcpHeader->getHeaderOption(i);
            short kind = option->getKind();
            EV_INFO << optionName(kind) << " ";
        }
    }
    EV_INFO << "\n";
}

void TcpConnection::initClonedConnection(TcpConnection *listenerConn)
{
    Enter_Method("initClonedConnection");
    listeningSocketId = listenerConn->getSocketId();

    // following code to be kept consistent with initConnection()
    const char *sendQueueClass = listenerConn->sendQueue->getClassName();
    sendQueue = check_and_cast<TcpSendQueue *>(inet::utils::createOne(sendQueueClass));
    sendQueue->setConnection(this);

    const char *receiveQueueClass = listenerConn->receiveQueue->getClassName();
    receiveQueue = check_and_cast<TcpReceiveQueue *>(inet::utils::createOne(receiveQueueClass));
    receiveQueue->setConnection(this);

    // create SACK retransmit queue
    rexmitQueue = new TcpSackRexmitQueue();
    rexmitQueue->setConnection(this);

    tcpAlgorithm = createTcpAlgorithm(listenerConn->tcpAlgorithm->getNedTypeName());

    socketAdvertisedWindow = listenerConn->socketAdvertisedWindow;
    socketReceiveBufferSize = listenerConn->socketReceiveBufferSize;
    socketSynRetries = listenerConn->socketSynRetries;
    keepAliveSockopt = listenerConn->keepAliveSockopt;
    noDelaySockopt = listenerConn->noDelaySockopt;
    maxSegSockopt = listenerConn->maxSegSockopt;
    pathMtuSockopt = listenerConn->pathMtuSockopt;
    corkSockopt = listenerConn->corkSockopt;
    rcvBufSockopt = listenerConn->rcvBufSockopt;
    state = tcpAlgorithm->getStateVariables();
    configureStateVariables();
    tcpAlgorithm->initializeAlgorithm();
    if (keepAliveSockopt >= 0)
        tcpAlgorithm->setKeepAlive(keepAliveSockopt != 0);

    // put it into LISTEN, with our localAddr/localPort
    state->active = false;
    state->fork = true;
    state->forked = true;
    localAddr = listenerConn->localAddr;
    localPort = listenerConn->localPort;
    autoRead = listenerConn->autoRead;

    FSM_Goto(fsm, TCP_S_LISTEN);
}

TcpConnection *TcpConnection::cloneListeningConnection()
{
    auto moduleType = cModuleType::get("inet.transportlayer.tcp.TcpConnection");
    int newSocketId = getActiveSimulationOrEnvir()->getUniqueNumber();
    char submoduleName[24];
    snprintf(submoduleName, sizeof(submoduleName), "conn-%d", newSocketId);
    auto conn = check_and_cast<TcpConnection *>(moduleType->createScheduleInit(submoduleName, tcpMain));
    conn->initConnection(tcpMain, newSocketId);
    conn->initClonedConnection(this);
    return conn;
}

int TcpConnection::receivedEcnCodepoint(Packet *tcpSegment)
{
    auto ecnInd = tcpSegment->findTag<EcnInd>();
    return ecnInd != nullptr ? ecnInd->getExplicitCongestionNotification() : IP_ECN_NOT_ECT;
}

uint8_t TcpConnection::accEcnReflectedAce(int ipEcnCodepoint)
{
    // RFC 9768 Tables 2 and 3: Not-ECT -> 0b010, ECT(1) -> 0b011, ECT(0) -> 0b100, CE -> 0b110
    switch (ipEcnCodepoint) {
        case IP_ECN_ECT_1: return 3;
        case IP_ECN_ECT_0: return 4;
        case IP_ECN_CE: return 6;
        default: return 2;
    }
}

bool TcpConnection::getAccEcnOptionFields(int& order, int& wanted, int& required) const
{
    if (!state->accEcnNegotiated || !state->accEcnOptionEnabled || state->accEcnOptFailSend)
        return false;

    // the counters that changed since the last option, and those that ever changed
    // from their initial values (RFC 9768 section 3.2.1)
    bool e0Changed = state->rcvEct0Bytes != state->accEcnOptEct0Bytes;
    bool ceChanged = state->rcvCeBytes != state->accEcnOptCeBytes;
    bool e1Changed = state->rcvEct1Bytes != state->accEcnOptEct1Bytes;
    bool e0Ever = state->rcvEct0Bytes != 1;
    bool ceEver = state->rcvCeBytes != 0;
    bool e1Ever = state->rcvEct1Bytes != 1;
    // the fields that an option of order 0 (EE0B, ECEB, EE1B) or of order 1 (EE1B,
    // ECEB, EE0B) needs for the given counters (Table 5)
    auto fields = [] (int order, bool e0, bool ce, bool e1) {
        return (order == 0 ? e1 : e0) ? 3 : ce ? 2 : (order == 0 ? e0 : e1) ? 1 : 0;
    };

    if (tcpMain->accEcnOptionLinux) {
        // Linux (tcp_established_options(), tcp_ecn_option=2): each segment, once the
        // peer sent an option; kind 174 with each field that fits, but at least the
        // fields that changed
        if (!state->sawAccEcnOpt)
            return false;
        order = 1;
        wanted = 3;
        required = fields(1, e0Changed, ceChanged, e1Changed);
        return true;
    }

    // section 3.2.3.2.1: the client "SHOULD include an AccECN Option in the first ACK
    // at the end of the three-way handshake", and "SHOULD also include an AccECN
    // Option on the first data segment it sends"; with each field that fits
    if (state->active && !state->accEcnFirstDataOptionSent) {
        order = 0;
        wanted = 3;
        required = fields(0, e0Changed, ceChanged, e1Changed);
        return true;
    }

    // section 3.2.3.3, the recommended simple scheme: an option "on every scheduled ACK
    // if any byte counter has incremented since the last ACK", here since the last
    // option, which also covers an option that did not fit; "Whenever possible, it
    // SHOULD include a field for every byte counter that has changed at some time
    // during the connection". Of the two orders, the one with fewer fields.
    if (!e0Changed && !ceChanged && !e1Changed)
        return false;
    order = fields(1, e0Ever, ceEver, e1Ever) < fields(0, e0Ever, ceEver, e1Ever) ? 1 : 0;
    wanted = fields(order, e0Ever, ceEver, e1Ever);
    required = fields(order, e0Changed, ceChanged, e1Changed);
    return true;
}

unsigned int TcpConnection::getAccEcnOptionSpaceUsed(const Ptr<TcpHeader>& tcpHeader) const
{
    if (!tcpMain->accEcnOptionLinux)
        return tcpHeader->getHeaderOptionArrayLength().get<B>();
    // Linux decides the fields of the option with its own layout (tcp_established_options()):
    // NOP, NOP, TS (12 bytes) and NOP, NOP, SACK (4 + 8 bytes for each block); INET can pack
    // them tighter, which would give an option with more fields than Linux sends
    unsigned int used = 0;
    for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        switch (option->getKind()) {
            case TCPOPTION_END_OF_OPTION_LIST:
            case TCPOPTION_NO_OPERATION: break;
            case TCPOPTION_TIMESTAMP: used += 12; break;
            case TCPOPTION_SACK: used += 2 + option->getLength(); break;
            default: used += (option->getLength() + 3) & ~3u; break;
        }
    }
    return used;
}

bool TcpConnection::accEcnOptionFits(unsigned int used, int numFields, bool sack) const
{
    unsigned int length = 2 + 3 * numFields;
    if (tcpMain->accEcnOptionLinux) {
        // Linux (tcp_options_fit_accecn()): the option can take the 2 NOP bytes before
        // a SACK option, so it rounds down to 4 bytes if that saves the odd bytes
        unsigned int saving = sack ? 2 : 0;
        unsigned int cost = (length & 3) > saving ? (length + 3) & ~3u : length & ~3u;
        return used + cost <= TCP_OPTIONS_MAX_SIZE.get<B>();
    }
    return ((used + length + 3) & ~3u) <= TCP_OPTIONS_MAX_SIZE.get<B>();
}

void TcpConnection::appendAccEcnOption(const Ptr<TcpHeader>& tcpHeader, int order, int wanted, int required)
{
    // RFC 9768 section 3.2.3.3: the option "MAY be truncated to omit one or two fields
    // from the end of the option ..., provided that the counter(s) that have changed
    // since the previous AccECN TCP Option are not omitted. If there is insufficient
    // space ..., then the entire AccECN TCP Option MUST be omitted."
    bool sack = false;
    for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++)
        if (tcpHeader->getHeaderOption(i)->getKind() == TCPOPTION_SACK)
            sack = true;
    unsigned int used = getAccEcnOptionSpaceUsed(tcpHeader);
    int numFields = wanted;
    while (numFields >= required && !accEcnOptionFits(used, numFields, sack))
        numFields--;
    if (numFields < required) {
        EV_INFO << "Tcp Header Option AccECN not sent: no space for the " << required << " fields that changed\n";
        return;
    }
    unsigned int length = 2 + 3 * numFields;
    if (tcpMain->alignOptions) {
        unsigned int realUsed = tcpHeader->getHeaderOptionArrayLength().get<B>();
        while ((realUsed + length) % 4 != 0) {
            tcpHeader->appendHeaderOption(new TcpOptionNop());
            realUsed++;
        }
    }
    auto *option = new TcpOptionAccEcn();
    option->setKind(order == 0 ? TCPOPTION_ACCECN0 : TCPOPTION_ACCECN1);
    option->setLength(length);
    // section 3.2.3.1: "the 24 least significant bits of the current value of the
    // associated counter"
    option->setEct0Bytes(state->rcvEct0Bytes & 0xFFFFFF);
    option->setCeBytes(state->rcvCeBytes & 0xFFFFFF);
    option->setEct1Bytes(state->rcvEct1Bytes & 0xFFFFFF);
    tcpHeader->appendHeaderOption(option);
    EV_INFO << "Tcp Header Option AccECN" << order << " (" << numFields << " fields, E0B=" << option->getEct0Bytes()
            << ", CEB=" << option->getCeBytes() << ", E1B=" << option->getEct1Bytes() << ") sent\n";
}

void TcpConnection::sendToIP(Packet *tcpSegment, const Ptr<TcpHeader>& tcpHeader)
{
    sentSegments++;
    lastSentAck = tcpHeader->getAckNo();

    // record seq (only if we do send data) and ackno
    if (tcpSegment->getByteLength() > tcpHeader->getChunkLength().get<B>())
        emit(sndNxtSignal, tcpHeader->getSequenceNo());

    emit(sndAckSignal, tcpHeader->getAckNo());

    // final touches on the segment before sending
    tcpHeader->setSrcPort(localPort);
    tcpHeader->setDestPort(remotePort);
    ASSERT(tcpHeader->getHeaderLength() >= TCP_MIN_HEADER_LENGTH);
    ASSERT(tcpHeader->getHeaderLength() <= TCP_MAX_HEADER_LENGTH);
    ASSERT(tcpHeader->getChunkLength() == tcpHeader->getHeaderLength());

    EV_INFO << "Sending: ";
    printSegmentBrief(tcpSegment, tcpHeader);

    // TODO reuse next function for sending

    const IL3AddressType *addressType = remoteAddr.getAddressType();
    tcpSegment->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());

    if (ttl != -1 && tcpSegment->findTag<HopLimitReq>() == nullptr)
        tcpSegment->addTag<HopLimitReq>()->setHopLimit(ttl);

    if (dscp != -1 && tcpSegment->findTag<DscpReq>() == nullptr)
        tcpSegment->addTag<DscpReq>()->setDifferentiatedServicesCodePoint(dscp);

    if (tos != -1 && tcpSegment->findTag<TosReq>() == nullptr)
        tcpSegment->addTag<TosReq>()->setTos(tos);

    // PMTUD (RFC 1191): set Don't Fragment bit on IPv4 segments
    if (state->pmtudEnabled && remoteAddr.getType() == L3Address::IPv4)
        tcpSegment->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    auto addresses = tcpSegment->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(localAddr);
    addresses->setDestAddress(remoteAddr);

    // AccECN (RFC 9768 section 3.2.2): after the handshake, AE, CWR and ECE of each
    // segment with an ACK form the ACE field, the number of CE-marked packets
    // received (r.cep, which starts at 5), mod 8. The pure ACK of the SYN-ACK feeds
    // back the IP ECN codepoint of the SYN-ACK instead (section 3.2.2.1, Table 3):
    // "If the TCP Client (A) in AccECN mode uses a pure ACK with no SACK blocks to
    // acknowledge the SYN/ACK, it MUST use the binary encoding in Table 3". The
    // ACKs that TIME_WAIT sends carry no ACE field in Linux (tcp_v4_send_ack()),
    // selected by accEcnBareTimeWaitAck.
    if (state->accEcnNegotiated && tcpHeader->getAckBit() && !tcpHeader->getSynBit()
        && !(tcpMain->accEcnBareTimeWaitAck && the2MSLTimer != nullptr && the2MSLTimer->isScheduled()))
    {
        bool pureAck = tcpSegment->getByteLength() == 0;
        bool sackBlocks = false;
        for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++)
            if (tcpHeader->getHeaderOption(i)->getKind() == TCPOPTION_SACK)
                sackBlocks = true;
        uint8_t ace;
        if (state->accEcnReflectAce && pureAck && !sackBlocks) {
            ace = accEcnReflectedAce(state->accEcnReflectCodepoint);
            state->accEcnReflectAce = false;
        }
        else
            ace = (state->rcvCePkts + 5) & 7;
        tcpHeader->setAeBit((ace & 4) != 0);
        tcpHeader->setCwrBit((ace & 2) != 0);
        tcpHeader->setEceBit((ace & 1) != 0);
    }

    // AccECN option (RFC 9768 section 3.2.3.3): remember the counts that it reported,
    // so that the next option knows the counters that changed since. A data segment
    // with the option ends the extra options of the client (section 3.2.3.2.1). As
    // Linux (tcp_options_write()), remember also whether the option went out with a
    // D-SACK block, the first SACK block, below the acknowledgment number (RFC 2883).
    bool accEcnOptionSent = false;
    const TcpOptionSack *sackOption = nullptr;
    for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_ACCECN0 || option->getKind() == TCPOPTION_ACCECN1) {
            accEcnOptionSent = true;
            state->accEcnOptEct0Bytes = state->rcvEct0Bytes;
            state->accEcnOptCeBytes = state->rcvCeBytes;
            state->accEcnOptEct1Bytes = state->rcvEct1Bytes;
            if (tcpSegment->getByteLength() > 0)
                state->accEcnFirstDataOptionSent = true;
        }
        else if (option->getKind() == TCPOPTION_SACK)
            sackOption = check_and_cast<const TcpOptionSack *>(option);
    }
    if (state->accEcnNegotiated) {
        state->accEcnOptSentWithDsack = accEcnOptionSent && sackOption != nullptr && sackOption->getSackItemArraySize() > 0
                && seqLess(sackOption->getSackItem(0).getStart(), tcpHeader->getAckNo());
        if (state->accEcnOptSentWithDsack)
            state->accEcnSentDsackStart = sackOption->getSackItem(0).getStart();
    }

    // ECN: the ECN capable transport codepoint is ECT(0).
    //
    // RFC 3168, page 6:
    // "Routers treat the ECT(0) and ECT(1) codepoints
    // as equivalent.  Senders are free to use either the ECT(0) or the
    // ECT(1) codepoint to indicate ECT."
    //
    // RFC 8311, section 3, which gives ECT(1) to experiments (L4S, RFC 9331):
    // "A network node MUST NOT originate traffic marked with ECT(1) unless the
    // network node is participating in a Congestion Marking Differences
    // experiment that uses ECT(1)"
    //
    // rfc-3168, page 20:
    // For the current generation of TCP congestion control algorithms, pure
    // acknowledgement packets (e.g., packets that do not contain any
    // accompanying data) MUST be sent with the not-ECT codepoint.
    //
    // rfc-3168, page 20:
    // ECN-capable TCP implementations MUST NOT set either ECT codepoint
    // (ECT(0) or ECT(1)) in the IP header for retransmitted data packets
    //
    // RFC 3168, section 6.1.1: "A host MUST NOT set ECT on SYN or SYN-ACK packets."
    // RFC 3168, section 6.1.6: "the TCP data sender MUST NOT set either an ECT
    // codepoint or the CWR bit on window probe packets."
    // So ECT goes only on new data, also not on a FIN without data.
    //
    // DCTCP, RFC 8257 section 3.6: "for DCTCP connections, the sender SHOULD set ECT
    // for SYN, SYN-ACK, and RST packets", as Linux does for a flavour that needs ECN:
    // on an ECN-setup SYN, and on the SYN-ACK of an ECN connection. ecnMarkAll sets
    // ECT also on the other segments of an AccECN or DCTCP connection, as Linux
    // (tcp_ecn_send()); RFC 8257 section 4.1: "It is RECOMMENDED that an
    // implementation provide a configuration knob that will cause ECT to be set on
    // such control packets".
    bool needsEcn = tcpAlgorithm != nullptr && tcpAlgorithm->needsEcn();
    bool markEct;
    if (tcpHeader->getSynBit())
        markEct = needsEcn && (tcpHeader->getAckBit() ? state->ect : tcpHeader->getEceBit());
    else if (tcpMain->ecnMarkAll && (state->accEcnNegotiated || needsEcn))
        markEct = state->ect;
    else
        markEct = state->ect && tcpSegment->getByteLength() > 0 && !state->rexmit && !sendingWindowProbe;
    tcpSegment->addTagIfAbsent<EcnReq>()->setExplicitCongestionNotification(markEct ? IP_ECN_ECT_0 : IP_ECN_NOT_ECT);

    tcpHeader->setChecksum(0);
    tcpHeader->setChecksumMode(tcpMain->checksumMode);

    insertTransportProtocolHeader(tcpSegment, Protocol::tcp, tcpHeader);

    tcpMain->sendToIp(tcpSegment);
}

void TcpConnection::sendToIP(Packet *tcpSegment, const Ptr<TcpHeader>& tcpHeader, L3Address src, L3Address dest)
{
    EV_STATICCONTEXT;
    EV_INFO << "Sending: ";
    printSegmentBrief(tcpSegment, tcpHeader);

    const IL3AddressType *addressType = dest.getAddressType();
    ASSERT(tcpHeader->getChunkLength() == tcpHeader->getHeaderLength());
    tcpSegment->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());

    if (ttl != -1 && tcpSegment->findTag<HopLimitReq>() == nullptr)
        tcpSegment->addTag<HopLimitReq>()->setHopLimit(ttl);

    if (dscp != -1 && tcpSegment->findTag<DscpReq>() == nullptr)
        tcpSegment->addTag<DscpReq>()->setDifferentiatedServicesCodePoint(dscp);

    if (tos != -1 && tcpSegment->findTag<TosReq>() == nullptr)
        tcpSegment->addTag<TosReq>()->setTos(tos);

    auto addresses = tcpSegment->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(src);
    addresses->setDestAddress(dest);

    insertTransportProtocolHeader(tcpSegment, Protocol::tcp, tcpHeader);

    tcpMain->sendToIp(tcpSegment);
}

void TcpConnection::signalConnectionTimeout()
{
    sendIndicationToApp(TCP_I_TIMED_OUT);
}

void TcpConnection::sendIndicationToApp(int code, const int id)
{
    EV_INFO << "Notifying app: " << indicationName(code) << "\n";
    auto indication = new Indication(indicationName(code), code);
    TcpCommand *ind = new TcpCommand();
    ind->setUserId(id);
    indication->addTag<SocketInd>()->setSocketId(socketId);
    indication->setControlInfo(ind);
    sendToApp(indication);
}

bool TcpConnection::processIcmpv4Error(Indication *indication)
{
    Enter_Method("processIcmpv4Error");
    take(indication);

    auto& errorInd = indication->getTag<Icmpv4ErrorInd>();

    EV_WARN << "ICMPv4 error for connection " << localAddr << ":" << localPort
            << " > " << remoteAddr << ":" << remotePort
            << " type=" << errorInd->getType() << " code=" << errorInd->getCode() << "\n";

    if (!isIcmpErrorInWindow(errorInd->getOriginalPacket())) {
        delete indication;
        return true;
    }

    // Hard errors abort the connection during setup (RFC 5461).
    // Once ESTABLISHED, even hard errors are treated as soft to prevent
    // blind reset attacks.
    // RFC 9293 section 3.9.2.2: a hard error SHOULD abort the connection (SHLD-26), also
    // in SYN_RCVD; a connection that is neither active nor forked nor Fast Open returns to
    // LISTEN, and its application, which has no socket for it yet, is not informed
    // softIcmpErrorsAbortSetup: as Linux tcp_v4_err(), also a soft Destination Unreachable
    // (codes 0, 1 and 5) aborts a connection setup (RFC 5461 section 4)
    bool softAborts = state->softIcmpErrorsAbortSetup && errorInd->getType() == ICMP_DESTINATION_UNREACHABLE
        && (errorInd->getCode() == ICMP_DU_NETWORK_UNREACHABLE || errorInd->getCode() == ICMP_DU_HOST_UNREACHABLE
            || errorInd->getCode() == ICMP_DU_SOURCE_ROUTE_FAILED);
    if ((isHardIcmpv4Error(errorInd->getType(), errorInd->getCode()) || softAborts)
        && (fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD))
    {
        EV_DETAIL << "Hard ICMPv4 error during connection setup -- connection refused\n";

        sendQueue->discardUpTo(sendQueue->getBufferEndSeq());
        if (state->sack_enabled)
            rexmitQueue->discardUpTo(rexmitQueue->getBufferEndSeq());

        if (state->active || state->forked || state->fastopenAccelerated)
            sendIndicationToApp(TCP_I_CONNECTION_REFUSED);
        delete indication;

        return performStateTransition(TCP_E_RCV_RST);
    }

    // PMTUD (RFC 1191): if Fragmentation Needed and DF Set, reduce snd_mss
    if (state->pmtudEnabled && isFragNeeded(errorInd->getType(), errorInd->getCode())) {
        int mtu = errorInd->getMtu();
        if (mtu > 0) {
            // 20 bytes IPv4 header + 20 bytes TCP minimum header
            uint32_t newMss = mtu - 40;
            // mtuProbing: the reported MTU is the route MTU now, which bounds the search
            if (state->mtupEnabled && (uint32_t)mtu < state->pathMtu) {
                state->pathMtu = mtu;
                limitMtuSearch(mtu);
            }
            if (newMss < state->snd_mss) {
                EV_DETAIL << "PMTUD: reducing snd_mss from " << state->snd_mss
                          << " to " << newMss << " (reported MTU=" << mtu << ")\n";
                state->snd_mss = newMss;
                state->snd_effmss = calculateEffectiveMss();
                state->pmtudLastMssReduction = simTime();
                if (fsm.getState() == TCP_S_SYN_SENT)
                    retransmitSynAfterMtuReduction(newMss);
                else
                    retransmitOneSegment(true);
            }
        }
    }

    // Soft notification: forward the original indication to the application, no state change.
    // Don't forward if the connection is in a state where the app may not have
    // the socket registered (would cause MessageDispatcher error).
    int fsmState = fsm.getState();
    if (fsmState != TCP_S_ESTABLISHED && fsmState != TCP_S_FIN_WAIT_1 &&
        fsmState != TCP_S_FIN_WAIT_2 && fsmState != TCP_S_CLOSE_WAIT &&
        fsmState != TCP_S_CLOSING)
    {
        EV_DETAIL << "Ignoring soft ICMPv4 error in " << stateName(fsmState) << " state (app may not have socket registered)\n";
        delete indication;
        return true;
    }

    // The Icmpv4ErrorInd tag is already attached; just relabel and add SocketInd.
    indication->setName(indicationName(TCP_I_ICMPv4_ERROR));
    indication->setKind(TCP_I_ICMPv4_ERROR);
    indication->addTag<SocketInd>()->setSocketId(socketId);
    sendToApp(indication);
    return true;
}

bool TcpConnection::processIcmpv6Error(Indication *indication)
{
    Enter_Method("processIcmpv6Error");
    take(indication);

    auto& errorInd = indication->getTag<Icmpv6ErrorInd>();

    EV_WARN << "ICMPv6 error for connection " << localAddr << ":" << localPort
            << " > " << remoteAddr << ":" << remotePort
            << " type=" << errorInd->getType() << " code=" << errorInd->getCode() << "\n";

    if (!isIcmpErrorInWindow(errorInd->getOriginalPacket())) {
        delete indication;
        return true;
    }

    // Hard errors abort the connection during setup (RFC 5461).
    // Once ESTABLISHED, even hard errors are treated as soft to prevent
    // blind reset attacks.
    // RFC 9293 section 3.9.2.2: a hard error SHOULD abort the connection (SHLD-26), also
    // in SYN_RCVD; a connection that is neither active nor forked nor Fast Open returns to
    // LISTEN, and its application, which has no socket for it yet, is not informed
    // softIcmpErrorsAbortSetup: as Linux tcp_v6_err(), also a soft Destination Unreachable
    // (codes 0 and 3) aborts a connection setup (RFC 5461 section 4)
    bool softAborts = state->softIcmpErrorsAbortSetup && errorInd->getType() == ICMPv6_DESTINATION_UNREACHABLE
        && (errorInd->getCode() == NO_ROUTE_TO_DEST || errorInd->getCode() == ADDRESS_UNREACHABLE);
    if ((isHardIcmpv6Error(errorInd->getType(), errorInd->getCode()) || softAborts)
        && (fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD))
    {
        EV_DETAIL << "Hard ICMPv6 error during connection setup -- connection refused\n";

        sendQueue->discardUpTo(sendQueue->getBufferEndSeq());
        if (state->sack_enabled)
            rexmitQueue->discardUpTo(rexmitQueue->getBufferEndSeq());

        if (state->active || state->forked || state->fastopenAccelerated)
            sendIndicationToApp(TCP_I_CONNECTION_REFUSED);
        delete indication;

        return performStateTransition(TCP_E_RCV_RST);
    }

    // PMTUD (RFC 1981): if Packet Too Big, reduce snd_mss
    if (state->pmtudEnabled && isPacketTooBig(errorInd->getType(), errorInd->getCode())) {
        int mtu = errorInd->getMtu();
        if (mtu > 0) {
            // 40 bytes IPv6 header + 20 bytes TCP minimum header
            uint32_t newMss = mtu - 60;
            // mtuProbing: the reported MTU is the route MTU now, which bounds the search
            if (state->mtupEnabled && (uint32_t)mtu < state->pathMtu) {
                state->pathMtu = mtu;
                limitMtuSearch(mtu);
            }
            if (newMss < state->snd_mss) {
                EV_DETAIL << "PMTUD: reducing snd_mss from " << state->snd_mss
                          << " to " << newMss << " (reported MTU=" << mtu << ")\n";
                state->snd_mss = newMss;
                state->snd_effmss = calculateEffectiveMss();
                state->pmtudLastMssReduction = simTime();
                if (fsm.getState() == TCP_S_SYN_SENT)
                    retransmitSynAfterMtuReduction(newMss);
                else
                    retransmitOneSegment(true);
            }
        }
    }

    // Soft notification: forward the original indication to the application, no state change.
    // Don't forward if the connection is in a closing/closed state where the app
    // may have already cleaned up its socket (would cause MessageDispatcher error).
    int fsmState = fsm.getState();
    if (fsmState != TCP_S_ESTABLISHED && fsmState != TCP_S_FIN_WAIT_1 &&
        fsmState != TCP_S_FIN_WAIT_2 && fsmState != TCP_S_CLOSE_WAIT &&
        fsmState != TCP_S_CLOSING)
    {
        EV_DETAIL << "Ignoring soft ICMPv6 error in " << stateName(fsmState) << " state (app may not have socket registered)\n";
        delete indication;
        return true;
    }

    // The Icmpv6ErrorInd tag is already attached; just relabel and add SocketInd.
    indication->setName(indicationName(TCP_I_ICMPv6_ERROR));
    indication->setKind(TCP_I_ICMPv6_ERROR);
    indication->addTag<SocketInd>()->setSocketId(socketId);
    sendToApp(indication);
    return true;
}

bool TcpConnection::isIcmpErrorInWindow(const Packet *originalPacket) const
{
    // icmpSequenceCheck: as Linux tcp_v4_err() and tcp_v6_err() (RFC 5927 section 4.1), an
    // error that quotes a sequence number outside SND.UNA to SND.MAX is stale or forged,
    // and the connection ignores it
    if (!state->icmpSequenceCheck || originalPacket == nullptr)
        return true;
    const auto& quotedHeader = originalPacket->peekAtFront<TcpHeader>(b(-1), Chunk::PF_ALLOW_INCOMPLETE);
    uint32_t quotedSeq = quotedHeader->getSequenceNo();
    if (seqLE(state->snd_una, quotedSeq) && seqLE(quotedSeq, state->snd_max))
        return true;
    EV_DETAIL << "Ignoring the ICMP error: it quotes sequence number " << quotedSeq << ", outside SND.UNA=" << state->snd_una
              << " to SND.MAX=" << state->snd_max << "\n";
    return false;
}

bool TcpConnection::isHardIcmpv4Error(int type, int code)
{
    // ICMPv4 Destination Unreachable with protocol/port unreachable or admin prohibited
    return type == ICMP_DESTINATION_UNREACHABLE
           && (code == ICMP_DU_PROTOCOL_UNREACHABLE
               || code == ICMP_DU_PORT_UNREACHABLE
               || code == ICMP_DU_COMMUNICATION_PROHIBITED);
}

bool TcpConnection::isHardIcmpv6Error(int type, int code)
{
    // ICMPv6 Destination Unreachable with port unreachable or admin prohibited
    return type == ICMPv6_DESTINATION_UNREACHABLE
           && (code == PORT_UNREACHABLE
               || code == COMM_WITH_DEST_PROHIBITED);
}

bool TcpConnection::isFragNeeded(IcmpType type, int code)
{
    // ICMPv4 Destination Unreachable / Fragmentation Needed and DF Set (RFC 1191)
    return type == ICMP_DESTINATION_UNREACHABLE
           && code == ICMP_DU_FRAGMENTATION_NEEDED;
}

bool TcpConnection::isPacketTooBig(Icmpv6Type type, int code)
{
    // ICMPv6 Packet Too Big (RFC 1981)
    return type == ICMPv6_PACKET_TOO_BIG;
}

void TcpConnection::sendAvailableIndicationToApp()
{
    EV_INFO << "Notifying app: " << indicationName(TCP_I_AVAILABLE) << "\n";
    auto indication = new Indication(indicationName(TCP_I_AVAILABLE), TCP_I_AVAILABLE);
    TcpAvailableInfo *ind = new TcpAvailableInfo();
    ind->setNewSocketId(socketId);
    ind->setLocalAddr(localAddr);
    ind->setRemoteAddr(remoteAddr);
    ind->setLocalPort(localPort);
    ind->setRemotePort(remotePort);
    ind->setAutoRead(autoRead);

    indication->addTag<SocketInd>()->setSocketId(listeningSocketId);
    indication->setControlInfo(ind);
    sendToApp(indication);
}

void TcpConnection::sendEstabIndicationToApp()
{
    EV_INFO << "Notifying app: " << indicationName(TCP_I_ESTABLISHED) << "\n";
    auto indication = new Indication(indicationName(TCP_I_ESTABLISHED), TCP_I_ESTABLISHED);
    TcpConnectInfo *ind = new TcpConnectInfo();
    ind->setLocalAddr(localAddr);
    ind->setRemoteAddr(remoteAddr);
    ind->setLocalPort(localPort);
    ind->setRemotePort(remotePort);
    ind->setAutoRead(autoRead);
    indication->addTag<SocketInd>()->setSocketId(socketId);
    indication->setControlInfo(ind);
    sendToApp(indication);
}

void TcpConnection::sendToApp(cMessage *msg)
{
    tcpMain->sendToApp(msg);
}

void TcpConnection::sendAvailableDataToApp()
{
    if (receiveQueue->getAmountOfBufferedBytes()) {
        if (autoRead || maxByteCountRequested > 0) {
            uint32_t endSeqNo = state->rcv_nxt;
            // rcv_nxt may already be advanced past a received FIN (which
            // occupies a sequence number but has no bytes in the queue) --
            // clamp extraction at the FIN or the queue's range assert trips
            if (state->fin_rcvd && seqLess(state->rcv_fin_seq, endSeqNo))
                endSeqNo = state->rcv_fin_seq;
            if (!autoRead) {
                uint32_t requestedEndPos = receiveQueue->getFirstSeqNo() + maxByteCountRequested;
                if (seqLess(requestedEndPos, endSeqNo))
                    endSeqNo = requestedEndPos;
            }
            while (auto msg = extractDataForApp(endSeqNo)) {
                sendToApp(msg);
                if (!autoRead) {
                    maxByteCountRequested = 0;
                    break;
                }
            }
            // a delivery is a read of the application (D-146)
            adjustRcvSpace();
        }
    }
}

Packet *TcpConnection::extractDataForApp(uint32_t endSeqNo)
{
    // urgentOutOfBand, as Linux tcp_recvmsg() without SO_OOBINLINE: the mark is the last
    // urgent byte (urg_seq); a read stops before it, and at the mark the byte leaves the
    // stream and does not count toward the bytes of the read
    if (state->urgentOutOfBand && state->rcvUrgentPending) {
        uint32_t markSeqNo = state->rcv_up - 1;
        uint32_t firstSeqNo = receiveQueue->getFirstSeqNo();
        if (firstSeqNo == markSeqNo && receiveQueue->getAcknowledgedDataLength() > 0) {
            state->rcvUrgentPending = false;
            Packet *oobByte = receiveQueue->extractBytesUpTo(markSeqNo + 1);
            releaseRcvBuffer(oobByte->getByteLength());
            delete oobByte;
            EV_INFO << "Urgent data: the out-of-band byte at " << markSeqNo << " leaves the stream\n";
            uint32_t dataEndSeqNo = receiveQueue->getFirstSeqNo() + receiveQueue->getAcknowledgedDataLength();
            endSeqNo = seqLess(endSeqNo + 1, dataEndSeqNo) ? endSeqNo + 1 : dataEndSeqNo;
        }
        else if (seqLess(firstSeqNo, markSeqNo) && seqLess(markSeqNo, endSeqNo))
            endSeqNo = markSeqNo;
    }
    Packet *msg = receiveQueue->extractBytesUpTo(endSeqNo);
    if (msg == nullptr)
        return nullptr;
    releaseRcvBuffer(msg->getByteLength());
    if (rxTimestampingEnabled)
        msg->addTag<TcpRxTimestampInd>(); // SO_TIMESTAMPING: the time of the delivery
    msg->setKind(TCP_I_DATA);
    msg->addTag<SocketInd>()->setSocketId(socketId);
    return msg;
}

void TcpConnection::processUrgentPointer(uint32_t urgentPointer)
{
    // urgentOutOfBand, as Linux tcp_check_urg(): RCV.UP from a URG segment, but no pointer
    // whose urgent data the application received, none whose last urgent byte is before
    // RCV.NXT ("Do not replay urg ptr"), and none that does not advance
    uint32_t consumed = receiveQueue->getFirstSeqNo();
    if (seqGE(consumed, urgentPointer) || seqLE(urgentPointer, state->rcv_nxt))
        return;
    if (state->rcvUrgentPending && seqLE(urgentPointer, state->rcv_up))
        return;
    state->rcv_up = urgentPointer;
    state->rcvUrgentPending = true;
    EV_INFO << "Urgent data: the urgent pointer is " << state->rcv_up << "\n";
}

void TcpConnection::initConnection(TcpOpenCommand *openCmd)
{
    // create send queue
    sendQueue = tcpMain->createSendQueue();
    sendQueue->setConnection(this);

    // create receive queue
    receiveQueue = tcpMain->createReceiveQueue();
    receiveQueue->setConnection(this);

    // create SACK retransmit queue
    rexmitQueue = new TcpSackRexmitQueue();
    rexmitQueue->setConnection(this);

    socketAdvertisedWindow = openCmd->getAdvertisedWindow();
    socketReceiveBufferSize = openCmd->getReceiveBufferSize();
    socketSynRetries = openCmd->getSynRetries();

    // create algorithm
    const char *tcpAlgorithmClass = openCmd->getTcpAlgorithmClass();

    if (opp_isempty(tcpAlgorithmClass))
        tcpAlgorithmClass = tcpMain->par("tcpAlgorithmClass");

    tcpAlgorithm = createTcpAlgorithm(tcpAlgorithmClass);

    // create state block
    state = tcpAlgorithm->getStateVariables();
    configureStateVariables();
    tcpAlgorithm->initializeAlgorithm();
    if (keepAliveSockopt >= 0)
        tcpAlgorithm->setKeepAlive(keepAliveSockopt != 0);
}

TcpAlgorithm *TcpConnection::createTcpAlgorithm(const char *tcpAlgorithmClass)
{
    // the NED type has the name of the C++ class: a qualified NED name, or a name in the
    // package of the flavours (D-122)
    std::string nedTypeName = strchr(tcpAlgorithmClass, '.') != nullptr ? tcpAlgorithmClass
            : std::string("inet.transportlayer.tcp.flavours.") + tcpAlgorithmClass;
    cModuleType *moduleType = cModuleType::find(nedTypeName.c_str());
    if (moduleType == nullptr)
        throw cRuntimeError("The TCP algorithm \"%s\" has no NED type %s: add a simple module type "
                "that extends inet.transportlayer.tcp.flavours.TcpAlgorithm", tcpAlgorithmClass, nedTypeName.c_str());
    auto algorithm = check_and_cast<TcpAlgorithm *>(moduleType->createScheduleInit("algorithm", this));
    algorithm->setConnection(this);
    return algorithm;
}

void TcpConnection::configureStateVariables()
{
    // the window of the socket, from its open command, takes the place of the module
    // parameters, with an automatic window scale (D-4)
    uint32_t advertisedWindow = socketAdvertisedWindow >= 0 ? (uint32_t)socketAdvertisedWindow : (uint32_t)tcpMain->par("advertisedWindow").intValue();
    state->ws_support = tcpMain->par("windowScalingSupport");
    int windowScalingFactor = socketAdvertisedWindow >= 0 ? -1 : (int)tcpMain->par("windowScalingFactor");
    if (windowScalingFactor < -1 || windowScalingFactor > 14)
        throw cRuntimeError("Invalid parameter value windowScalingFactor=%d -- valid values are 0..14, and -1 for automatic selection based on advertisedWindow", windowScalingFactor);
    if (state->ws_support) {
        uint32_t maxAdvertisedWindow = TCP_MAX_WIN << (windowScalingFactor==-1 ? 14 : windowScalingFactor);
        if (advertisedWindow > maxAdvertisedWindow)
            throw cRuntimeError("Invalid parameter value: advertisedWindow=%" PRIu32 " exceeds representable maximum %" PRIu32 " with windowScalingFactor=%d", advertisedWindow, maxAdvertisedWindow, windowScalingFactor);
    }
    else if (advertisedWindow > TCP_MAX_WIN) {
        throw cRuntimeError("Invalid parameter value: advertisedWindow=%" PRIu32 " exceeds representable maximum %lu, try turning on window scaling (windowScalingSupport=true)", advertisedWindow, TCP_MAX_WIN);
    }
    state->ws_manual_scale = windowScalingFactor;

    state->rcv_wnd = advertisedWindow;
    state->rcv_adv = advertisedWindow;

    if (state->ws_support && advertisedWindow > TCP_MAX_WIN) {
        state->rcv_wnd = TCP_MAX_WIN; // we cannot to guarantee that the other end is also supporting the Window Scale (header option) (RFC 1322)
        state->rcv_adv = TCP_MAX_WIN; // therefore TCP_MAX_WIN is used as initial value for rcv_wnd and rcv_adv
    }

    state->maxRcvBuffer = advertisedWindow;
    double receiveBufferSize = socketReceiveBufferSize >= 0 ? (double)socketReceiveBufferSize : tcpMain->par("receiveBufferSize").doubleValue();
    state->rcvBufferSize = receiveBufferSize > 0 ? (uint32_t)receiveBufferSize : 0;
    // SO_RCVBUF locks the buffer (Linux SOCK_RCVBUF_LOCK), before and after the open; a
    // listening socket passes its buffer to the connections that it forks
    state->rcvbufLocked = socketReceiveBufferSize >= 0;
    if (rcvBufSockopt >= 0) {
        state->rcvBufferSize = (uint32_t)rcvBufSockopt;
        state->rcvbufLocked = true;
    }
    state->acceptBeyondBufferAtEmptyQueue = tcpMain->par("acceptBeyondBufferAtEmptyQueue");
    // window auto-tuning (Linux tcp_grow_window()): the offer starts at the advertised
    // window and grows toward half the receive buffer (tcp_win_from_space() at the
    // default scaling ratio)
    if (tcpMain->par("windowAutoTuning").boolValue() && state->rcvBufferSize > 0) {
        state->rcv_ssthresh = advertisedWindow;
        state->window_clamp = std::max(advertisedWindow, state->rcvBufferSize / 2);
    }
    // windowShrinkAllowed (Linux tcp_shrink_window=1): the window follows the receive
    // buffer, and the first offer is half the buffer (tcp_select_initial_window() at the
    // default scaling ratio)
    state->windowShrinkAllowed = tcpMain->par("windowShrinkAllowed");
    if (state->windowShrinkAllowed && state->rcvBufferSize > 0) {
        state->maxRcvBuffer = state->rcvBufferSize;
        state->rcv_wnd = state->rcv_adv = std::min<uint32_t>(state->rcvBufferSize / 2, TCP_MAX_WIN);
        if (state->rcv_ssthresh > 0) {
            state->rcv_ssthresh = std::max(state->rcv_ssthresh, state->rcvBufferSize);
            state->window_clamp = std::max(state->window_clamp, state->rcvBufferSize);
        }
    }
    // a receive buffer smaller than the advertised window limits the first offer too
    if (state->rcvBufferSize > 0 && state->rcv_wnd > state->rcvBufferSize)
        state->rcv_wnd = state->rcv_adv = state->rcvBufferSize;
    state->rcvBufferLowRttUs = (uint32_t)tcpMain->par("receiveBufferLowRtt").doubleValueInUnit("us");
    state->rcvBufferMax = (uint32_t)std::min<double>(tcpMain->par("receiveBufferMax").doubleValueInUnit("B"), UINT32_MAX);
    state->delayed_acks_enabled = tcpMain->par("delayedAcksEnabled"); // delayed ACK algorithm (RFC 1122) enabled/disabled
    state->delayedAckFrameCount = tcpMain->par("delayedAckFrameCount");
    state->adaptiveDelayedAcks = tcpMain->par("adaptiveDelayedAcks");
    state->synRetries = socketSynRetries >= 0 ? socketSynRetries : (int)tcpMain->par("synRetries");
    state->synLinearTimeouts = tcpMain->par("synLinearTimeouts");
    state->dropSynData = tcpMain->par("dropSynData");
    state->linuxZeroWindowProbe = !strcmp(tcpMain->par("zeroWindowProbe").stringValue(), "linux");
    state->nagle_enabled = tcpMain->par("nagleEnabled"); // Nagle's algorithm (RFC 896) enabled/disabled
    if (corkSockopt >= 0)
        state->tcp_cork = corkSockopt != 0; // TCP_CORK of this connection
    if (noDelaySockopt >= 0)
        state->nagle_enabled = noDelaySockopt == 0; // TCP_NODELAY of this connection
    state->minshallNagle = tcpMain->par("minshallNagle");
    state->pushOnWriteBoundary = tcpMain->par("pushSegmentsOnWriteBoundary");
    state->pushLastBuffered = tcpMain->par("pushLastBufferedSegment");
    state->tsRecentAfterAckCheck = tcpMain->par("tsRecentAfterAckCheck");
    state->rfc5961AckCheck = tcpMain->par("rfc5961AckCheck");
    state->dropBeyondWindowEdge = tcpMain->par("dropBeyondWindowEdge");
    state->urgentOutOfBand = tcpMain->par("urgentOutOfBand");
    state->windowUpdateAfterRead = tcpMain->par("windowUpdateAfterRead");
    state->abortRstWithAck = tcpMain->par("abortRstWithAck");
    state->softIcmpErrorsAbortSetup = tcpMain->par("softIcmpErrorsAbortSetup");
    state->icmpSequenceCheck = tcpMain->par("icmpSequenceCheck");
    state->limited_transmit_enabled = tcpMain->par("limitedTransmitEnabled"); // Limited Transmit algorithm (RFC 3042) enabled/disabled
    state->increased_IW_enabled = tcpMain->par("increasedIWEnabled"); // Increased Initial Window (RFC 3390) enabled/disabled
    // initialWindow; the deprecated increasedIWEnabled selects RFC 3390, and only the
    // other increased window, RFC 6928, conflicts with it
    const char *initialWindow = tcpMain->par("initialWindow");
    if (!strcmp(initialWindow, "rfc6928"))
        state->init_cwnd_mode = 2;
    else if (!strcmp(initialWindow, "rfc3390"))
        state->init_cwnd_mode = 1;
    else
        state->init_cwnd_mode = 0;
    if (state->increased_IW_enabled) {
        if (state->init_cwnd_mode == 2)
            throw cRuntimeError("Tcp: set either the deprecated increasedIWEnabled or initialWindow = \"rfc6928\", not both");
        state->init_cwnd_mode = 1;
    }
    int initialCwnd = tcpMain->par("initialCwnd");
    if (initialCwnd < 0)
        throw cRuntimeError("initialCwnd must be 0 or more, but it is %d", initialCwnd);
    state->initialCwndSegments = initialCwnd;
    // Maximum Segment Size (RFC 9293 section 3.7.1); mss = -1 waits for the address of the peer, and
    // until then the connection has the 536 bytes of RFC 9293 (MUST-15)
    int mss = tcpMain->par("mss");
    if (mss < -1)
        throw cRuntimeError("mss must be -1 or more, but it is %d", mss);
    mssFromAddressFamily = mss == -1;
    state->snd_mss = mssFromAddressFamily ? 536 : mss;
    if (maxSegSockopt > 0 && (uint32_t)maxSegSockopt < state->snd_mss)
        state->snd_mss = maxSegSockopt; // TCP_MAXSEG of this connection, as Linux user_mss
    state->advertisedMss = state->snd_mss; // our own receive limit; stays when snd_mss falls to the MSS of the peer
    state->ts_support = tcpMain->par("timestampSupport"); // if set, this means that current host supports TS (RFC 7323 section 3)
    const char *tcpEcnMode = tcpMain->par("tcpEcnMode");
    if (tcpMain->par("ecnWillingness").boolValue()) {
        // the deprecated parameter replaces a mode that requests no ECN: "off" and the
        // default, "passive"
        if (strcmp(tcpEcnMode, "off") != 0 && strcmp(tcpEcnMode, "passive") != 0)
            throw cRuntimeError("Set the deprecated ecnWillingness or tcpEcnMode, not both");
        state->ecnMode = TCP_ECN_MODE_RFC3168;
    }
    else if (!strcmp(tcpEcnMode, "passive"))
        state->ecnMode = TCP_ECN_MODE_PASSIVE;
    else if (!strcmp(tcpEcnMode, "rfc3168"))
        state->ecnMode = TCP_ECN_MODE_RFC3168;
    else if (!strcmp(tcpEcnMode, "accecn"))
        state->ecnMode = TCP_ECN_MODE_ACCECN;
    else if (!strcmp(tcpEcnMode, "accecn-passive"))
        state->ecnMode = TCP_ECN_MODE_ACCECN_PASSIVE;
    else
        state->ecnMode = TCP_ECN_MODE_OFF;
    state->ecnWillingness = state->ecnMode != TCP_ECN_MODE_OFF;
    state->accEcnOptionEnabled = tcpMain->par("accEcnOptionEnabled");
    state->dupthresh = tcpMain->par("dupthresh");
    state->sack_support = tcpMain->par("sackSupport"); // if set, this means that current host supports SACK (RFC 2018, 2883, 3517)
    // SACK-based (RFC 6675) loss recovery is provided by flavours whose createRecovery()
    // can return an Rfc6675Recovery (TcpReno, TcpNewReno). Other flavours (TcpTahoe,
    // TcpVegas, TcpWestwood, DumbTcp, ...) have no SACK recovery path. Rather than error,
    // treat sackSupport as a willingness (as Linux does; SACK is orthogonal to the congestion
    // control) and simply do not use SACK for a flavour that cannot recover with it.
    if (state->sack_support && !tcpAlgorithm->supportsSackRecovery()) {
        EV_WARN << "sackSupport=true but tcpAlgorithmClass=\"" << tcpAlgorithm->getClassName()
                << "\" has no SACK-based loss recovery; disabling SACK for this connection\n";
        state->sack_support = false;
    }
    state->seedRttFromHandshake = tcpMain->par("seedRttFromHandshake");
    state->fastopenClientEnabled = tcpMain->par("fastopenClientEnabled");
    state->fastopenServerEnabled = tcpMain->par("fastopenServerEnabled");
    state->fastopenLenientCookieValidation = tcpMain->par("fastopenLenientCookieValidation");
    state->fastopenExpOptionEnabled = tcpMain->par("fastopenExpOptionEnabled");
    state->fastopenAcceptWithoutCookie = tcpMain->par("fastopenAcceptWithoutCookie");
    state->fastopenClientNoCookieRequired = tcpMain->par("fastopenClientNoCookieRequired");
    state->fastopenCookieBytes = tcpMain->par("fastopenCookieBytes");
    if (state->fastopenCookieBytes < 4 || state->fastopenCookieBytes > 16 || state->fastopenCookieBytes % 2 != 0)
        throw cRuntimeError("fastopenCookieBytes must be an even number from 4 to 16 (RFC 7413 section 4.1.1), but it is %d", state->fastopenCookieBytes);
    state->prrEnabled = tcpMain->par("prrEnabled");
    state->lossUndoEnabled = tcpMain->par("lossUndoEnabled");
    state->frtoEnabled = tcpMain->par("frtoEnabled");
    state->tlpEnabled = tcpMain->par("tlpEnabled");
    state->adaptiveReorderingEnabled = tcpMain->par("adaptiveReorderingEnabled");
    state->dsack_enabled = tcpMain->par("dsackEnabled");
    state->maxReordering = tcpMain->par("maxReordering");
    state->reordering = state->dupthresh; // dynamic DupThresh starts at the static value
    state->lossDetectionMode = !strcmp(tcpMain->par("lossDetectionMode"), "rack") ? 1 : 0;
    if (state->lossDetectionMode == 1 && !state->sack_support) {
        // RACK needs the SACK scoreboard. Rather than make the connection
        // unusable, fall back to classic DupThresh -- the same "willingness"
        // treatment sackSupport itself gets just above, so that turning RACK on
        // by default cannot break a flavour or peer that ends up without SACK.
        EV_WARN << "lossDetectionMode=\"rack\" requires SACK, which is not enabled for this "
                   "connection; falling back to DupThresh loss detection\n";
        state->lossDetectionMode = 0;
    }
    state->pmtudEnabled = tcpMain->par("pmtudEnabled"); // Path MTU Discovery (RFC 1191, RFC 1981)
    int mtuProbing = tcpMain->par("mtuProbing");
    if (mtuProbing < 0 || mtuProbing > 2)
        throw cRuntimeError("mtuProbing must be 0, 1 or 2, but it is %d", mtuProbing);
    state->mtupEnabled = mtuProbing == 2;
    state->pmtudTimeout = tcpMain->par("pmtudTimeout"); // time after which original MSS is restored
    state->pmtudLastMssReduction = -1; // never reduced yet

    // TCP_INFO time counters: idle and not limited until the first SEND or
    // sendData() call says otherwise (enqueueSendCommandData(), sendData()).
    state->busyStartTime = -1;
    state->rwndLimitedStartTime = -1;

    // TCP_NOTSENT_LOWAT: a TcpSetNotsentLowatCommand before the open takes the place of
    // the parameter
    int notsentLowat = notsentLowatSockopt != INT_MIN ? notsentLowatSockopt : (int)tcpMain->par("notsentLowat").intValue();
    state->notsentLowat = notsentLowat < 0 ? (uint32_t)-1 : (uint32_t)notsentLowat;
    state->sndbufLimitedStartTime = -1;

    WATCH_EXPR("snd_nxt", state->snd_nxt);
    WATCH_EXPR("rcv_nxt", state->rcv_nxt);
    WATCH_EXPR("snd_una", state->snd_una);

}

void TcpConnection::selectInitialSeqNum()
{
    // set the initial send sequence number: initialSendSequenceNumber, else the clock of
    // RFC 9293 section 3.4.1 (one tick in 4 microseconds)
    int64_t iss = tcpMain->par("initialSendSequenceNumber");
    if (iss < -1 || iss > 0xffffffffLL)
        throw cRuntimeError("initialSendSequenceNumber must be -1 or 0 to 4294967295, but it is %" PRId64, iss);
    state->iss = iss != -1 ? (uint32_t)iss : (unsigned long)fmod(SIMTIME_DBL(simTime()) * 250000.0, 1.0 + (double)(unsigned)0xffffffffUL) & 0xffffffffUL;

    state->snd_una = state->snd_nxt = state->snd_max = state->iss;
    state->pushed_seq = state->iss + 1; // Linux tcp_connect_init(): no PSH mark yet

    sendQueue->init(state->iss + 1); // + 1 is for SYN
    rexmitQueue->init(state->iss + 1); // + 1 is for SYN
}

bool TcpConnection::isSegmentAcceptable(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader) const
{
    // check that segment entirely falls in receive window
    // RFC 9293 section 3.10.7.4 (First, check sequence number):
    // "There are four cases for the acceptability test for an incoming segment:
    //    Segment Receive  Test
    //    Length  Window
    //    ------- -------  -------------------------------------------
    //       0       0     SEG.SEQ = RCV.NXT
    //       0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
    //      >0       0     not acceptable
    //      >0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
    //                  or RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND"
    uint32_t len = tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>();
    uint32_t seqNo = tcpHeader->getSequenceNo();
    uint32_t ackNo = tcpHeader->getAckNo();
    uint32_t rcvWndEnd = state->rcv_nxt + state->rcv_wnd;
    bool ret;

    if (len == 0) {
        // After the window shrank, a peer may still send from the highest edge ever
        // offered, as Linux tcp_sequence() accepts. Without a shrink that edge is not
        // beyond the window, and the test is the one of RFC 9293.
        uint32_t promiseEnd = seqGreater(state->rcv_mwnd_seq, rcvWndEnd) ? state->rcv_mwnd_seq : rcvWndEnd;
        if (state->rcv_wnd == 0 && promiseEnd == rcvWndEnd)
            ret = (seqNo == state->rcv_nxt);
        else
            ret = seqLE(state->rcv_nxt, seqNo) && seqLE(seqNo, promiseEnd); // Accept an ACK on end of window
    }
    else { // len > 0
        if (state->rcv_wnd == 0)
            ret = false;
        else { // rcv_wnd > 0
            // RFC 9293: SEG.LEN counts the SYN and the FIN, so a segment of old data with a
            // FIN at RCV.NXT, as a peer retransmits its last data together with its FIN,
            // is acceptable, and its FIN is processed
            uint32_t endSeq = seqNo + len + tcpHeader->getSynFinLen();
            ret = (seqLE(state->rcv_nxt, seqNo) && seqLess(seqNo, rcvWndEnd))
                || (seqLess(state->rcv_nxt, endSeq) && seqLE(endSeq, rcvWndEnd)); // Accept an ACK on end of window
            // dropBeyondWindowEdge, as Linux tcp_sequence(): a segment whose data ends
            // beyond the highest window edge offered (rcv_mwnd_seq) is dropped whole, unless it
            // starts within it and the in-order receive queue is empty
            if (ret && state->dropBeyondWindowEdge && seqGreater(seqNo + len, state->rcv_mwnd_seq)
                && (seqGreater(seqNo, state->rcv_mwnd_seq) || receiveQueue->getAcknowledgedDataLength() != 0))
            {
                EV_WARN << "The segment ends beyond the highest window edge offered (" << state->rcv_mwnd_seq << "): dropping it\n";
                ret = false;
            }
        }
    }

    // RFC 9293 section 3.4:
    // "A new acknowledgment (called an "acceptable ack") is one for which
    // the inequality below holds:
    //    SND.UNA < SEG.ACK =< SND.NXT"
    if (!ret && len == 0) {
        if (!state->afterRto)
            ret = (seqLess(state->snd_una, ackNo) && seqLE(ackNo, state->snd_nxt));
        else
            ret = (seqLess(state->snd_una, ackNo) && seqLE(ackNo, state->snd_max)); // after RTO snd_nxt is reduced therefore we need to use snd_max instead of snd_nxt here
    }

    if (!ret)
        EV_WARN << "Not Acceptable segment. seqNo=" << seqNo << " ackNo=" << ackNo << " len=" << len << " rcv_nxt="
                << state->rcv_nxt << " rcv_wnd=" << state->rcv_wnd << " afterRto=" << state->afterRto << "\n";

    return ret;
}

void TcpConnection::sendSyn()
{
    if (remoteAddr.isUnspecified() || remotePort == -1)
        throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: foreign socket unspecified");

    if (localPort == -1)
        throw cRuntimeError(tcpMain, "Error processing command OPEN_ACTIVE: local port unspecified");

    // create segment
    const auto& tcpHeader = makeShared<TcpHeader>();
    tcpHeader->setSequenceNo(state->iss);
    tcpHeader->setSynBit(true);
    updateRcvWnd();
    tcpHeader->setWindow(state->rcv_wnd);

    // TCP Fast Open (RFC 7413 section 4.2.2): the data that process_SEND() gave
    // the SYN takes the sequence numbers after the SYN
    uint32_t synDataLen = state->fastopenSynDataLen;
    state->snd_max = state->snd_nxt = state->iss + 1 + synDataLen;
    emit(sndMaxSignal, state->snd_max);

    // The rexmit queue records the data of the SYN once, at the first SYN. A
    // retransmitted SYN leaves the data out (RFC 7413 section 4.2.2), and the
    // data stays in flight until the SYN-ACK shows what the server acknowledged.
    if (synDataLen > 0 && rexmitQueue->getBufferEndSeq() == state->iss + 1) {
        rexmitQueue->enqueueSentData(state->iss + 1, state->iss + 1 + synDataLen);
        // pushSegmentsOnWriteBoundary: Linux tcp_send_syn_data() gives the skb of the
        // SYN data PSH, so its retransmission after the handshake has PSH, also in a
        // longer write
        if (state->pushOnWriteBoundary)
            insertSeqNum(pushSeqNums, state->iss + 1 + synDataLen);
    }

    // ECN: the "accecn" mode requests Accurate ECN with (AE,CWR,ECE) = (1,1,1) (RFC 9768
    // section 3.1.1), and the "rfc3168" mode requests ECN (RFC 3168)
    if (state->ecnMode == TCP_ECN_MODE_ACCECN) {
        tcpHeader->setAeBit(true);
        tcpHeader->setCwrBit(true);
        tcpHeader->setEceBit(true);
        state->aeSynSent = true;
        state->ecnSynSent = false;
        EV << "AccECN-setup SYN packet sent\n";
    }
    else if (state->ecnMode == TCP_ECN_MODE_RFC3168) {
        tcpHeader->setEceBit(true);
        tcpHeader->setCwrBit(true);
        state->ecnSynSent = true;
        EV << "ECN-setup SYN packet sent\n";
    }
    else {
        // rfc 3168 page 16:
        // A host that is not willing to use ECN on a TCP connection SHOULD
        // clear both the ECE and CWR flags in all non-ECN-setup SYN and/or
        // SYN-ACK packets that it sends to indicate this unwillingness.
        tcpHeader->setEceBit(false);
        tcpHeader->setCwrBit(false);
        state->ecnSynSent = false;
//        EV << "non-ECN-setup SYN packet sent\n";
    }

    // AccECN fallback. RFC 9768 section 3.1.4.1: the client "SHOULD attempt to negotiate
    // the use of AccECN at least one more time by continuing to set all three TCP-ECN
    // flags (AE,CWR,ECE) = (1,1,1) on the first retransmitted SYN", and it "SHOULD send
    // subsequent retransmissions of the SYN with the three TCP-ECN flags cleared". A
    // SYN-ACK that answers an earlier SYN still sets the mode, so the request stays
    // recorded. An ECN-setup SYN keeps its flags: RFC 3168 section 6.1.1.1 lets it clear
    // them ("MAY"), but a SYN lost to congestion would then cost the connection its ECN.
    if (state->syn_rexmit_count >= 2 && state->ecnMode == TCP_ECN_MODE_ACCECN) {
        tcpHeader->setAeBit(false);
        tcpHeader->setCwrBit(false);
        tcpHeader->setEceBit(false);
        EV << "SYN retransmission " << state->syn_rexmit_count << ": no ECN flags (AccECN fallback)\n";
    }

    // write header options
    writeHeaderOptions(tcpHeader);
    bool attachSynData = synDataLen > 0 && state->syn_rexmit_count == 0;
    Packet *fp = attachSynData ? sendQueue->createSegmentWithBytes(state->iss + 1, synDataLen) : new Packet("SYN");

    state->handshakeSentTime = simTime(); // for the handshake RTT seed on ESTABLISHED
    if (state->syn_rexmit_count == 0)
        state->firstSynSentTime = simTime(); // for the TSecr check of the SYN-ACK

    // send it
    sendToIP(fp, tcpHeader);

    // the data of a Fast Open SYN goes out here, not by sendSegment(), so its zero-copy
    // SENDs complete here (snd_nxt is past the SYN data)
    if (attachSynData)
        notifyZerocopyCompletions();
}

void TcpConnection::sendSynAck()
{
    // create segment
    const auto& tcpHeader = makeShared<TcpHeader>();
    tcpHeader->setSequenceNo(state->iss);
    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setSynBit(true);
    tcpHeader->setAckBit(true);
    updateRcvWnd();
    tcpHeader->setWindow(state->rcv_wnd);

    // The SYN-ACK takes the ISS: snd_nxt and snd_max are at least ISS+1. A
    // retransmitted SYN-ACK of a TCP Fast Open server that sent data in SYN_RCVD
    // does not move them back over the data.
    if (seqLess(state->snd_nxt, state->iss + 1))
        state->snd_nxt = state->iss + 1;
    if (seqLess(state->snd_max, state->iss + 1)) {
        state->snd_max = state->iss + 1;
        emit(sndMaxSignal, state->snd_max);
    }

    // ECN. RFC 3168, section 6.1.1: "If a host has received an ECN-setup SYN packet,
    // then it MAY send an ECN-setup SYN-ACK packet.  Otherwise, it MUST NOT send an
    // ECN-setup SYN-ACK packet."
    if (state->accEcnNegotiated) {
        // RFC 9768 section 3.1.1: the flags of the SYN-ACK feed back the IP ECN
        // codepoint of the SYN (Table 2)
        uint8_t ace = accEcnReflectedAce(state->accEcnReflectCodepoint);
        tcpHeader->setAeBit((ace & 4) != 0);
        tcpHeader->setCwrBit((ace & 2) != 0);
        tcpHeader->setEceBit((ace & 1) != 0);
        EV << "AccECN-setup SYN-ACK packet sent (ACE=" << (int)ace << ")\n";
        // RFC 9768 section 3.2.3.2.2: if the first retransmission of the SYN-ACK "times
        // out, to expedite connection setup, the TCP Server SHOULD retransmit the SYN/ACK
        // with (AE,CWR,ECE) = (0,0,0) and no AccECN Option, but it remains in AccECN
        // feedback mode"; after it, the server "MUST NOT set ECT on any packet for the
        // rest of the connection" (section 3.1.5)
        if (state->syn_rexmit_count >= 2) {
            tcpHeader->setAeBit(false);
            tcpHeader->setCwrBit(false);
            tcpHeader->setEceBit(false);
            state->accEcnNoEct = true;
            EV << "SYN-ACK retransmission " << state->syn_rexmit_count << ": no AccECN flags (AccECN fallback), and no ECT\n";
        }
    }
    else if (state->ecnWillingness && state->endPointIsWillingECN) {
        tcpHeader->setEceBit(true);
        tcpHeader->setCwrBit(false);
        EV << "ECN-setup SYN-ACK packet sent\n";
    }
    else {
        tcpHeader->setEceBit(false);
        tcpHeader->setCwrBit(false);
        if (state->endPointIsWillingECN)
            EV << "non-ECN-setup SYN-ACK packet sent\n";
    }
    if (state->accEcnNegotiated)
        state->ect = !state->accEcnNoEct;
    else if (state->ecnWillingness && state->endPointIsWillingECN) {
        state->ect = true;
        EV << "both end-points are willing to use ECN... ECN is enabled\n";
    }
    else { // TODO not sure if we have to.
           // rfc-3168, page 16:
           // A host that is not willing to use ECN on a TCP connection SHOULD
           // clear both the ECE and CWR flags in all non-ECN-setup SYN and/or
           // SYN-ACK packets that it sends to indicate this unwillingness.
        state->ect = false;
        if (state->endPointIsWillingECN)
            EV << "ECN is disabled\n";
    }

    // write header options
    writeHeaderOptions(tcpHeader);

    Packet *fp = new Packet("SYN+ACK");

    state->handshakeSentTime = simTime(); // for the handshake RTT seed on ESTABLISHED

    // send it
    sendToIP(fp, tcpHeader);

    // notify
    tcpAlgorithm->ackSent();
}

void TcpConnection::sendRst(uint32_t seqNo)
{
    sendRst(seqNo, localAddr, remoteAddr, localPort, remotePort);
}

void TcpConnection::sendAbortRst()
{
    // RFC 7323 section 5.2: "If an <RST> is being generated because of a user abort, and
    // Snd.TS.OK is set, then a Timestamps option SHOULD be included in the <RST>"; the
    // RST has no ACK bit, so its TSecr is 0 (section 3.2)
    // RFC 9293 ABORT: "<SEQ=SND.NXT><CTL=RST>"; SND.NXT is the next new sequence number,
    // snd_max here, because snd_nxt goes back during a retransmission, and a RST below
    // RCV.NXT of the peer lies outside its window and is dropped
    // abortRstWithAck: the RST with ACK and RCV.NXT of Linux (tcp_send_active_reset())
    if (state->abortRstWithAck)
        sendRstAck(state->snd_max, state->rcv_nxt, localAddr, remoteAddr, localPort, remotePort, state->ts_enabled);
    else
        sendRst(state->snd_max, localAddr, remoteAddr, localPort, remotePort, state->ts_enabled);
}

void TcpConnection::sendRst(uint32_t seq, L3Address src, L3Address dest, int srcPort, int destPort, bool withTimestamp)
{
    const auto& tcpHeader = makeShared<TcpHeader>();

    tcpHeader->setSrcPort(srcPort);
    tcpHeader->setDestPort(destPort);

    tcpHeader->setRstBit(true);
    tcpHeader->setSequenceNo(seq);
    tcpHeader->setChecksumMode(tcpMain->checksumMode);
    tcpHeader->setChecksum(0);

    if (withTimestamp) {
        tcpHeader->appendHeaderOption(new TcpOptionNop());
        tcpHeader->appendHeaderOption(new TcpOptionNop());
        auto option = new TcpOptionTimestamp();
        option->setSenderTimestamp(convertSimtimeToTS(simTime()));
        option->setEchoedTimestamp(0);
        tcpHeader->appendHeaderOption(option);
        tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + tcpHeader->getHeaderOptionArrayLength());
        tcpHeader->setChunkLength(B(tcpHeader->getHeaderLength()));
    }

    Packet *fp = new Packet("RST");

    // RFC 8257 section 3.6: the RST of a DCTCP connection carries ECT (see sendToIP())
    if (state != nullptr && state->ect && tcpAlgorithm != nullptr && tcpAlgorithm->needsEcn())
        fp->addTag<EcnReq>()->setExplicitCongestionNotification(IP_ECN_ECT_0);

    // send it
    sendToIP(fp, tcpHeader, src, dest);
}

void TcpConnection::sendRstAck(uint32_t seq, uint32_t ack, L3Address src, L3Address dest, int srcPort, int destPort, bool withTimestamp)
{
    const auto& tcpHeader = makeShared<TcpHeader>();

    tcpHeader->setSrcPort(srcPort);
    tcpHeader->setDestPort(destPort);

    tcpHeader->setRstBit(true);
    tcpHeader->setAckBit(true);
    tcpHeader->setSequenceNo(seq);
    tcpHeader->setAckNo(ack);
    tcpHeader->setChecksumMode(tcpMain->checksumMode);
    tcpHeader->setChecksum(0);

    if (withTimestamp) {
        tcpHeader->appendHeaderOption(new TcpOptionNop());
        tcpHeader->appendHeaderOption(new TcpOptionNop());
        auto option = new TcpOptionTimestamp();
        option->setSenderTimestamp(convertSimtimeToTS(simTime()));
        option->setEchoedTimestamp(state->ts_recent); // the ACK bit is set (RFC 7323 section 3.2)
        tcpHeader->appendHeaderOption(option);
        tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + tcpHeader->getHeaderOptionArrayLength());
        tcpHeader->setChunkLength(B(tcpHeader->getHeaderLength()));
    }

    Packet *fp = new Packet("RST+ACK");

    // send it
    sendToIP(fp, tcpHeader, src, dest);

    // notify
    if (tcpAlgorithm)
        tcpAlgorithm->ackSent();
}

void TcpConnection::sendAck()
{
    const auto& tcpHeader = makeShared<TcpHeader>();

    tcpHeader->setAckBit(true);
    // SND.NXT of RFC 9293, "<SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>", is the next new sequence
    // number, which a retransmission does not move back; that is snd_max here, because
    // snd_nxt goes back to snd_una during a retransmission (also BSD and Linux)
    tcpHeader->setSequenceNo(state->snd_max);
    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setWindow(updateRcvWnd());

    // rfc-3168, pages 19-20:
    // When TCP receives a CE data packet at the destination end-system, the
    // TCP data receiver sets the ECN-Echo flag in the TCP header of the
    // subsequent ACK packet.
    // ...
    // After a TCP receiver sends an ACK packet with the ECN-Echo bit set,
    // that TCP receiver continues to set the ECN-Echo flag in all the ACK
    // packets it sends (whether they acknowledge CE data packets or non-CE
    // data packets) until it receives a CWR packet (a packet with the CWR
    // flag set).  After the receipt of the CWR packet, acknowledgments for
    // subsequent non-CE data packets do not have the ECN-Echo flag set.

    // (an AccECN connection uses the ECE bit for the ACE field, see sendToIP())
    TcpStateVariables *state = getStateForUpdate();
    if (state && state->ect && !state->accEcnNegotiated) {
        if (tcpAlgorithm->shouldMarkAck()) {
            tcpHeader->setEceBit(true);
            EV_INFO << "In ecnEcho state... send ACK with ECE bit set\n";
        }
    }

    // write header options
    writeHeaderOptions(tcpHeader);
    Packet *fp = new Packet("TcpAck");

    // send it (Not-ECT, as each segment without data, see sendToIP())
    sendToIP(fp, tcpHeader);

    // notify
    tcpAlgorithm->ackSent();
}

void TcpConnection::sendFin()
{
    const auto& tcpHeader = makeShared<TcpHeader>();

    // Note: ACK bit *must* be set for both FIN and FIN+ACK. What makes
    // the difference for FIN+ACK is that its ackNo acks the remote Tcp's FIN.
    tcpHeader->setFinBit(true);
    tcpHeader->setAckBit(true);
    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setSequenceNo(state->snd_nxt);
    tcpHeader->setWindow(updateRcvWnd());
    // the options of each segment, the TS option among them (RFC 7323 section 3.2)
    writeHeaderOptions(tcpHeader);
    Packet *fp = new Packet("FIN");

    // send it
    sendToIP(fp, tcpHeader);

    // notify
    tcpAlgorithm->ackSent();
}

uint32_t TcpConnection::sendSegment(uint32_t bytes)
{
    // PMTUD (RFC 1191): if the probe timeout has elapsed since the last MSS reduction,
    // restore the original negotiated MSS to probe for increased path MTU.
    if (state->pmtudEnabled && state->pmtudLastMssReduction >= SIMTIME_ZERO
        && simTime() - state->pmtudLastMssReduction >= state->pmtudTimeout)
    {
        EV_INFO << "PMTUD: probe timeout elapsed, restoring snd_mss from " << state->snd_mss
                << " to original " << state->pmtudOriginalMss << "\n";
        state->snd_mss = state->pmtudOriginalMss;
        state->snd_effmss = calculateEffectiveMss();
        state->pmtudLastMssReduction = -1;
    }

    // FIXME check it: where is the right place for the next code (sacked/rexmitted)
    if (state->sack_enabled && state->afterRto) {
        // check rexmitQ and try to forward snd_nxt before sending new data
        uint32_t forward = rexmitQueue->checkRexmitQueueForSackedOrRexmittedSegments(state->snd_nxt);

        if (forward > 0) {
            EV_INFO << "sendSegment(" << bytes << ") forwarded " << forward << " bytes of snd_nxt from " << state->snd_nxt;
            state->snd_nxt += forward;
            EV_INFO << " to " << state->snd_nxt << endl;
            EV_DETAIL << rexmitQueue->detailedInfo();
        }
    }

    // mtuProbing: a retransmission into the probe shows that the probe is lost; Linux
    // decides at the fast retransmit (tcp_fastretrans_alert())
    if (state->mtupProbeSize != 0 && seqLess(state->snd_nxt, state->snd_max)
        && seqGE(state->snd_nxt, state->mtupProbeSeqStart) && seqLess(state->snd_nxt, state->mtupProbeSeqEnd))
        mtuProbeFailed();

    uint32_t buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (bytes > buffered) // last segment?
        bytes = buffered;

    // After a retransmission timeout, the forward above can move snd_nxt to the end
    // of the send queue, when the receiver has SACKed or the sender has retransmitted
    // all data above it. A segment of zero bytes cannot be made. Send nothing and
    // return 0; the callers stop their send loops at a zero return. The go-back-N
    // after the timeout is then complete.
    if (bytes == 0) {
        if (state->afterRto && seqGE(state->snd_nxt, state->snd_max))
            state->afterRto = false;
        return 0;
    }

    // if header options will be added, this could reduce the number of data bytes allowed for this segment,
    // because following condition must to be respected:
    //     bytes + options_len <= snd_mss
    const auto& tmpTcpHeader = makeShared<TcpHeader>();
    tmpTcpHeader->setAckBit(true); // needed for TS option, otherwise TSecr will be set to 0
    writeHeaderOptions(tmpTcpHeader);
    uint options_len = (tmpTcpHeader->getHeaderLength() - TCP_MIN_HEADER_LENGTH).get<B>();

    ASSERT(options_len < state->snd_mss);

    // an RFC 4821 probe (mtuProbing) is larger than the MSS on purpose
    if (state->mtupProbeBytes == 0 && bytes + options_len > state->snd_mss)
        bytes = state->snd_mss - options_len;

    // MSG_EOR: a segment ends at the next record boundary, also in a retransmission.
    // The boundaries that are acknowledged are removed first.
    removeSeqNumsUpTo(eorSeqNums, state->snd_una);
    removeSeqNumsUpTo(pushSeqNums, state->snd_una);
    removeSeqNumsUpTo(forcedPushSeqNums, state->snd_una);
    auto nextBoundary = findSeqNumAfter(eorSeqNums, state->snd_nxt);
    if (nextBoundary != eorSeqNums.end() && bytes > *nextBoundary - state->snd_nxt)
        bytes = *nextBoundary - state->snd_nxt;

    // A retransmission never extends past the previously sent high-water mark
    // in the same segment: Linux retransmits skbs from the rtx queue (possibly
    // collapsed together, but only from already-SENT data); unsent data goes
    // out in its own segments behind it.
    if (seqLess(state->snd_nxt, state->snd_max) && bytes > state->snd_max - state->snd_nxt)
        bytes = state->snd_max - state->snd_nxt;

    // ... and it honors the ORIGINAL segment boundaries: Linux's rtx queue
    // holds whole skbs, and tcp_retrans_try_collapse merges only ENTIRE
    // adjacent sent skbs that fit cur_mss together -- it never splits the
    // next skb to top a retransmit up to the MSS. Cap at the largest recorded
    // transmission boundary inside the budget. A budget that ends at snd_max
    // ends with a whole segment, so it needs no cap.
    if (seqLess(state->snd_nxt + bytes, state->snd_max) && bytes > 0 && rexmitQueue != nullptr) {
        const auto& starts = rexmitQueue->xmitSegmentStarts;
        auto it = starts.upper_bound(state->snd_nxt + bytes);
        if (it != starts.begin()) {
            uint32_t b = *std::prev(it);
            if (seqGreater(b, state->snd_nxt) && seqLess(b, state->snd_nxt + bytes))
                bytes = b - state->snd_nxt;
        }
    }

    uint32_t sentBytes = bytes;

    // send one segment of 'bytes' bytes from snd_nxt, and advance snd_nxt
    Packet *tcpSegment = sendQueue->createSegmentWithBytes(state->snd_nxt, bytes);
    const auto& tcpHeader = makeShared<TcpHeader>();
    tcpHeader->setSequenceNo(state->snd_nxt);
    ASSERT(tcpHeader != nullptr);

    // Remember old_snd_next to store in SACK rexmit queue.
    uint32_t old_snd_nxt = state->snd_nxt;

    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setAckBit(true);
    tcpHeader->setWindow(updateRcvWnd());

    // ECN
    // (RFC 9768 section 3.1.5: an AccECN connection "MUST NOT set CWR to indicate
    // that it has received and responded to indications of congestion")
    if (state->ect && state->sndCwr && !sendingWindowProbe && !state->accEcnNegotiated) {
        tcpHeader->setCwrBit(true);
        EV_INFO << "set CWR bit\n";
        state->sndCwr = false;
    }

    // a partial that TCP_CORK or MSG_MORE held, see sendData()
    if (state->pushThisSegment)
        tcpHeader->setPshBit(true);
    // TODO set URG bit if needed
    ASSERT(bytes == tcpSegment->getByteLength());

    state->snd_nxt += bytes;

    // pushSegmentsOnWriteBoundary: PSH on the segment that ends a write or a mark of
    // forced_push(). Linux marks the skb at the time of the write, so a retransmission
    // keeps PSH. A held partial that goes now has the PSH rule of the cork above.
    if (state->pushOnWriteBoundary && !state->corkFlush && !state->corkedDataPending
            && (containsSeqNum(pushSeqNums, state->snd_nxt) || containsSeqNum(forcedPushSeqNums, state->snd_nxt)))
        tcpHeader->setPshBit(true);

    // pushLastBufferedSegment, RFC 9293 section 3.9.1.2: the SEND of INET has no PUSH
    // flag, so the TCP "MUST set the PSH bit in the last buffered segment (i.e., when
    // there is no more queued data to be sent)" (MUST-61); the rule of Linux above
    // replaces it
    if (state->pushLastBuffered && !state->pushOnWriteBoundary && state->snd_nxt == sendQueue->getBufferEndSeq())
        tcpHeader->setPshBit(true);

    // check if afterRto bit can be reset
    if (state->afterRto && seqGE(state->snd_nxt, state->snd_max))
        state->afterRto = false;

    if (state->send_fin && state->snd_nxt == state->snd_fin_seq) {
        EV_DETAIL << "Setting FIN on segment\n";
        tcpHeader->setFinBit(true);
        state->snd_nxt = state->snd_fin_seq + 1;
    }

    notifyZerocopyCompletions();

    updateSndbufLimitedChrono(); // this segment can have sent the last unsent data

    // copy the region of tcpHeader to rexmitQueue; without SACK, the queue
    // still records what is in flight
    rexmitQueue->enqueueSentData(old_snd_nxt, state->snd_nxt);

    // add header options and update header length (from tcpseg_temp)
    for (uint i = 0; i < tmpTcpHeader->getHeaderOptionArraySize(); i++)
        tcpHeader->appendHeaderOption(tmpTcpHeader->getHeaderOption(i)->dup());
    tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + tcpHeader->getHeaderOptionArrayLength());
    tcpHeader->setChunkLength(B(tcpHeader->getHeaderLength()));

    ASSERT(tcpHeader->getHeaderLength() == tmpTcpHeader->getHeaderLength());

    // send it
    sendToIP(tcpSegment, tcpHeader);

    // let application fill queue again, if there is space
    const uint32_t alreadyQueued = sendQueue->getBytesAvailable(sendQueue->getBufferStartSeq());
    const uint32_t abated = (state->sendQueueLimit > alreadyQueued) ? state->sendQueueLimit - alreadyQueued : 0;
    if ((state->sendQueueLimit > 0) && !state->queueUpdate && (abated >= state->snd_mss)) { // request more data if space >= 1 MSS
        // Tell upper layer readiness to accept more data
        sendIndicationToApp(TCP_I_SEND_MSG, abated);
        state->queueUpdate = true;
    }

    // TCP_NOTSENT_LOWAT: the unsent bytes are at the mark again
    if (state->notsentLowat != (uint32_t)-1 && !state->notsentLowatUpdate) {
        uint32_t notsentBytes = sendQueue->getBytesAvailable(state->snd_nxt);
        if (notsentBytes <= state->notsentLowat) {
            sendIndicationToApp(TCP_I_SEND_MSG, notsentBytes);
            state->notsentLowatUpdate = true;
        }
    }

    // minshallNagle: the end of the last small segment of new data, as Linux
    // tcp_minshall_update(); small is less than this segment could carry
    if (bytes > 0 && bytes + options_len < state->snd_mss && seqGreater(state->snd_nxt, state->snd_max))
        state->snd_sml = state->snd_nxt;

    // remember highest seq sent (snd_nxt may be set back on retransmission,
    // but we'll need snd_max to check validity of ACKs -- they must ack
    // something we really sent)
    if (seqGreater(state->snd_nxt, state->snd_max)) {
        state->snd_max = state->snd_nxt;
        emit(sndMaxSignal, state->snd_max);
    }

    // The peak segments in flight in the current window of data, as Linux
    // tcp_cwnd_validate() keeps it: a new window starts when snd_una has passed
    // the snd_nxt of the last start. A partial segment counts as a whole one,
    // as Linux counts packets.
    if (state->snd_effmss > 0) {
        uint32_t packetsOut = (state->snd_max - state->snd_una + state->snd_effmss - 1) / state->snd_effmss;
        if (!seqLess(state->snd_una, state->cwndUsageSeq) || packetsOut > state->maxPacketsOut) {
            state->maxPacketsOut = packetsOut;
            state->cwndUsageSeq = state->snd_nxt;
        }
    }

    return sentBytes;
}

void TcpConnection::enqueueSendCommandData(Packet *packet)
{
    // A SEND of zero bytes has nothing to queue, and TcpSendQueue::enqueueAppData()
    // cannot take it: its peekDataAt(B(0), 0) throws "Returning an empty chunk is not
    // allowed". It is not a malformed command: send(fd, x, 0) is a legal no-op, and
    // sendto(fd, x, 0, MSG_FASTOPEN) asks for nothing but a Fast Open cookie.
    if (packet->getByteLength() == 0) {
        EV_DETAIL << "SEND of zero bytes: nothing to queue\n";
        delete packet;
        return;
    }

    // TCP_INFO time counters (busy_time): read-only bookkeeping -- if the connection
    // was fully idle (nothing outstanding, nothing queued) before this SEND, it
    // becomes busy now. See processAckInEstabEtc() for the matching "back to idle" exit.
    if (state->busyStartTime < SIMTIME_ZERO && state->snd_una == state->snd_max
        && sendQueue->getBytesAvailable(state->snd_nxt) == 0)
    {
        state->busyStartTime = simTime();
    }

    bool zerocopy = packet->findTag<TcpSendZerocopyReq>() != nullptr;
    bool eor = packet->findTag<TcpSendEorReq>() != nullptr;
    // MSG_MORE holds the partial segment at the end of this SEND (sendData() takes the
    // flag). A SEND without MSG_MORE while a partial is held gives that partial PSH
    // when it goes, as Linux tcp_mark_push() on the last write.
    bool msgMore = packet->findTag<TcpSendMoreReq>() != nullptr;
    state->msgMoreThisSend = msgMore;
    if (!msgMore && (state->tcp_cork || state->corkedDataPending))
        state->pushHeldPartial = true;
    uint32_t writeStartSeq = sendQueue->getBufferEndSeq(); // Linux tp->write_seq before this write
    sendQueue->enqueueAppData(packet);

    // MSG_EOR: the end of this SEND is a record boundary
    if (eor) {
        eorSeqNums.push_back(sendQueue->getBufferEndSeq());
        EV_DETAIL << "MSG_EOR: record boundary at sequence number " << eorSeqNums.back() << "\n";
    }

    // pushSegmentsOnWriteBoundary, as Linux tcp_sendmsg(): forced_push() marks the skb that
    // a write fills more than max_window/2 after the last mark. The skbs of a write are
    // size_goal long (tcp_xmit_size_goal(), half the largest window in whole MSS), and
    // the PSH of a mark is on the second MSS of the skb.
    if (state->pushOnWriteBoundary && !msgMore && state->max_window > 0) {
        uint32_t halfWnd = state->max_window >> 1;
        const uint32_t mssCache = 1460; // Linux tp->mss_cache (IPv4 PMTU 1500 - 40)
        uint32_t alignQ = (halfWnd + mssCache - 1) & ~(mssCache - 1); // the ALIGN() of Linux, which is for a power of 2
        uint32_t sizeGoal = std::max((alignQ / state->snd_mss) * state->snd_mss, state->snd_mss);
        uint32_t writeEndSeq = sendQueue->getBufferEndSeq();
        for (uint32_t f = writeStartSeq + sizeGoal; seqLE(f, writeEndSeq); f += sizeGoal) {
            if (seqGreater(f, state->pushed_seq + halfWnd)) {
                insertSeqNum(forcedPushSeqNums, f - sizeGoal + 2 * state->snd_mss);
                state->pushed_seq = f;
                EV_DETAIL << "forced_push: PSH at sequence number " << f - sizeGoal + 2 * state->snd_mss << "\n";
            }
        }
    }
    // pushSegmentsOnWriteBoundary: PSH on the segment that ends this write (Linux
    // tcp_push() and tcp_mark_push() on the last skb of a write without MSG_MORE)
    if (state->pushOnWriteBoundary && !msgMore) {
        insertSeqNum(pushSeqNums, sendQueue->getBufferEndSeq());
        state->pushed_seq = sendQueue->getBufferEndSeq();
    }

    // MSG_ZEROCOPY: the completion of this SEND waits until its last byte goes out
    if (zerocopy) {
        uint32_t endSeq = sendQueue->getBufferEndSeq();
        zerocopySeqNums[endSeq] = nextZerocopyId;
        EV_DETAIL << "MSG_ZEROCOPY: completion " << nextZerocopyId << " at sequence number " << endSeq << "\n";
        nextZerocopyId++;
    }

    updateSndbufLimitedChrono(); // new unsent data ends a limited interval
}

int TcpConnection::deriveLinuxCaState() const
{
    if (state->afterRto)
        return 4; // TCP_CA_Loss
    if (state->lossRecovery)
        return 3; // TCP_CA_Recovery
    if (state->sndCwr)
        return 2; // TCP_CA_CWR
    // TCP_CA_Disorder: SACK/dup information has arrived (segments sit above
    // snd_una) but not enough to enter recovery yet -- Linux tcp_fastretrans_alert
    // holds ca_state at Disorder while sacked_out > 0 without a confirmed loss.
    // sackedBytes is kept current on both the SACK and the cumulative-ACK path, so
    // this reverts to Open as soon as snd_una catches up to the SACKed data.
    if (state->sack_enabled && state->sackedBytes > 0)
        return 1; // TCP_CA_Disorder
    return 0; // TCP_CA_Open
}

uint32_t TcpConnection::getDataSndUna() const
{
    // only a TCP Fast Open server sends data while snd_una is at the ISS
    if (fsm.getState() == TCP_S_SYN_RCVD && state->snd_una == state->iss)
        return state->iss + 1;
    return state->snd_una;
}

bool TcpConnection::sendData(uint32_t congestionWindow)
{
    // MSG_MORE holds only for the SEND that brought it, not for a later send of an ACK
    bool msgMoreHold = state->msgMoreThisSend;
    state->msgMoreThisSend = false;

    // we'll start sending from snd_max, if not after RTO
    if (!state->afterRto)
        state->snd_nxt = state->snd_max;

    uint32_t old_highRxt = 0;

    if (state->sack_enabled)
        old_highRxt = rexmitQueue->getHighestRexmittedSeqNum();

    // check how many bytes we have
    uint32_t buffered = sendQueue->getBytesAvailable(state->snd_nxt);

    if (buffered == 0)
        return false;

    // effectiveWindow: number of bytes we're allowed to send now. The advertised
    // window limits the unacknowledged sequence space, and the congestion window
    // limits the bytes in flight that the algorithm reports.
    uint32_t unackedInWindow = state->snd_nxt - getDataSndUna();
    uint32_t bytesInFlight = tcpAlgorithm->getBytesInFlight();
    int64_t effectiveWin = std::min((int64_t)state->snd_wnd - unackedInWindow, (int64_t)congestionWindow - bytesInFlight);

    // TCP_INFO time counters (rwnd_limited): read-only bookkeeping, consulted only
    // by TcpStatusInfo -- never influences the send decision below. "rwnd-limited"
    // here means: there is more buffered data than can be sent right now, and the
    // peer's advertised window (not the congestion window) is the binding
    // constraint.
    bool rwndBinding = (state->snd_wnd < congestionWindow)
        && ((int64_t)buffered > std::max<int64_t>(effectiveWin, 0));
    if (rwndBinding) {
        if (state->rwndLimitedStartTime < SIMTIME_ZERO)
            state->rwndLimitedStartTime = simTime();
    }
    else if (state->rwndLimitedStartTime >= SIMTIME_ZERO) {
        state->rwndLimitedAccumulated += simTime() - state->rwndLimitedStartTime;
        state->rwndLimitedStartTime = -1;
    }

    if (effectiveWin <= 0) {
        EV_WARN << "Effective window is zero (advertised window " << state->snd_wnd
                << ", congestion window " << congestionWindow << "), cannot send.\n";
        return false;
    }

    // RFC 9293 section 3.8.6.2.1, the sender SWS avoidance (MUST-38): a segment of new
    // data smaller than the effective MSS goes out only if it carries all queued data
    // (rule (2)) or at least Fs = 1/2 of the largest window that the peer offered (rule
    // (3)); else it waits for a larger window, or for the override timeout (rule (4), the
    // persist timer, see TcpAlgorithmBase::sendData()). As BSD, the usable window is also
    // limited by the congestion window, and a retransmission is never held.
    auto swsHold = [&] (int64_t usable, uint32_t queued) {
        return !state->swsOverride && seqGE(state->snd_nxt, state->snd_max) && usable < state->snd_effmss
               && queued > usable && usable < state->max_window / 2;
    };
    if (swsHold(effectiveWin, buffered)) {
        EV_WARN << "Not sending to prevent Silly Window Syndrome.\n";
        return false;
    }

    uint32_t bytesToSend = std::min(buffered, (uint32_t)effectiveWin);

    // make a temporary tcp header for detecting tcp options length (copied from 'TcpConnection::sendSegment(uint32_t bytes)' )
    const auto& tmpTcpHeader = makeShared<TcpHeader>();
    tmpTcpHeader->setAckBit(true); // needed for TS option, otherwise TSecr will be set to 0
    writeHeaderOptions(tmpTcpHeader);
    uint options_len = (tmpTcpHeader->getHeaderLength() - TCP_MIN_HEADER_LENGTH).get<B>();
    ASSERT(options_len < state->snd_mss);
    uint32_t effectiveMss = state->snd_mss - options_len;

    uint32_t old_snd_nxt = state->snd_nxt;

    // start sending 'bytesToSend' bytes
    EV_INFO << "May send " << bytesToSend << " bytes (effectiveWindow " << effectiveWin << ", in buffer " << buffered << " bytes)\n";

    // mtuProbing, as Linux tcp_write_xmit(): the RFC 4821 probe goes first, one segment
    // larger than the MSS, from data that goes out anyway; when only the window edge or
    // the pipe has no room for it, the sender waits for the ACKs that make room
    int64_t probeBytes = state->mtupEnabled ? getMtuProbeBytes(buffered, congestionWindow, bytesInFlight) : -1;
    if (probeBytes == 0) {
        EV_INFO << "RFC 4821: waiting for room for an MTU probe\n";
        return false;
    }
    if (probeBytes > 0 && probeBytes <= bytesToSend) {
        state->mtupProbeSeqStart = state->snd_nxt;
        state->mtupProbeBytes = probeBytes;
        uint32_t sentBytes = sendSegment(probeBytes);
        state->mtupProbeBytes = 0;
        state->mtupProbeSeqEnd = state->mtupProbeSeqStart + sentBytes;
        state->mtupProbeSize = mssToMtu(sentBytes);
        EV_INFO << "RFC 4821: sent an MTU probe of " << sentBytes << " bytes (MTU " << state->mtupProbeSize << ")\n";
        bytesToSend -= sentBytes;
        effectiveWin -= sentBytes;
        buffered -= sentBytes;
    }

    // send whole segments. pushSegmentsOnWriteBoundary: PSH on the last segment of each
    // GSO burst of more than one segment of new data, as Linux tcp_transmit_skb()
    // (tcp_skb_pcount() > 1). A retransmission is its own skb in Linux, so it has no
    // place in a burst.
    uint32_t burstSegs = state->pushOnWriteBoundary ? getGsoBurstSegments(congestionWindow, bytesInFlight, effectiveMss) : 1;
    uint32_t newSegIdx = 0;
    uint32_t oldSndMax = state->snd_max;
    while (bytesToSend >= effectiveMss) {
        bool newData = seqGE(state->snd_nxt, oldSndMax);
        state->pushThisSegment = newData && burstSegs > 1 && newSegIdx % burstSegs == burstSegs - 1;
        uint32_t sentBytes = sendSegment(effectiveMss);
        state->pushThisSegment = false;
        if (newData)
            newSegIdx++;
        if (sentBytes == 0) { // no data left after the forward of snd_nxt
            bytesToSend = 0;
            break;
        }
        ASSERT(bytesToSend >= sentBytes);
        bytesToSend -= sentBytes;
        effectiveWin -= sentBytes;
        buffered -= sentBytes;
    }

    // MSG_EOR: a record that ends in the partial segment at the end cannot become
    // larger, so Nagle and the cork do not hold it, as Linux tcp_write_xmit(), which
    // applies them only to the last skb of the queue. They hold only the data after the
    // last record boundary.
    uint32_t recordEnd = bytesToSend > 0 && !swsHold(effectiveWin, buffered) ? getLastRecordBoundaryBefore(state->snd_nxt + bytesToSend) : state->snd_nxt;
    while (seqLess(state->snd_nxt, recordEnd)) {
        uint32_t sentBytes = sendSegment(recordEnd - state->snd_nxt);
        if (sentBytes == 0)
            break;
        bytesToSend -= sentBytes;
        effectiveWin -= sentBytes;
        buffered -= sentBytes;
    }

    if (bytesToSend > 0) {
        // Nagle's algorithm: when a TCP connection has outstanding data that has not
        // yet been acknowledged, small segments cannot be sent until the outstanding
        // data is acknowledged.
        // The SYN-ACK of a TCP Fast Open server that sends in SYN_RCVD is not
        // data: Nagle does not wait for its ACK (Linux applies Nagle to the data
        // in the write queue only). In SYN_SENT, the ACK that completes the
        // handshake carries the data of a Fast Open SYN that the SYN-ACK did not
        // acknowledge (RFC 7413 section 4.2.2: "include the data after SND.UNA if
        // data is available"), without Nagle.
        // Nagle holds only new data. After a go-back (a retransmission timeout, or a
        // SYN-ACK that did not acknowledge the data of a Fast Open SYN), snd_nxt is below
        // snd_max, and the partial segment at snd_nxt is a retransmission: RFC 9293
        // section 3.7.4 tests SND.NXT > SND.UNA, and Linux never sends a retransmission
        // through Nagle.
        bool unacknowledgedData = seqGE(state->snd_nxt, state->snd_max) && (getDataSndUna() != state->snd_max)
                && fsm.getState() != TCP_S_SYN_SENT;
        // minshallNagle, as Linux (tcp_minshall_check()): only an unacknowledged small
        // segment holds, not a flight of full-sized segments
        if (state->minshallNagle)
            unacknowledgedData = unacknowledgedData && seqGreater(state->snd_sml, state->snd_una) && seqLE(state->snd_sml, state->snd_nxt);
        bool containsFin = state->send_fin && (state->snd_nxt + bytesToSend) == state->snd_fin_seq;
        // TCP_CORK and MSG_MORE hold the partial segment at the end of the data, also with
        // nothing in flight; an uncork, TCP_NODELAY or the cork timer (corkFlush) sends it.
        // Nagle stays after an uncork, as Linux __tcp_sock_set_cork().
        bool corkHold = (state->tcp_cork || msgMoreHold) && !state->corkFlush && !containsFin && buffered < state->snd_effmss;
        if (swsHold(effectiveWin, buffered))
            EV_WARN << "Not sending to prevent Silly Window Syndrome.\n";
        else if (corkHold) {
            state->corkedDataPending = true;
            EV_WARN << "Holding the partial segment (TCP_CORK or MSG_MORE)\n";
        }
        else if (state->nagle_enabled && unacknowledgedData && !containsFin)
            EV_WARN << "Cannot send (last) segment due to Nagle, not enough data for a full segment\n";
        else {
            // a held partial carries PSH when a later SEND came without MSG_MORE, or when
            // the cork timer sends it (Linux tcp_mark_push(), tcp_write_wakeup())
            bool flushingCorked = state->corkedDataPending || state->corkFlush;
            state->pushThisSegment = flushingCorked && (state->pushHeldPartial || state->forcePushHeld);
            sendSegment(bytesToSend);
            state->pushThisSegment = false;
            state->corkedDataPending = false;
            state->pushHeldPartial = false;
        }
    }

    // The cork timer (Linux ICSK_TIME_PROBE0) runs while a held partial waits and
    // nothing is in flight; else an ACK brings the next try.
    if (state->corkedDataPending && state->snd_una == state->snd_max)
        tcpAlgorithm->scheduleCorkTimer();
    else
        tcpAlgorithm->cancelCorkTimer();

    if (old_snd_nxt == state->snd_nxt)
        return false; // no data sent

    emit(unackedSignal, state->snd_max - state->snd_una);

    // notify (once is enough)
    tcpAlgorithm->ackSent();

    if (state->sack_enabled && state->lossRecovery && old_highRxt != state->highRxt) {
        // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
        EV_DETAIL << "Retransmission sent during recovery, restarting REXMIT timer.\n";
        tcpAlgorithm->restartRexmitTimer();
    }
    else // don't measure RTT for retransmitted packets
        tcpAlgorithm->dataSent(old_snd_nxt);

    return true;
}

void TcpConnection::sendKeepAliveProbe()
{
    // RFC 9293 section 3.8.4: "Such a segment generally contains SEG.SEQ = SND.NXT-1";
    // the connection is idle, so SND.NXT = SND.UNA. No data (SHLD-12).
    EV_INFO << "Sending keepalive probe, seq=" << (state->snd_una - 1) << "\n";
    sendSegmentBelowWindow("TcpKeepAlive");
}

void TcpConnection::sendSegmentBelowWindow(const char *name)
{
    // As Linux tcp_xmit_probe_skb(): a segment without data with SEG.SEQ = SND.UNA - 1.
    // The sequence number is below the window of the peer, so the peer answers with an
    // ACK and accepts nothing. No data, and so no ECT; snd_nxt and snd_max stay. The
    // segment carries the current ACK (Linux tcp_event_ack_sent()).
    const auto& tcpHeader = makeShared<TcpHeader>();
    tcpHeader->setAckBit(true);
    tcpHeader->setSequenceNo(state->snd_una - 1);
    tcpHeader->setAckNo(state->rcv_nxt);
    tcpHeader->setWindow(updateRcvWnd());
    writeHeaderOptions(tcpHeader);
    Packet *fp = new Packet(name);
    sendToIP(fp, tcpHeader);
    tcpAlgorithm->ackSent();
}

uint32_t TcpConnection::getGsoBurstSegments(uint32_t congestionWindow, uint32_t bytesInFlight, uint32_t effectiveMss) const
{
    if (effectiveMss == 0)
        return 1;
    // tcp_cwnd_test(): at most half of the congestion window, and at most the free part
    uint32_t cwndSegs = congestionWindow / effectiveMss;
    uint32_t inFlightSegs = bytesInFlight / effectiveMss;
    if (cwndSegs == 0 || inFlightSegs >= cwndSegs)
        return 1;
    uint32_t cwndQuota = std::min(std::max(cwndSegs / 2, 1u), cwndSegs - inFlightSegs);
    // tcp_tso_autosize(): the bytes that the pacing rate gives in about 1 ms (rate >>
    // sk_pacing_shift), at least two segments. Without an RTT sample, Linux does not
    // divide the rate, so the congestion window alone sets the burst.
    simtime_t srtt = tcpAlgorithm->getSrtt();
    if (srtt <= SIMTIME_ZERO)
        return std::max(1u, cwndQuota);
    // the pacing rate of slow start (tcp_pacing_ss_ratio = 200%)
    double bytesPerBurst = (2.0 * congestionWindow / srtt.dbl()) / 1024.0;
    uint32_t tsoSegs = std::max<uint32_t>(2, (uint32_t)(bytesPerBurst / effectiveMss));
    return std::max(1u, std::min(cwndQuota, tsoSegs));
}

void TcpConnection::notifyZerocopyCompletions()
{
    // MSG_ZEROCOPY: a completion for each zero-copy SEND whose data went out now; one
    // segment can complete several small SENDs. As Linux on a device that copies the
    // data (a tun device), TCP is then done with the buffer of the application.
    while (!zerocopySeqNums.empty() && !seqGreater(zerocopySeqNums.begin()->first, state->snd_nxt)) {
        uint32_t zerocopyId = zerocopySeqNums.begin()->second;
        zerocopySeqNums.erase(zerocopySeqNums.begin());
        EV_INFO << "Notifying app: ZEROCOPY_COMPLETION id=" << zerocopyId << "\n";
        auto *indication = new Indication("ZerocopyCompletion", TCP_I_ZEROCOPY_COMPLETION);
        auto *completionInfo = new TcpZerocopyCompletionInfo();
        completionInfo->setZerocopyId(zerocopyId);
        indication->addTag<SocketInd>()->setSocketId(socketId);
        indication->setControlInfo(completionInfo);
        sendToApp(indication);
    }
}

uint32_t TcpConnection::getLastRecordBoundaryBefore(uint32_t seq) const
{
    auto it = findSeqNumAfter(eorSeqNums, seq - 1); // the first boundary at seq or after it
    if (it != eorSeqNums.begin() && seqGreater(*std::prev(it), state->snd_nxt))
        return *std::prev(it);
    return state->snd_nxt;
}

void TcpConnection::flushCorkedData(bool forcePush)
{
    // only in the states that send data
    if (fsm.getState() != TCP_S_ESTABLISHED && fsm.getState() != TCP_S_CLOSE_WAIT)
        return;
    state->corkFlush = true;
    state->forcePushHeld = forcePush;
    tcpAlgorithm->sendCommandInvoked();
    state->corkFlush = false;
    state->forcePushHeld = false;
}

bool TcpConnection::sendProbe()
{
    // RFC 9293 section 3.8.6.1: "The sending TCP peer must regularly transmit at least
    // one octet of new data (if available), or retransmit to the receiving TCP peer even
    // if the send window is zero, in order to "probe" the window." The probe is the
    // first unacknowledged octet: new data at the first probe, and the same octet again
    // at the later ones, as BSD sends it in its persist state. snd_nxt stays at snd_una,
    // so the octet goes again with the data when the window opens. The probe starts no
    // retransmission timer: the persist timer repeats it, so the connection stays open
    // as long as the peer answers (MUST-37).
    if (sendQueue->getBytesAvailable(state->snd_una) == 0) {
        EV_WARN << "Cannot send probe because send buffer is empty\n";
        return false;
    }

    // zeroWindowProbe="linux": the probe of Linux without data (tcp_xmit_probe_skb())
    if (state->linuxZeroWindowProbe) {
        EV_INFO << "Sending zero-window probe without data, seq=" << (state->snd_una - 1) << "\n";
        sendSegmentBelowWindow("ZeroWindowProbe");
        return true;
    }

    EV_INFO << "Sending 1 byte as probe, with seq=" << state->snd_una << "\n";
    unsendWindowProbe(); // the earlier probe octet: each probe is the first transmission of it
    bool afterRto = state->afterRto; // the probe octet is the octet at snd_una (see sendSegment())
    state->afterRto = false;
    sendingWindowProbe = true; // no ECT and no CWR (RFC 3168 section 6.1.6)
    sendSegment(1);
    sendingWindowProbe = false;
    state->afterRto = afterRto;
    if (seqGreater(state->snd_nxt, state->snd_max))
        state->snd_max = state->snd_nxt;
    state->snd_nxt = state->snd_una;

    emit(unackedSignal, state->snd_max - state->snd_una);

    // notify
    tcpAlgorithm->ackSent();

    return true;
}

void TcpConnection::unsendWindowProbe()
{
    if (seqGreater(state->snd_max, state->snd_una)) {
        ASSERT(state->snd_max - state->snd_una == 1);
        rexmitQueue->discardFrom(state->snd_una);
        state->snd_max = state->snd_una;
        emit(sndMaxSignal, state->snd_max);
    }
    state->snd_nxt = state->snd_una;
}

void TcpConnection::markOutstandingLostOnRto()
{
    if (rexmitQueue == nullptr || rexmitQueue->getQueueLength() == 0)
        return;
    // With SACK, TcpAlgorithmBase::processRexmitTimer() has already reset the SACKed
    // and retransmitted bits. Without SACK, the SACKed bits are inferred SACKs
    // (Linux tcp_reset_reno_sack()), and the retransmissions after the timeout are
    // the only data in flight.
    if (!state->sack_enabled) {
        rexmitQueue->resetSackedBit();
        rexmitQueue->resetRexmittedBit();
    }
    // Clamp to the scoreboard's range: snd_una may sit below the queue start (already
    // discarded) and snd_max may sit above the queue end (e.g. an outstanding FIN,
    // which carries no data byte in the rexmit queue).
    uint32_t from = state->snd_una;
    uint32_t to = state->snd_max;
    if (seqLess(from, rexmitQueue->getBufferStartSeq()))
        from = rexmitQueue->getBufferStartSeq();
    if (seqGreater(to, rexmitQueue->getBufferEndSeq()))
        to = rexmitQueue->getBufferEndSeq();
    if (seqLess(from, to))
        rexmitQueue->markLost(from, to);
}

void TcpConnection::retransmitOneSegment(bool called_at_rto)
{
    // rfc-3168, page 20:
    // ECN-capable TCP implementations MUST NOT set either ECT codepoint
    // (ECT(0) or ECT(1)) in the IP header for retransmitted data packets
    if (state && state->ect)
        state->rexmit = true;

    uint32_t old_snd_nxt = state->snd_nxt;

    // retransmit one segment at snd_una, and set snd_nxt accordingly (if not called at RTO)
    state->snd_nxt = state->snd_una;

    // ... but at the first data byte, while a TCP Fast Open server sends data in
    // SYN_RCVD: the SYN-ACK is not in the send queue, and the SYN-REXMIT timer
    // retransmits it
    if (fsm.getState() == TCP_S_SYN_RCVD && seqLess(state->snd_nxt, state->iss + 1))
        state->snd_nxt = state->iss + 1;

    // When FIN sent the snd_max - snd_nxt larger than bytes available in queue
    uint32_t bytes = std::min(std::min(state->snd_effmss, state->snd_max - state->snd_nxt),
                sendQueue->getBytesAvailable(state->snd_nxt));

    // FIN (without user data) needs to be resent
    if (bytes == 0 && state->send_fin && state->snd_fin_seq == sendQueue->getBufferEndSeq()) {
        uint32_t oldSndMax = state->snd_max;
        state->snd_max = sendQueue->getBufferEndSeq();
        EV_DETAIL << "No outstanding DATA, resending FIN, advancing snd_nxt over the FIN\n";
        state->snd_nxt = state->snd_max;
        sendFin();
        tcpAlgorithm->segmentRetransmitted(state->snd_nxt, state->snd_nxt + 1);
        state->snd_max = ++state->snd_nxt;
        if (state->snd_max != oldSndMax)
            emit(sndMaxSignal, state->snd_max);

        emit(unackedSignal, state->snd_max - state->snd_una);
    }
    else {
        ASSERT(bytes != 0);

        if (sendSegment(bytes) > 0) // 0: no data left after the forward of snd_nxt
            tcpAlgorithm->segmentRetransmitted(state->snd_una, state->snd_nxt);

        if (!called_at_rto) {
            if (seqGreater(old_snd_nxt, state->snd_nxt))
                state->snd_nxt = old_snd_nxt;
        }

        // notify
        tcpAlgorithm->ackSent();

        if (state->sack_enabled) {
            // RFC 3517, page 7: "(3) Retransmit the first data segment presumed dropped -- the segment
            // starting with sequence number HighACK + 1.  To prevent repeated
            // retransmission of the same data, set HighRxt to the highest
            // sequence number in the retransmitted segment."
            state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
        }
    }

    if (state && state->ect)
        state->rexmit = false;
}

bool TcpConnection::sendTlpProbe()
{
    // RFC 8985 section 7.3, Linux tcp_send_loss_probe(): send new data if a segment
    // of it exists and the receive window allows it. The receiver can acknowledge
    // it normally, and its ACK or SACK shows the loss of the tail.
    uint32_t available = sendQueue->getBytesAvailable(state->snd_max);
    if (available > 0 && seqLess(state->snd_max, state->snd_una + state->snd_wnd)) {
        uint32_t win = state->snd_una + state->snd_wnd - state->snd_max;
        uint32_t bytes = std::min(std::min(state->snd_mss, available), win);
        uint32_t old_snd_nxt = state->snd_nxt;
        state->snd_nxt = state->snd_max;
        uint32_t sent = sendSegment(bytes);
        if (seqGreater(old_snd_nxt, state->snd_nxt))
            state->snd_nxt = old_snd_nxt;
        if (sent > 0) {
            state->tlpRetrans = false;
            EV_INFO << "TLP: probing with " << sent << " bytes of new data\n";
            return true;
        }
    }

    // A FIN that the connection sent is the highest segment: send the FIN again
    // (Linux retransmits the last skb, which holds only the FIN then).
    if (state->send_fin && state->snd_fin_seq == sendQueue->getBufferEndSeq()
            && state->snd_max == state->snd_fin_seq + 1)
    {
        state->snd_nxt = state->snd_fin_seq;
        sendFin();
        tcpAlgorithm->segmentRetransmitted(state->snd_fin_seq, state->snd_fin_seq + 1);
        state->snd_nxt = state->snd_fin_seq + 1;
        state->tlpRetrans = true;
        EV_INFO << "TLP: probing by resending the FIN\n";
        return true;
    }

    // Else retransmit the last segment that the connection sent (the highest
    // sequence numbers, at most one MSS). The send queue holds data only, so its
    // start, not snd_una, limits the segment: an unacknowledged SYN is not in it.
    uint32_t bufStart = sendQueue->getBufferStartSeq();
    if (seqGE(bufStart, state->snd_max))
        return false; // no data to retransmit
    uint32_t len = std::min(state->snd_mss, state->snd_max - bufStart);
    uint32_t start = state->snd_max - len;

    // RFC 3168: no ECT on retransmissions (see retransmitOneSegment())
    if (state->ect)
        state->rexmit = true;
    uint32_t old_snd_nxt = state->snd_nxt;
    state->snd_nxt = start;
    uint32_t sent = sendSegment(len);
    if (sent > 0)
        tcpAlgorithm->segmentRetransmitted(start, start + sent);
    if (seqGreater(old_snd_nxt, state->snd_nxt))
        state->snd_nxt = old_snd_nxt;
    if (state->ect)
        state->rexmit = false;
    if (sent == 0)
        return false;
    state->tlpRetrans = true;
    EV_INFO << "TLP: probing by retransmitting the last " << sent << " bytes\n";
    return true;
}

void TcpConnection::retransmitData()
{
    // rfc-3168, page 20:
    // ECN-capable TCP implementations MUST NOT set either ECT codepoint
    // (ECT(0) or ECT(1)) in the IP header for retransmitted data packets
    if (state && state->ect)
        state->rexmit = true;

    // retransmit everything from snd_una
    state->snd_nxt = state->snd_una;

    // ... from the first data byte, while a TCP Fast Open server sends data in
    // SYN_RCVD (see retransmitOneSegment())
    if (fsm.getState() == TCP_S_SYN_RCVD && seqLess(state->snd_nxt, state->iss + 1))
        state->snd_nxt = state->iss + 1;

    uint32_t bytesToSend = state->snd_max - state->snd_nxt;

    // FIN (without user data) needs to be resent. bytesToSend counts an unacknowledged
    // FIN, so a FIN without data before it is found by the data in the queue, as in
    // retransmitOneSegment(): sendSegment() makes no segment without data.
    if ((bytesToSend == 0 || sendQueue->getBytesAvailable(state->snd_nxt) == 0)
            && state->send_fin && state->snd_fin_seq == sendQueue->getBufferEndSeq()) {
        uint32_t oldSndMax = state->snd_max;
        state->snd_max = sendQueue->getBufferEndSeq();
        EV_DETAIL << "No outstanding DATA, resending FIN, advancing snd_nxt over the FIN\n";
        state->snd_nxt = state->snd_max;
        sendFin();
        state->snd_max = ++state->snd_nxt;
        if (state->snd_max != oldSndMax)
            emit(sndMaxSignal, state->snd_max);

        emit(unackedSignal, state->snd_max - state->snd_una);
        return;
    }

    ASSERT(bytesToSend != 0);

    // TODO - avoid to send more than allowed - check cwnd and rwnd before retransmitting data!
    while (bytesToSend > 0) {
        uint32_t bytes = std::min(bytesToSend, state->snd_effmss);
        bytes = std::min(bytes, sendQueue->getBytesAvailable(state->snd_nxt));
        uint32_t sentBytes = sendSegment(bytes);
        if (sentBytes == 0) // no data left after the forward of snd_nxt
            break;

        // Do not send packets after the FIN.
        // fixes bug that occurs in examples/inet/bulktransfer at event #64043  T=13.861159213744
        if (state->send_fin && state->snd_nxt == state->snd_fin_seq + 1)
            break;

        ASSERT(bytesToSend >= sentBytes);
        bytesToSend -= sentBytes;
    }
    tcpAlgorithm->segmentRetransmitted(state->snd_una, state->snd_nxt);

    if (state && state->ect)
        state->rexmit = false;
}

void TcpConnection::readHeaderOptions(const Ptr<const TcpHeader>& tcpHeader)
{
    EV_INFO << "Tcp Header Option(s) received:\n";

    for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        short kind = option->getKind();
        short length = option->getLength();
        bool ok = true;

        EV_DETAIL << "Option type " << kind << " (" << optionName(kind) << "), length " << length << "\n";

        switch (kind) {
            case TCPOPTION_END_OF_OPTION_LIST: // EOL=0
            case TCPOPTION_NO_OPERATION: // NOP=1
                if (length != 1) {
                    EV_ERROR << "ERROR: option length incorrect\n";
                    ok = false;
                }
                break;

            case TCPOPTION_MAXIMUM_SEGMENT_SIZE: // MSS=2
                ok = processMSSOption(tcpHeader, *check_and_cast<const TcpOptionMaxSegmentSize *>(option));
                break;

            case TCPOPTION_WINDOW_SCALE: // WS=3
                ok = processWSOption(tcpHeader, *check_and_cast<const TcpOptionWindowScale *>(option));
                break;

            case TCPOPTION_SACK_PERMITTED: // SACK_PERMITTED=4
                ok = processSACKPermittedOption(tcpHeader, *check_and_cast<const TcpOptionSackPermitted *>(option));
                break;

            case TCPOPTION_SACK: { // SACK=5
                // A SACK block from a peer we never negotiated SACK with, or one that
                // arrives before the algorithm has a SACK-capable recovery object, is
                // malformed input -- drop the option, never abort the simulation.
                auto *recovery = state->sack_enabled ? dynamic_cast<Rfc6675Recovery *>(tcpAlgorithm->getRecovery()) : nullptr;
                if (recovery == nullptr) {
                    EV_ERROR << "ERROR: " << (state->sack_enabled ? "no SACK-capable recovery in use" : "SACK received but sack_enabled is false") << ", dropping SACK option\n";
                    ok = false;
                }
                else
                    ok = recovery->processSACKOption(tcpHeader, *check_and_cast<const TcpOptionSack *>(option));
                break;
            }

            case TCPOPTION_TIMESTAMP: // TS=8
                ok = processTSOption(tcpHeader, *check_and_cast<const TcpOptionTimestamp *>(option));
                break;

            case TCPOPTION_TCP_FASTOPEN: // TFO=34
                ok = processFastOpenOption(tcpHeader, *check_and_cast<const TcpOptionTcpFastOpen *>(option));
                break;

            case TCPOPTION_RFC3692_STYLE_EXPERIMENT_2: // 254: the experimental Fast Open option, if enabled
                if (state->fastopenExpOptionEnabled && dynamic_cast<const TcpOptionTcpFastOpenExp *>(option))
                    ok = processFastOpenExpOption(tcpHeader, *check_and_cast<const TcpOptionTcpFastOpenExp *>(option));
                else
                    EV_ERROR << "ERROR: Unsupported Tcp option kind " << kind << "\n";
                break;

            case TCPOPTION_ACCECN0: // 172
            case TCPOPTION_ACCECN1: // 174
                // RFC 9768 section 3.2.3.2.1: "If there is somehow an AccECN Option on a
                // SYN, it MUST be ignored when forwarded or received."
                if (!tcpHeader->getSynBit() || tcpHeader->getAckBit()) {
                    state->sawAccEcnOpt = true;
                    // section 3.2.3.2.4: a host "MAY check that the initial value of the EE0B
                    // field or the EE1B field is non-zero" in the first option, and if either
                    // is zero, it "will switch into a mode that ignores AccECN Options for this
                    // half-connection" (as Linux tcp_accecn_option_init())
                    auto accEcnOption = check_and_cast<const TcpOptionAccEcn *>(option);
                    int numFields = std::min(3, (length - 2) / 3);
                    bool order0 = kind == TCPOPTION_ACCECN0;
                    bool e0 = numFields >= (order0 ? 1 : 3);
                    bool e1 = numFields >= (order0 ? 3 : 1);
                    if (!state->accEcnOptionChecked && (e0 || e1)) {
                        state->accEcnOptionChecked = true;
                        if ((e0 && accEcnOption->getEct0Bytes() == 0) || (e1 && accEcnOption->getEct1Bytes() == 0)) {
                            state->accEcnOptionIgnored = true;
                            EV_INFO << "AccECN option with a zeroed EE0B or EE1B field: the AccECN options are ignored\n";
                        }
                    }
                }
                break;

            // TODO add new TCPOptions here once they are implemented
            // TODO delegate to TcpAlgorithm as well -- it may want to recognized additional options

            default:
                EV_ERROR << "ERROR: Unsupported Tcp option kind " << kind << "\n";
                break;
        }
        (void)ok; // unused
    }
}

bool TcpConnection::processMSSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionMaxSegmentSize& option)
{
    if (option.getLength() != 4) {
        EV_ERROR << "ERROR: MSS option length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: Tcp Header Option MSS received, but in unexpected state\n";
        return false;
    }

    // RFC 2581, page 1:
    // "The SMSS is the size of the largest segment that the sender can transmit.
    // This value can be based on the maximum transmission unit of the network,
    // the path MTU discovery [MD90] algorithm, RMSS (see next item), or other
    // factors.  The size does not include the TCP/IP headers and options."
    //
    // "The RMSS is the size of the largest segment the receiver is willing to accept.
    // This is the value specified in the MSS option sent by the receiver during
    // connection startup.  Or, if the MSS option is not used, 536 bytes [Bra89].
    // The size does not include the TCP/IP headers and options."
    //
    //
    // The value of snd_mss (SMSS) is set to the minimum of snd_mss (local parameter) and
    // the value specified in the MSS option received during connection startup.
    state->peerAdvertisedMss = option.getMaxSegmentSize(); // before the local limit, for the Fast Open cache
    state->snd_mss = std::min(state->snd_mss, (uint32_t)option.getMaxSegmentSize());

    if (state->snd_mss == 0)
        state->snd_mss = 536;

    // Store negotiated MSS for PMTUD: this is the value we restore after the probe timeout
    state->pmtudOriginalMss = state->snd_mss;

    EV_INFO << "Tcp Header Option MSS(=" << option.getMaxSegmentSize() << ") received, SMSS is set to " << state->snd_mss << "\n";
    return true;
}

void TcpConnection::applyDefaultSendMss(const Ptr<const TcpHeader>& tcpHeader, const L3Address& peerAddr)
{
    // RFC 9293, section 3.7.1:
    //"
    // If an MSS Option is not received at connection setup, TCP
    // implementations MUST assume a default send MSS of 536 (576 - 40) for
    // IPv4 or 1220 (1280 - 60) for IPv6 (MUST-15).
    //"
    for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++)
        if (tcpHeader->getHeaderOption(i)->getKind() == TCPOPTION_MAXIMUM_SEGMENT_SIZE)
            return;
    uint32_t defaultMss = peerAddr.getType() == L3Address::IPv6 ? 1220 : 536;
    if (state->snd_mss > defaultMss) {
        EV_INFO << "No MSS option received, SMSS is set to the default " << defaultMss << " (RFC 9293 MUST-15)\n";
        state->snd_mss = defaultMss;
        state->pmtudOriginalMss = state->snd_mss;
    }
}

bool TcpConnection::processWSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionWindowScale& option)
{
    if (option.getLength() != 3) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: Tcp Header Option WS received, but in unexpected state\n";
        return false;
    }

    state->rcv_ws = true;
    state->ws_enabled = state->ws_support && state->snd_ws && state->rcv_ws;
    state->snd_wnd_scale = option.getWindowScale();
    EV_INFO << "Tcp Header Option WS(=" << state->snd_wnd_scale << ") received, WS (ws_enabled) is set to " << state->ws_enabled << "\n";

    if (state->snd_wnd_scale > 14) { // RFC 7323 section 2.3: "the shift count MUST be limited to 14"
        EV_ERROR << "ERROR: Tcp Header Option WS received but shift count value is exceeding 14\n";
        state->snd_wnd_scale = 14;
    }

    return true;
}

bool TcpConnection::processTSOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTimestamp& option)
{
    // RFC 7323 section 5.2: "information from the Timestamps option MUST NOT be used to
    // update connection state information" of a RST
    if (tcpHeader->getRstBit())
        return true;

    // Eifel input (RFC 3522 / Linux rx_opt.rcv_tsecr): remember the echo so the
    // undo logic can compare it against the first retransmission's timestamp.
    if (tcpHeader->getAckBit() && option.getEchoedTimestamp() != 0)
        state->lastRcvdTSecr = option.getEchoedTimestamp();

    if (option.getLength() != 10) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if ((!state->ts_enabled && fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) ||
        (state->ts_enabled && fsm.getState() != TCP_S_SYN_RCVD && fsm.getState() != TCP_S_ESTABLISHED &&
         fsm.getState() != TCP_S_FIN_WAIT_1 && fsm.getState() != TCP_S_FIN_WAIT_2 && fsm.getState() != TCP_S_CLOSE_WAIT &&
         fsm.getState() != TCP_S_CLOSING && fsm.getState() != TCP_S_LAST_ACK && fsm.getState() != TCP_S_TIME_WAIT))
    {
        EV_ERROR << "ERROR: Tcp Header Option TS received, but in unexpected state\n";
        return false;
    }

    if (!state->ts_enabled) {
        state->rcv_initial_ts = true;
        state->ts_enabled = state->ts_support && state->snd_initial_ts && state->rcv_initial_ts;
        EV_INFO << "Tcp Header Option TS(TSval=" << option.getSenderTimestamp() << ", TSecr=" << option.getEchoedTimestamp() << ") received, TS (ts_enabled) is set to " << state->ts_enabled << "\n";
    }
    else
        EV_INFO << "Tcp Header Option TS(TSval=" << option.getSenderTimestamp() << ", TSecr=" << option.getEchoedTimestamp() << ") received\n";

    // RFC 7323 appendix D: the TSval of a SYN in LISTEN, and of a SYN or a SYN-ACK in
    // SYN_SENT, goes to TS.Recent, so that the SYN-ACK and the ACK that completes the
    // handshake echo it. No ACK has gone out yet, so the rule of Last.ACK.sent below
    // cannot take it.
    if (tcpHeader->getSynBit() && (fsm.getState() == TCP_S_LISTEN || fsm.getState() == TCP_S_SYN_SENT)) {
        if (state->ts_support) {
            state->ts_recent = option.getSenderTimestamp();
            state->ts_recent_stamp = simTime();
            EV_DETAIL << "Setting ts_recent from the handshake segment: ts_recent=" << state->ts_recent << "\n";
        }
    }
    // a synchronized connection updates TS.Recent after the window test, see updateTsRecent()

    return true;
}

void TcpConnection::updateTsRecent(const Ptr<const TcpHeader>& tcpHeader)
{
    // RFC 7323 section 5.3: "R3) If an arriving segment satisfies SEG.TSval >= TS.Recent
    // and SEG.SEQ <= Last.ACK.sent (see Section 4.3), then record its timestamp in
    // TS.Recent", after the window test (R2), so a segment outside the window does not
    // set it. A RST changes no timestamp state (section 5.2). An older TSval sets
    // TS.Recent only when TS.Recent is invalid after 24 days of idle time (section 5.5);
    // else PAWS has dropped the segment.
    const TcpOptionTimestamp *option = state->ts_enabled && !tcpHeader->getRstBit() ? findTSOption(tcpHeader) : nullptr;
    if (option == nullptr)
        return;
    // tsRecentAfterAckCheck: as Linux, not from a segment whose ACK the connection drops
    // later, because it acknowledges data that was not sent
    if (state->tsRecentAfterAckCheck && tcpHeader->getAckBit() && seqGreater(tcpHeader->getAckNo(), state->snd_max)) {
        EV_DETAIL << "Not updating ts_recent: the segment acknowledges unsent data\n";
        return;
    }
    bool tsRecentValid = (simTime() - state->ts_recent_stamp) <= PAWS_IDLE_TIME_THRESH;
    bool olderTsval = seqLess(option->getSenderTimestamp(), state->ts_recent);
    if ((!olderTsval || !tsRecentValid) && seqLE(tcpHeader->getSequenceNo(), state->last_ack_sent)) {
        // a newer timestamp lets the AccECN decode read this ACK (Linux FLAG_TS_PROGRESS)
        if (seqGreater(option->getSenderTimestamp(), state->ts_recent))
            state->accEcnTsProgress = true;
        state->ts_recent = option->getSenderTimestamp();
        state->ts_recent_stamp = simTime();
        EV_DETAIL << "Updating ts_recent from segment: new ts_recent=" << state->ts_recent << "\n";
    }
}

bool TcpConnection::processSACKPermittedOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionSackPermitted& option)
{
    if (option.getLength() != 2) {
        EV_ERROR << "ERROR: length incorrect\n";
        return false;
    }

    if (fsm.getState() != TCP_S_LISTEN && fsm.getState() != TCP_S_SYN_SENT) {
        EV_ERROR << "ERROR: Tcp Header Option SACK_PERMITTED received, but in unexpected state\n";
        return false;
    }

    state->rcv_sack_perm = true;
    state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
    EV_INFO << "Tcp Header Option SACK_PERMITTED received, SACK (sack_enabled) is set to " << state->sack_enabled << "\n";
    return true;
}

uint32_t TcpConnection::calculateEffectiveMss()
{
    // RFC 5681 defines SMSS as the size of the largest segment that the sender
    // can transmit, without the TCP/IP headers and options. Of the options of an
    // established connection, only the timestamp option is on every segment:
    // 10 bytes and 2 bytes of padding. The SACK option is not counted: its length
    // varies, and it is on few segments of a data sender.
    return state->snd_mss - (state->ts_enabled ? 10 + 2 : 0);
}

void TcpConnection::resolveAddressFamilyMss()
{
    // mss = -1, as Linux derives the MSS from the MTU of the route (tcp_mss_clamp()):
    // the MTU of 1500 bytes less the IP header and the TCP header, before the options of
    // the SYN are read, so that the MSS of the peer limits this value and not the other
    // way round; TCP_MAXSEG limits it too
    if (!mssFromAddressFamily)
        return;
    mssFromAddressFamily = false;
    state->snd_mss = remoteAddr.getType() == L3Address::IPv6 ? 1440 : 1460;
    if (maxSegSockopt > 0 && (uint32_t)maxSegSockopt < state->snd_mss)
        state->snd_mss = maxSegSockopt;
    state->advertisedMss = state->snd_mss;
    state->snd_effmss = calculateEffectiveMss();
    EV_DETAIL << "The MSS of the address family is " << state->snd_mss << "\n";
}

uint32_t TcpConnection::getMssClamp() const
{
    // Linux mss_clamp: the MSS option of the peer, else the default of RFC 9293 (MUST-15),
    // limited by TCP_MAXSEG
    uint32_t clamp = state->peerAdvertisedMss > 0 ? state->peerAdvertisedMss : (remoteAddr.getType() == L3Address::IPv6 ? 1220 : 536);
    if (maxSegSockopt > 0 && (uint32_t)maxSegSockopt < clamp)
        clamp = maxSegSockopt;
    return clamp;
}

uint32_t TcpConnection::mtuToMss(uint32_t mtu) const
{
    // Linux tcp_mtu_to_mss(): the MTU less the IP header and the TCP header, at most
    // mss_clamp, less the options of each segment (the timestamps); snd_mss of INET
    // counts these options, as mss_clamp does
    uint32_t headers = (remoteAddr.getType() == L3Address::IPv6 ? 40 : 20) + B(TCP_MIN_HEADER_LENGTH).get();
    uint32_t mss = std::min(mtu > headers ? mtu - headers : 0, getMssClamp());
    uint32_t options = state->ts_enabled ? 12 : 0;
    return mss > options ? mss - options : 0;
}

uint32_t TcpConnection::mssToMtu(uint32_t mss) const
{
    return mss + (state->ts_enabled ? 12 : 0) + B(TCP_MIN_HEADER_LENGTH).get() + (remoteAddr.getType() == L3Address::IPv6 ? 40 : 20);
}

void TcpConnection::retransmitSynAfterMtuReduction(uint32_t newMss)
{
    // in SYN_SENT, as Linux tcp_v4_mtu_reduced() and tcp_v6_mtu_reduced(): the MSS option
    // of the next SYN falls too (advmss of the route). The error shows that a router
    // dropped the SYN, but only a SYN with data goes again at once, as a bare SYN
    // (tcp_simple_retransmit(), kernel commit c31b70c9968f); a SYN without data is not
    // larger than the new MSS, and it waits for its timer
    if (newMss < state->advertisedMss)
        state->advertisedMss = newMss;
    if (seqGreater(state->snd_max, state->iss + 1)) {
        EV_DETAIL << "The SYN carried data: retransmitting a bare SYN at once\n";
        state->syn_rexmit_count++;
        sendSyn();
        rescheduleAfter(state->syn_rexmit_timeout, synRexmitTimer);
    }
}

void TcpConnection::limitMtuSearch(uint32_t mtu)
{
    // Linux tcp_sync_mss(): the upper bound of the search does not exceed the route MTU
    if (!state->mtupEnabled || state->mtupSearchHigh <= mtu)
        return;
    state->mtupSearchHigh = mtu;
    if (state->mtupSearchLow > mtu)
        state->mtupSearchLow = mtu;
    EV_INFO << "RFC 4821: the route MTU " << mtu << " limits the path MTU search, which runs from "
            << state->mtupSearchLow << " to " << state->mtupSearchHigh << "\n";
}

void TcpConnection::initMtuProbing()
{
    // mtuProbing, as Linux tcp_mtup_init(): the search runs between the MTU of the base MSS
    // and the MTU of the MSS of the peer; the route MTU (pathMtu, else the MTU of the
    // negotiated MSS) limits the MSS that a success gives
    if (!state->mtupEnabled)
        return;
    uint32_t headers = (remoteAddr.getType() == L3Address::IPv6 ? 40 : 20) + B(TCP_MIN_HEADER_LENGTH).get();
    state->mtupSearchHigh = getMssClamp() + headers;
    state->mtupSearchLow = mssToMtu((int)tcpMain->par("baseMss"));
    state->mtupProbeSize = 0;
    int pathMtu = pathMtuSockopt > 0 ? pathMtuSockopt : (int)tcpMain->par("pathMtu");
    state->pathMtu = pathMtu > 0 ? pathMtu : state->snd_mss + headers;
    EV_INFO << "RFC 4821: the path MTU search runs from " << state->mtupSearchLow << " to " << state->mtupSearchHigh
            << ", the route MTU is " << state->pathMtu << "\n";
    // an active opener starts at the base MSS, because Linux tcp_connect_init() arms the
    // search before tcp_sync_mss() ("use initial MSS of tcp_base_mss"); a passive
    // connection keeps its MSS, as the Linux script mtu_probe/basic-v4 shows
    uint32_t baseMss = mtuToMss(state->mtupSearchLow) + (state->ts_enabled ? 12 : 0);
    if (state->active && baseMss < state->snd_mss) {
        state->snd_mss = baseMss;
        state->snd_effmss = calculateEffectiveMss();
        EV_INFO << "RFC 4821: an active opener starts at the base MSS, SMSS is " << state->snd_mss << "\n";
    }
}

int64_t TcpConnection::getMtuProbeBytes(uint32_t buffered, uint32_t congestionWindow, uint32_t bytesInFlight) const
{
    // mtuProbing, as Linux tcp_mtu_probe(): one probe at a time, from new data, in the
    // congestion state Open (no duplicate ACK either), with a congestion window of 11
    // segments, and not while this side sends SACK blocks, whose length changes the size
    // of the probe
    uint32_t mss = state->snd_effmss;
    bool sackBlocksToSend = state->sack_enabled
        && (state->snd_dsack || receiveQueue->getAmountOfBufferedBytes() > receiveQueue->getAcknowledgedDataLength());
    if (state->mtupProbeSize != 0 || seqLess(state->snd_nxt, state->snd_max) || deriveLinuxCaState() != 0
        || state->dupacks > 0 || congestionWindow < 11 * mss || sackBlocksToSend)
        return -1;
    // the middle of the search; none when the search is narrower than 8 bytes (Linux
    // tcp_probe_threshold), and none at or below the MSS in use
    uint32_t probe = mtuToMss((state->mtupSearchHigh + state->mtupSearchLow) / 2);
    if (probe > mtuToMss(state->mtupSearchHigh) || state->mtupSearchHigh - state->mtupSearchLow < 8 || probe <= mss)
        return -1;
    // and none larger than the route MTU, which the packet could not leave
    if (mssToMtu(probe) > state->pathMtu)
        return -1;
    // the probe and reordering + 1 segments after it, so that duplicate ACKs find a lost
    // probe (RFC 4821 section 7.4)
    uint32_t sizeNeeded = probe + (state->reordering + 1) * mss;
    if (buffered < sizeNeeded || state->snd_wnd < sizeNeeded)
        return -1;
    // a probe does not span a record boundary (MSG_EOR; Linux tcp_can_coalesce_send_queue_head())
    auto boundary = findSeqNumAfter(eorSeqNums, state->snd_nxt);
    if (boundary != eorSeqNums.end() && seqLess(*boundary, state->snd_nxt + probe))
        return -1;
    // the sender waits for room at the window edge, and for two free segments in the
    // pipe, unless nothing is in flight
    if (seqGreater(state->snd_nxt + sizeNeeded, getDataSndUna() + state->snd_wnd))
        return 0;
    if (bytesInFlight + 2 * mss > congestionWindow)
        return bytesInFlight == 0 ? -1 : 0;
    return probe;
}

void TcpConnection::mtuProbeSucceeded()
{
    // Linux tcp_mtup_probe_success(): the path carries the probe, so the lower bound rises
    // to it, and the MSS follows, up to the route MTU (tcp_sync_mss())
    uint32_t probeMtu = state->mtupProbeSize;
    state->mtupSearchLow = probeMtu;
    state->mtupProbeSize = 0;
    uint32_t mss = std::min(mtuToMss(state->pathMtu), mtuToMss(state->mtupSearchLow)) + (state->ts_enabled ? 12 : 0);
    if (mss > state->snd_mss) {
        state->snd_mss = mss;
        state->snd_effmss = calculateEffectiveMss();
    }
    EV_INFO << "RFC 4821: the MTU probe of " << probeMtu << " is acknowledged, snd_mss is " << state->snd_mss
            << ", the search runs from " << state->mtupSearchLow << " to " << state->mtupSearchHigh << "\n";
}

void TcpConnection::mtuProbeFailed()
{
    // Linux tcp_mtup_probe_failed(): the path does not carry the probe, so the search
    // continues below it, and below a route MTU that an ICMP error reported
    state->mtupSearchHigh = std::min(state->mtupSearchHigh, state->mtupProbeSize - 1);
    EV_INFO << "RFC 4821: the MTU probe of " << state->mtupProbeSize << " is lost, the search runs from "
            << state->mtupSearchLow << " to " << state->mtupSearchHigh << "\n";
    state->mtupProbeSize = 0;
}

bool TcpConnection::processFastOpenOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTcpFastOpen& option)
{
    std::vector<uint8_t> cookie(option.getCookieArraySize());
    for (size_t i = 0; i < cookie.size(); i++)
        cookie[i] = option.getCookie(i);
    return processFastOpenCookieBytes(cookie);
}

bool TcpConnection::processFastOpenExpOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionTcpFastOpenExp& option)
{
    // the server answers in the option of the client, and the client caches the
    // option of the cookie (Linux foc->exp)
    state->fastopenPeerUsedExpOption = true;
    std::vector<uint8_t> cookie(option.getCookieArraySize());
    for (size_t i = 0; i < cookie.size(); i++)
        cookie[i] = option.getCookie(i);
    return processFastOpenCookieBytes(cookie);
}

bool TcpConnection::processFastOpenCookieBytes(const std::vector<uint8_t>& cookie)
{
    // RFC 7413 section 4.2: without Fast Open, a server "MUST ignore all cookie
    // options", and so does a client
    bool serverSyn = state->fastopenServerEnabled && fsm.getState() == TCP_S_LISTEN;
    bool clientSynAck = state->fastopenClientEnabled && fsm.getState() == TCP_S_SYN_SENT;
    if (!serverSyn && !clientSynAck)
        return true;

    // RFC 7413 section 4.1.1: the cookie is empty, or it has an even number of
    // bytes from 4 to 16, and "Options with invalid Length values ... MUST be
    // ignored"
    size_t cookieLength = cookie.size();
    if (cookieLength != 0 && (cookieLength < 4 || cookieLength > 16 || cookieLength % 2 != 0)) {
        EV_WARN << "Fast Open: ignoring the option with a cookie of " << cookieLength << " bytes\n";
        return false;
    }

    if (serverSyn) {
        if (cookieLength == 0) {
            // RFC 7413 section 4.2.1: the client requests a cookie
            state->fastopenCookieRequested = true;
            state->fastopenCookieToSend = tcpMain->generateFastOpenCookie(localAddr, remoteAddr, state->fastopenCookieBytes);
            state->fastopenSendCookieOption = true;
            EV_INFO << "Fast Open: cookie requested\n";
        }
        else if (state->fastopenLenientCookieValidation || cookie == tcpMain->generateFastOpenCookie(localAddr, remoteAddr, cookieLength)) {
            state->fastopenCookieValid = true;
            EV_INFO << "Fast Open: valid cookie\n";
        }
        else {
            // RFC 7413 section 4.1.2: "if the incoming cookie fails the check, a
            // valid cookie is readily available to be sent to the client"
            state->fastopenCookieToSend = tcpMain->generateFastOpenCookie(localAddr, remoteAddr, state->fastopenCookieBytes);
            state->fastopenSendCookieOption = true;
            EV_INFO << "Fast Open: the cookie is not valid, giving a new one\n";
        }
    }
    else {
        // RFC 7413 section 4.2.2, client step 1: cache the cookie with the MSS of
        // the server, before the local limit. As Linux, cache it only when the SYN
        // carried a Fast Open option.
        if (!state->fastopenSynCarriedOption)
            EV_INFO << "Fast Open: ignoring a cookie that the SYN did not request\n";
        else if (cookieLength > 0) {
            uint32_t cacheMss = state->peerAdvertisedMss > 0 ? state->peerAdvertisedMss : state->snd_mss;
            tcpMain->setFastOpenCookie(remoteAddr, cookie, cacheMss);
            tcpMain->setFastOpenCookieExpForm(remoteAddr, state->fastopenPeerUsedExpOption);
            EV_INFO << "Fast Open: caching the cookie of " << cookieLength << " bytes for " << remoteAddr
                    << (state->fastopenPeerUsedExpOption ? " (experimental option)" : "") << "\n";
        }
    }
    return true;
}

// a TCP Fast Open option (kind 34), or the experimental option (kind 254 with the
// magic number 0xF989, RFC 7413 appendix A); with alignment, 2 NOPs before it make
// its length a multiple of 4
static void appendFastOpenOption(const Ptr<TcpHeader>& tcpHeader, const std::vector<uint8_t>& cookie, bool align, bool exp)
{
    int length = (exp ? 4 : 2) + cookie.size();
    if (align && length % 4 == 2) {
        tcpHeader->appendHeaderOption(new TcpOptionNop());
        tcpHeader->appendHeaderOption(new TcpOptionNop());
    }
    if (exp) {
        auto option = new TcpOptionTcpFastOpenExp();
        option->setExpId(0xF989);
        option->setCookieArraySize(cookie.size());
        for (size_t i = 0; i < cookie.size(); i++)
            option->setCookie(i, cookie[i]);
        option->setLength(length);
        tcpHeader->appendHeaderOption(option);
    }
    else {
        auto option = new TcpOptionTcpFastOpen();
        option->setCookieArraySize(cookie.size());
        for (size_t i = 0; i < cookie.size(); i++)
            option->setCookie(i, cookie[i]);
        option->setLength(length);
        tcpHeader->appendHeaderOption(option);
    }
}

TcpHeader TcpConnection::writeHeaderOptions(const Ptr<TcpHeader>& tcpHeader)
{
    // SYN flag set and connetion in INIT or LISTEN state (or after synRexmit timeout)
    if (tcpHeader->getSynBit() && (fsm.getState() == TCP_S_INIT || fsm.getState() == TCP_S_LISTEN
                                || ((fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD)
                                    && (state->syn_rexmit_count > 0
                                        // TCP Fast Open: the deferred SYN goes out in SYN_SENT
                                        || state->fastopenSynDeferred
                                        // simultaneous open: the SYN-ACK that answers the
                                        // crossing SYN goes out in SYN_SENT, and it carries the
                                        // handshake options as any other SYN segment
                                        || tcpHeader->getAckBit()))))
    {
        // MSS header option
        // RFC 9293, section 3.7.1: the MSS option announces the maximum segment
        // size that this side can receive. By the time of the SYN-ACK, snd_mss is
        // already the minimum with the MSS of the peer, so it is not announced.
        if (tcpMain->sendMssOption && state->advertisedMss > 0) {
            TcpOptionMaxSegmentSize *option = new TcpOptionMaxSegmentSize();
            option->setMaxSegmentSize(state->advertisedMss);
            tcpHeader->appendHeaderOption(option);
            EV_INFO << "Tcp Header Option MSS(=" << state->advertisedMss << ") sent\n";
        }

        // WS header option
        if (state->ws_support && (state->rcv_ws || (fsm.getState() == TCP_S_INIT
                                                    || (fsm.getState() == TCP_S_SYN_SENT && (state->syn_rexmit_count > 0 || state->fastopenSynDeferred)))))
        {
            if (tcpMain->alignOptions) // 1 padding byte
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP

            // Update WS variables
            if (state->ws_manual_scale > -1) {
                state->rcv_wnd_scale = state->ws_manual_scale;
            }
            else {
                // the largest window that the connection can offer: the advertised window,
                // or with windowAutoTuning the window that auto-tuning can reach: the bound
                // of the receive buffer, unless SO_RCVBUF locks the buffer (Linux
                // tcp_select_initial_window() represents tcp_rmem[2])
                ulong scaled_rcv_wnd = receiveQueue->getFirstSeqNo() + std::max(state->maxRcvBuffer, state->window_clamp) - state->rcv_nxt;
                if (state->rcv_ssthresh > 0 && !state->rcvbufLocked)
                    scaled_rcv_wnd = std::max<ulong>(scaled_rcv_wnd, state->rcvBufferMax);
                state->rcv_wnd_scale = 0;

                while (scaled_rcv_wnd > TCP_MAX_WIN && state->rcv_wnd_scale < 14) { // RFC 7323 section 2.3: "the shift count MUST be limited to 14"
                    scaled_rcv_wnd = scaled_rcv_wnd >> 1;
                    state->rcv_wnd_scale++;
                }
            }

            TcpOptionWindowScale *option = new TcpOptionWindowScale();
            option->setWindowScale(state->rcv_wnd_scale); // rcv_wnd_scale is also set in scaleRcvWnd()
            state->snd_ws = true;
            state->ws_enabled = state->ws_support && state->snd_ws && state->rcv_ws;
            EV_INFO << "Tcp Header Option WS(=" << option->getWindowScale() << ") sent, WS (ws_enabled) is set to " << state->ws_enabled << "\n";
            tcpHeader->appendHeaderOption(option);
        }

        // SACK_PERMITTED header option
        if (state->sack_support && (state->rcv_sack_perm || (fsm.getState() == TCP_S_INIT
                                                             || (fsm.getState() == TCP_S_SYN_SENT && (state->syn_rexmit_count > 0 || state->fastopenSynDeferred)))))
        {
            if (tcpMain->alignOptions && !state->ts_support) { // if TS is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
            }

            tcpHeader->appendHeaderOption(new TcpOptionSackPermitted());

            // Update SACK variables
            state->snd_sack_perm = true;
            state->sack_enabled = state->sack_support && state->snd_sack_perm && state->rcv_sack_perm;
            EV_INFO << "Tcp Header Option SACK_PERMITTED sent, SACK (sack_enabled) is set to " << state->sack_enabled << "\n";
        }

        // TS header option
        if (state->ts_support && (state->rcv_initial_ts || (fsm.getState() == TCP_S_INIT
                                                            || (fsm.getState() == TCP_S_SYN_SENT && (state->syn_rexmit_count > 0 || state->fastopenSynDeferred)))))
        {
            if (tcpMain->alignOptions && !state->sack_support) { // if SACK is supported by host, do not add NOPs to this segment
                // 2 padding bytes
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
            }

            TcpOptionTimestamp *option = new TcpOptionTimestamp();

            // Update TS variables
            // RFC 7323 section 3.2: "The TSval field contains the current value of the timestamp clock of the TCP sending the option."
            option->setSenderTimestamp(convertSimtimeToTS(simTime()));

            // RFC 7323 section 4.3: "(3)  When a TSopt is sent, its TSecr field is set to the current TS.Recent value."
            // RFC 7323 section 3.2:
            // "The TSecr field is valid if the ACK bit is set in the TCP header.  If
            // the ACK bit is not set in the outgoing TCP header, the sender of that
            // segment SHOULD set the TSecr field to zero."
            option->setEchoedTimestamp(tcpHeader->getAckBit() ? state->ts_recent : 0);

            state->snd_initial_ts = true;
            state->ts_enabled = state->ts_support && state->snd_initial_ts && state->rcv_initial_ts;
            EV_INFO << "Tcp Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent, TS (ts_enabled) is set to " << state->ts_enabled << "\n";
            tcpHeader->appendHeaderOption(option);
        }

        // TCP Fast Open (RFC 7413), server: the SYN-ACK gives a cookie. A
        // retransmitted SYN-ACK goes out without it (RFC 7413 section 4.2.2).
        if (state->fastopenServerEnabled && state->fastopenSendCookieOption && tcpHeader->getAckBit()
            && state->syn_rexmit_count == 0)
        {
            appendFastOpenOption(tcpHeader, state->fastopenCookieToSend, tcpMain->alignOptions, state->fastopenPeerUsedExpOption);
            EV_INFO << "Tcp Header Option FAST_OPEN (cookie of " << state->fastopenCookieToSend.size() << " bytes) sent\n";
        }

        // TCP Fast Open (RFC 7413 section 4.2), client: the first SYN of a connection
        // that asked for Fast Open carries the cached cookie, or an empty option that
        // requests a cookie. A retransmitted SYN goes out without it (RFC 7413
        // section 4.2.1), and so does the SYN-ACK of a simultaneous open.
        if (state->fastopenClientEnabled && state->fastopenRequested && state->syn_rexmit_count == 0
            && !tcpHeader->getAckBit() && !state->fastopenClientNoCookieRequired)
        {
            std::vector<uint8_t> cookie;
            bool cachedCookie = !state->fastopenCookieRequestPending && tcpMain->getFastOpenCookie(remoteAddr, cookie);
            if (cachedCookie || state->fastopenCookieRequestPending) {
                bool exp = state->fastopenExpOptionEnabled && tcpMain->getFastOpenUseExpOption(remoteAddr);
                appendFastOpenOption(tcpHeader, cookie, tcpMain->alignOptions, exp);
                state->fastopenSynCarriedOption = true;
                EV_INFO << "Tcp Header Option FAST_OPEN (cookie of " << cookie.size() << " bytes) sent\n";
            }
        }

        // AccECN (RFC 9768 section 3.2.3.2.1): "A TCP Server that confirms its support
        // for AccECN ... SHOULD include an AccECN TCP Option on the SYN/ACK", with each
        // field that fits; a retransmitted SYN-ACK goes out without it (section
        // 3.2.3.2.2). The SYN never carries it ("MUST NOT").
        if (tcpHeader->getAckBit() && state->accEcnNegotiated && state->accEcnOptionEnabled && state->syn_rexmit_count == 0)
            appendAccEcnOption(tcpHeader, tcpMain->accEcnOptionLinux ? 1 : 0, 3, 0);

        // TODO add new TCPOptions here once they are implemented
    }
    else if (fsm.getState() == TCP_S_SYN_SENT || fsm.getState() == TCP_S_SYN_RCVD
             || fsm.getState() == TCP_S_ESTABLISHED || fsm.getState() == TCP_S_FIN_WAIT_1
             || fsm.getState() == TCP_S_FIN_WAIT_2 || fsm.getState() == TCP_S_CLOSE_WAIT
             || fsm.getState() == TCP_S_CLOSING || fsm.getState() == TCP_S_LAST_ACK
             || fsm.getState() == TCP_S_TIME_WAIT) // connection is not in INIT or LISTEN state
    {
        // RFC 7323 section 3.2: "the TSopt MUST be sent in every non-<RST> segment for
        // the duration of the connection", also in CLOSING, LAST_ACK and TIME_WAIT
        // TS header option
        if (state->ts_enabled) { // Is TS enabled?
            if (tcpMain->alignOptions && !(state->sack_enabled && (state->snd_sack || state->snd_dsack))) { // if SACK is enabled and SACKs need to be added, do not add NOPs to this segment
                // 2 padding bytes
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
                tcpHeader->appendHeaderOption(new TcpOptionNop()); // NOP
            }

            TcpOptionTimestamp *option = new TcpOptionTimestamp();

            // Update TS variables
            // RFC 7323 section 3.2: "The TSval field contains the current value of the timestamp clock of the TCP sending the option."
            option->setSenderTimestamp(convertSimtimeToTS(simTime()));

            // RFC 7323 section 4.3: "(3)  When a TSopt is sent, its TSecr field is set to the current TS.Recent value."
            // RFC 7323 section 3.2:
            // "The TSecr field is valid if the ACK bit is set in the TCP header.  If
            // the ACK bit is not set in the outgoing TCP header, the sender of that
            // segment SHOULD set the TSecr field to zero."
            option->setEchoedTimestamp(tcpHeader->getAckBit() ? state->ts_recent : 0);

            EV_INFO << "Tcp Header Option TS(TSval=" << option->getSenderTimestamp() << ", TSecr=" << option->getEchoedTimestamp() << ") sent\n";
            tcpHeader->appendHeaderOption(option);
        }

        // SACK header option

        // RFC 2018, page 4:
        // "If sent at all, SACK options SHOULD be included in all ACKs which do
        // not ACK the highest sequence number in the data receiver's queue.  In
        // this situation the network has lost or mis-ordered data, such that
        // the receiver holds non-contiguous data in its queue.  RFC 1122,
        // Section 4.2.2.21, discusses the reasons for the receiver to send ACKs
        // in response to additional segments received in this state.  The
        // receiver SHOULD send an ACK for every valid segment that arrives
        // containing new data, and each of these "duplicate" ACKs SHOULD bear a
        // SACK option."
        if (state->sack_enabled && (state->snd_sack || state->snd_dsack)) {
            addSacks(tcpHeader);
        }

        // AccECN option (RFC 9768 section 3.2.3), after the SACK option, which leaves
        // room for the fields that changed (see addSacks())
        int accEcnOrder, accEcnWanted, accEcnRequired;
        if (tcpHeader->getAckBit() && getAccEcnOptionFields(accEcnOrder, accEcnWanted, accEcnRequired))
            appendAccEcnOption(tcpHeader, accEcnOrder, accEcnWanted, accEcnRequired);

        // TODO add new TCPOptions here once they are implemented
        // TODO delegate to TcpAlgorithm as well -- it may want to append additional options
    }

    if (tcpHeader->getHeaderOptionArraySize() != 0) {
        // RFC 9293 section 3.1: the header ends on a 32-bit boundary, and the padding
        // is zeros, that is, End of Option List bytes. An option area that the options
        // do not fill (for example MSS and SACK-Permitted without NOPs) gets them.
        while (tcpHeader->getHeaderOptionArrayLength().get() % 4 != 0)
            tcpHeader->appendHeaderOption(new TcpOptionEnd());

        B options_len = tcpHeader->getHeaderOptionArrayLength();

        if (options_len <= TCP_OPTIONS_MAX_SIZE) { // Options length allowed? - maximum: 40 Bytes
            tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + options_len);
            tcpHeader->setChunkLength(B(TCP_MIN_HEADER_LENGTH + options_len));
        }
        else {
            tcpHeader->dropHeaderOptions(); // drop all options
            tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH);
            tcpHeader->setChunkLength(TCP_MIN_HEADER_LENGTH);
            EV_ERROR << "ERROR: Options length exceeded! Segment will be sent without options" << "\n";
        }
    }

    return *tcpHeader;
}

uint32_t TcpConnection::getTSval(const Ptr<const TcpHeader>& tcpHeader) const
{
    for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_TIMESTAMP)
            return check_and_cast<const TcpOptionTimestamp *>(option)->getSenderTimestamp();
    }

    return 0;
}

const TcpOptionTimestamp *TcpConnection::findTSOption(const Ptr<const TcpHeader>& tcpHeader) const
{
    for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_TIMESTAMP)
            return check_and_cast<const TcpOptionTimestamp *>(option);
    }
    return nullptr;
}

uint32_t TcpConnection::getTSecr(const Ptr<const TcpHeader>& tcpHeader) const
{
    for (uint i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
        const TcpOption *option = tcpHeader->getHeaderOption(i);
        if (option->getKind() == TCPOPTION_TIMESTAMP)
            return check_and_cast<const TcpOptionTimestamp *>(option)->getEchoedTimestamp();
    }

    return 0;
}

void TcpConnection::updateSndbufLimitedChrono()
{
    // as Linux at the end of tcp_write_xmit(): the send buffer limits the sender when
    // the write queue has no unsent data, but the blocked writer waits for space
    if (state == nullptr || sendQueue == nullptr)
        return;
    bool limited = writerBlocked && sendQueue->getBytesAvailable(state->snd_nxt) == 0 && state->snd_una != state->snd_max;
    if (limited && state->sndbufLimitedStartTime < SIMTIME_ZERO)
        state->sndbufLimitedStartTime = simTime();
    else if (!limited && state->sndbufLimitedStartTime >= SIMTIME_ZERO) {
        state->sndbufLimitedAccumulated += simTime() - state->sndbufLimitedStartTime;
        state->sndbufLimitedStartTime = -1;
    }
}

uint32_t TcpConnection::linuxSkbTruesize(uint32_t payloadBytes)
{
    // Linux tun_get_user() and tun_alloc_skb(): a packet that does not fit one page with
    // the NET_SKB_PAD of 64 bytes gets a linear part of one page (SKB_MAX_HEAD: the page
    // less the pad and skb_shared_info), and the rest in pages of the largest order up
    // to 3 (PAGE_ALLOC_COSTLY_ORDER) that it fills (alloc_skb_with_frags())
    uint32_t packetBytes = payloadBytes + 40;
    uint32_t alloc;
    if (64 + packetBytes >= 4096) {
        uint32_t rest = packetBytes - (4096 - 64 - 320);
        uint32_t order = 3;
        alloc = 4096;
        while (rest > 0) {
            uint32_t aligned = (rest + 4095) / 4096 * 4096;
            while (order > 0 && aligned < (4096u << order))
                order--;
            alloc += 4096u << order;
            rest -= std::min(rest, 4096u << order);
        }
    }
    else {
        uint32_t base = packetBytes + 320;
        alloc = 1024; // a power-of-two allocation of the linear packet
        while (alloc < base)
            alloc <<= 1;
    }
    return alloc + 232;
}

void TcpConnection::chargeRcvBuffer(uint32_t payloadBytes)
{
    uint32_t truesize = linuxSkbTruesize(payloadBytes);
    rcvSkbChain.push_back(std::make_pair(payloadBytes, truesize));
    rcvBufOccupancy += truesize;
    // Linux tcp_measure_rcv_mss(): a segment at least as large as the estimate, but of
    // another length, measures the ratio; the estimate follows min(length, advmss)
    if (payloadBytes >= rcvMssEstimate) {
        if (payloadBytes != rcvMssEstimate) {
            uint64_t ratio = ((uint64_t)payloadBytes << 8) / truesize;
            rcvScalingRatio = (uint8_t)std::min<uint64_t>(ratio ? ratio : 1, 255);
        }
        rcvMssEstimate = std::min<uint32_t>(payloadBytes, state->advertisedMss > 0 ? (uint32_t)state->advertisedMss : payloadBytes);
    }
}

void TcpConnection::growRcvBufferForOutOfOrderData(uint32_t endSeq)
{
    if (seqLE(oooHighestEnd, state->rcv_nxt) || seqGreater(endSeq, oooHighestEnd))
        oooHighestEnd = endSeq;
    // windowAutoTuning, as Linux (tcp_rcvbuf_grow() at the end of tcp_data_queue_ofo()):
    // out-of-order data grow the buffer with the receive space as it is
    growRcvBuffer(rcvqSpace);
}

void TcpConnection::initRcvSpace()
{
    // Linux tcp_init_buffer_space(): the receive space starts at the smallest of the
    // offer, the window and the initial window of 10 MSS, and the measurement of DRS
    // starts now, at the first unread byte
    rcvqSpace = std::min<uint32_t>({state->rcv_ssthresh > 0 ? state->rcv_ssthresh : state->rcv_wnd, state->rcv_wnd, 10 * (uint32_t)state->advertisedMss});
    state->rcvqSpaceTime = simTime();
    state->rcvqSpaceSeq = receiveQueue->getFirstSeqNo();
}

void TcpConnection::adjustRcvSpace()
{
    // Linux tcp_rcv_space_adjust(), at each read: after one receive RTT, the bytes that
    // the application read in it, less the unread bytes, grow the receive space and the
    // buffer when they are more than the space; the measurement starts again
    if (state->rcvRttUs == 0)
        return;
    if ((simTime() - state->rcvqSpaceTime).inUnit(SIMTIME_US) < (int64_t)(state->rcvRttUs >> 3))
        return;
    uint32_t copiedSeq = receiveQueue->getFirstSeqNo();
    int64_t copied = (int64_t)(uint32_t)(copiedSeq - state->rcvqSpaceSeq) - (int64_t)(uint32_t)(state->rcv_nxt - copiedSeq);
    if (copied > (int64_t)rcvqSpace) {
        EV_DETAIL << "Dynamic Right-Sizing: " << copied << " bytes read in one receive RTT\n";
        growRcvBuffer((uint32_t)copied);
    }
    state->rcvqSpaceSeq = copiedSeq;
    state->rcvqSpaceTime = simTime();
}

void TcpConnection::growRcvBuffer(uint32_t newSpace)
{
    // Linux tcp_rcvbuf_grow(): the receive space becomes newSpace; the buffer of a
    // connection that the application accepted grows to the window of twice the space
    // and the out-of-order span, at the payload ratio (tcp_space_from_win()), and the
    // clamp of the offer follows; a buffer that SO_RCVBUF locked does not grow
    uint32_t oldSpace = rcvqSpace;
    rcvqSpace = newSpace;
    if (state->rcv_ssthresh == 0 || state->rcvbufLocked || !appOwned || isToBeAccepted())
        return;
    // DRS is one RTT late: the window is twice the space, and it grows by the window times
    // the receive RTT over receiveBufferLowRtt below that RTT, else by twice the window
    // times the relative growth of the space, so that the sender can double its rate
    uint64_t rcvwin = 2 * (uint64_t)newSpace;
    uint64_t rttUs = state->rcvRttUs >> 3;
    uint64_t grow;
    if (rttUs < state->rcvBufferLowRttUs)
        grow = rcvwin * rttUs / state->rcvBufferLowRttUs;
    else
        grow = oldSpace > 0 && newSpace > oldSpace ? ((rcvwin << 1) * (newSpace - oldSpace)) / oldSpace : 0;
    rcvwin += grow + (seqGreater(oooHighestEnd, state->rcv_nxt) ? oooHighestEnd - state->rcv_nxt : 0);
    // the buffer of that window, at most tcp_rmem[2] (receiveBufferMax)
    uint64_t rcvbuf = std::min<uint64_t>((rcvwin << 8) / rcvScalingRatio, state->rcvBufferMax);
    if (rcvbuf > state->rcvBufferSize) {
        state->rcvBufferSize = (uint32_t)std::min<uint64_t>(rcvbuf, UINT32_MAX);
        if (state->maxRcvBuffer < state->rcvBufferSize)
            state->maxRcvBuffer = state->rcvBufferSize;
        state->window_clamp = (uint32_t)(((uint64_t)state->rcvBufferSize * rcvScalingRatio) >> 8);
        EV_DETAIL << "Window auto-tuning: the receive buffer grows to " << state->rcvBufferSize << "\n";
    }
}

void TcpConnection::updateRcvRtt(uint32_t sampleUs, bool winDep)
{
    // Linux tcp_rcv_rtt_update(): the estimate is in microseconds times 8. A smaller
    // sample replaces it; without timestamps (winDep) the estimate is a minimum; with
    // them, a sample when the application read all data moves it by 1/8
    uint32_t oldSample = state->rcvRttUs;
    uint64_t m = (uint64_t)sampleUs << 3;
    uint32_t newSample;
    if (oldSample == 0 || m < oldSample)
        newSample = (uint32_t)std::min<uint64_t>(m, UINT32_MAX);
    else {
        if (winDep)
            return;
        if (state->rcv_nxt != receiveQueue->getFirstSeqNo()) // unread data
            return;
        newSample = oldSample - (oldSample >> 3) + sampleUs;
    }
    state->rcvRttUs = newSample;
}

void TcpConnection::measureRcvRtt()
{
    // Linux tcp_rcv_rtt_measure(), at each segment of in-order data: one sample for each
    // window, the time until rcv_nxt passes the right edge of the window at its start
    if (state->rcvRttTime != SIMTIME_ZERO) {
        if (seqLess(state->rcv_nxt, state->rcvRttSeq))
            return;
        int64_t deltaUs = (simTime() - state->rcvRttTime).inUnit(SIMTIME_US);
        updateRcvRtt(deltaUs > 0 ? (uint32_t)std::min<int64_t>(deltaUs, UINT32_MAX) : 1, true);
    }
    state->rcvRttSeq = state->rcv_nxt + state->rcv_wnd;
    state->rcvRttTime = simTime();
}

void TcpConnection::measureRcvRttTs(const Ptr<const TcpHeader>& tcpHeader, uint32_t payloadLength)
{
    // Linux tcp_rcv_rtt_measure_ts(), at each segment in ESTABLISHED: a new TSecr on a
    // segment of at least the received MSS gives a sample, now minus TSecr, in the
    // milliseconds of the timestamp clock; a delta of 0 gives none
    const TcpOptionTimestamp *tsOption = state->ts_enabled ? findTSOption(tcpHeader) : nullptr;
    if (tsOption == nullptr)
        return;
    uint32_t tsecr = tsOption->getEchoedTimestamp();
    if (tsecr == state->rcvRttLastTsecr)
        return;
    state->rcvRttLastTsecr = tsecr;
    if (payloadLength >= rcvMssEstimate) {
        uint32_t delta = convertSimtimeToTS(simTime()) - tsecr;
        if (delta > 0 && delta < INT32_MAX / 1000)
            updateRcvRtt(delta * 1000, false);
    }
}

void TcpConnection::releaseRcvBuffer(uint64_t readBytes)
{
    // Linux frees an skb, with all its truesize, only when the application read all of it
    while (readBytes > 0 && !rcvSkbChain.empty()) {
        auto& head = rcvSkbChain.front();
        if (readBytes >= head.first) {
            readBytes -= head.first;
            rcvBufOccupancy -= std::min<uint64_t>(head.second, rcvBufOccupancy);
            rcvSkbChain.pop_front();
        }
        else {
            head.first -= (uint32_t)readBytes;
            readBytes = 0;
        }
    }
}

void TcpConnection::updateRcvQueueVars()
{
    // update receive queue related state variables: the unread and the out-of-order
    // data use the receive buffer (receiveBufferSize, else the advertised window)
    uint32_t bufferSize = state->rcvBufferSize > 0 ? state->rcvBufferSize : state->maxRcvBuffer;
    state->freeRcvBuffer = receiveQueue->getAmountOfFreeBytes(bufferSize);
    state->usedRcvBuffer = bufferSize - state->freeRcvBuffer;

    // update receive queue related statistics
    emit(tcpRcvQueueBytesSignal, state->usedRcvBuffer);

//    tcpEV << "receiveQ: receiveQLength=" << receiveQueue->getQueueLength() << " maxRcvBuffer=" << state->maxRcvBuffer << " usedRcvBuffer=" << state->usedRcvBuffer << " freeRcvBuffer=" << state->freeRcvBuffer << "\n";
}

uint32_t TcpConnection::selectRcvWnd()
{
    // windowShrinkAllowed, as Linux with tcp_shrink_window=1 (__tcp_select_window()): the
    // offer is the free buffer at the payload ratio, rounded down to the window scale;
    // zero when it falls under 1/16 of the buffer (or under one MSS or one scale unit)
    // while less than half the buffer is free; and at most rcv_ssthresh, aligned up.
    if (state->windowShrinkAllowed) {
        uint32_t rcvbuf = state->rcvBufferSize > 0 ? state->rcvBufferSize : state->maxRcvBuffer;
        uint64_t freeSpace = rcvBufOccupancy < rcvbuf ? (((uint64_t)rcvbuf - rcvBufOccupancy) * rcvScalingRatio) >> 8 : 0;
        uint64_t fullSpace = ((uint64_t)rcvbuf * rcvScalingRatio) >> 8;
        uint32_t scaleUnit = 1u << state->rcv_wnd_scale;
        freeSpace = (freeSpace / scaleUnit) * scaleUnit;
        if (freeSpace < (fullSpace >> 1) && (freeSpace < (fullSpace >> 4) || freeSpace < state->snd_mss || freeSpace < scaleUnit))
            freeSpace = 0;
        if (state->rcv_ssthresh > 0 && freeSpace > state->rcv_ssthresh)
            freeSpace = ((uint64_t)(state->rcv_ssthresh + scaleUnit - 1) / scaleUnit) * scaleUnit;
        const uint32_t maxWin = (state->ws_enabled && state->rcv_wnd_scale) ? (TCP_MAX_WIN << state->rcv_wnd_scale) : TCP_MAX_WIN;
        return (uint32_t)std::min<uint64_t>(freeSpace, maxWin);
    }

    // the free receive buffer, as updateRcvQueueVars() keeps it in freeRcvBuffer
    uint32_t bufferSize = state->rcvBufferSize > 0 ? state->rcvBufferSize : state->maxRcvBuffer;
    uint32_t win = receiveQueue->getAmountOfFreeBytes(bufferSize);

    // With a receive buffer apart from the advertised window, the window also limits
    // the offer: the advertised window less the in-order data that the application did
    // not read. The out-of-order data uses only the buffer.
    // With windowAutoTuning, the window that auto-tuning reached takes the place of the
    // advertised window. A locked buffer counts the unread data below, at their
    // truesize; there, as Linux (__tcp_select_window()), they do not count against the
    // auto-tuned window too.
    if (state->rcvBufferSize > 0) {
        uint32_t window = state->rcv_ssthresh > 0 ? std::min(state->rcv_ssthresh, state->window_clamp) : state->maxRcvBuffer;
        uint32_t unread = state->rcv_ssthresh > 0 && state->rcvbufLocked ? 0 : receiveQueue->getAcknowledgedDataLength();
        win = std::min(win, window > unread ? window - unread : 0);
    }

    // a buffer that SO_RCVBUF locked does not grow, so the offer may promise only its
    // free space, charged at the truesize of the Linux skbs (tcp_space())
    uint32_t fullSpace = state->maxRcvBuffer;
    if (state->rcvbufLocked && state->rcvBufferSize > 0) {
        uint64_t freeSpace = rcvBufOccupancy < state->rcvBufferSize ? (((uint64_t)state->rcvBufferSize - rcvBufOccupancy) * rcvScalingRatio) >> 8 : 0;
        win = (uint32_t)std::min<uint64_t>(win, freeSpace);
        fullSpace = (uint32_t)std::min<uint64_t>(fullSpace, ((uint64_t)state->rcvBufferSize * rcvScalingRatio) >> 8);
    }

    // Following lines are based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, chapter 26.7, pages 878-879]:
    // Don't advertise less than one full-sized segment to avoid SWS
    // (the buffer is the whole space that the receiver can offer: with a locked buffer
    // smaller than one MSS, the empty buffer must still open the window)
    if (win < (fullSpace / 4) && win < state->snd_mss)
        win = 0;

    return win;
}

uint16_t TcpConnection::updateRcvWnd()
{
    // update receive queue related state variables and statistics
    updateRcvQueueVars();

    // A locked buffer dropped the last arrival: this ACK offers a zero window, as Linux
    // tcp_select_window() with ICSK_ACK_NOMEM, and the edge in force moves back to
    // RCV.NXT; the highest edge offered stays. The window shrinks (RFC 9293 SHLD-14),
    // which only a buffer that the application locked does.
    if (state->ackNomem) {
        state->ackNomem = false;
        state->rcv_wnd = 0;
        state->rcv_adv = state->rcv_nxt;
        emit(rcvAdvSignal, state->rcv_adv);
        emit(rcvWndSignal, state->rcv_wnd);
        return 0;
    }

    uint32_t win = selectRcvWnd();

    // windowShrinkAllowed: the right edge may move back; rcv_mwnd_seq keeps the largest
    // edge that the receiver offered
    if (state->windowShrinkAllowed) {
        state->rcv_adv = state->rcv_nxt + win;
        emit(rcvAdvSignal, state->rcv_adv);
        if (seqGreater(state->rcv_adv, state->rcv_mwnd_seq))
            state->rcv_mwnd_seq = state->rcv_adv;
        state->rcv_wnd = win;
        emit(rcvWndSignal, state->rcv_wnd);
        return (uint16_t)(state->ws_enabled ? state->rcv_wnd >> state->rcv_wnd_scale : state->rcv_wnd);
    }

    // Do not shrink window
    // (rcv_adv minus rcv_nxt) is the amount of space still available to the sender that was previously advertised
    // (rcv_nxt can be beyond rcv_adv after acceptBeyondBufferAtEmptyQueue)
    if (seqGreater(state->rcv_adv, state->rcv_nxt) && win < state->rcv_adv - state->rcv_nxt)
        win = state->rcv_adv - state->rcv_nxt;

    // Observe upper limit for advertised window on this connection
    const uint32_t maxWin = (state->ws_enabled && state->rcv_wnd_scale) ? (TCP_MAX_WIN << state->rcv_wnd_scale) : TCP_MAX_WIN; // TCP_MAX_WIN = 65535 (16 bit)
    if (win > maxWin)
        win = maxWin; // Note: The window size is limited to a 16 bit value in the TCP header if WINDOW SCALE option (RFC 7323 section 2) is not used

    // Note: The order of the "Do not shrink window" and "Observe upper limit" parts has been changed to the order used in FreeBSD Release 7.1

    // update rcv_adv if needed
    if (win > 0 && seqGE(state->rcv_nxt + win, state->rcv_adv)) {
        state->rcv_adv = state->rcv_nxt + win;

        emit(rcvAdvSignal, state->rcv_adv);
    }
    if (seqGreater(state->rcv_adv, state->rcv_mwnd_seq))
        state->rcv_mwnd_seq = state->rcv_adv;

    state->rcv_wnd = win;

    emit(rcvWndSignal, state->rcv_wnd);

    // scale rcv_wnd:
    uint32_t scaled_rcv_wnd = state->rcv_wnd;
    if (state->ws_enabled && state->rcv_wnd_scale) {
        ASSERT(state->rcv_wnd_scale <= 14); // RFC 7323 section 2.3: "the shift count MUST be limited to 14"
        scaled_rcv_wnd = scaled_rcv_wnd >> state->rcv_wnd_scale;
    }

    ASSERT(scaled_rcv_wnd == (uint16_t)scaled_rcv_wnd);

    return (uint16_t)scaled_rcv_wnd;
}

uint32_t TcpConnection::getFlightSize() const
{
    uint32_t outstanding = state->snd_max - state->snd_una;
    return outstanding > state->limitedTransmitBytes ? outstanding - state->limitedTransmitBytes : 0;
}

void TcpConnection::updateWndInfo(const Ptr<const TcpHeader>& tcpHeader, bool doAlways)
{
    uint32_t true_window = tcpHeader->getWindow();
    // RFC 7323 section 2.3:
    // "The window field (SEG.WND) in the header of every incoming
    // segment, with the exception of <SYN> segments, MUST be left-
    // shifted by Snd.Wind.Shift bits before updating SND.WND:
    //    SND.WND = SEG.WND << Snd.Wind.Shift"
    if (state->ws_enabled && !tcpHeader->getSynBit())
        true_window = tcpHeader->getWindow() << state->snd_wnd_scale;

    // Following lines are based on [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 982]:
    if (doAlways || (tcpHeader->getAckBit()
                     && (seqLess(state->snd_wl1, tcpHeader->getSequenceNo()) ||
                         (state->snd_wl1 == tcpHeader->getSequenceNo() && seqLE(state->snd_wl2, tcpHeader->getAckNo())) ||
                         (state->snd_wl2 == tcpHeader->getAckNo() && true_window > state->snd_wnd))))
    {
        // send window should be updated
        state->snd_wnd = true_window;
        if (state->snd_wnd > state->max_window)
            state->max_window = state->snd_wnd;
        EV_INFO << "Updating send window from segment: new wnd=" << state->snd_wnd << "\n";
        state->snd_wl1 = tcpHeader->getSequenceNo();
        state->snd_wl2 = tcpHeader->getAckNo();

        emit(sndWndSignal, state->snd_wnd);
    }
}

void TcpConnection::sendOneNewSegment(bool fullSegmentsOnly, uint32_t congestionWindow)
{
    ASSERT(state->limited_transmit_enabled);

    // RFC 3042, page 3:
    // "When a TCP sender has previously unsent data queued for transmission
    // it SHOULD use the Limited Transmit algorithm, which calls for a TCP
    // sender to transmit new data upon the arrival of the first two
    // consecutive duplicate ACKs when the following conditions are
    // satisfied:
    //
    //  * The receiver's advertised window allows the transmission of the
    //  segment.
    //
    //  * The amount of outstanding data would remain less than or equal
    //  to the congestion window plus 2 segments.  In other words, the
    //  sender can only send two segments beyond the congestion window
    //  (cwnd).
    //
    // The congestion window (cwnd) MUST NOT be changed when these new
    // segments are transmitted.  Assuming that these new segments and the
    // corresponding ACKs are not dropped, this procedure allows the sender
    // to infer loss using the standard Fast Retransmit threshold of three
    // duplicate ACKs [RFC2581].  This is more robust to reordered packets
    // than if an old packet were retransmitted on the first or second
    // duplicate ACK.
    //
    // Note: If the connection is using selective acknowledgments [RFC2018],
    // the data sender MUST NOT send new segments in response to duplicate
    // ACKs that contain no new SACK information, as a misbehaving receiver
    // can generate such ACKs to trigger inappropriate transmission of data
    // segments.  See [SCWA99] for a discussion of attacks by misbehaving
    // receivers."
    if (!state->sack_enabled || (state->sack_enabled && state->sackedBytes_old != state->sackedBytes)) {
        // check how many bytes we have
        uint32_t buffered = sendQueue->getBytesAvailable(state->snd_max);

        if (buffered >= state->snd_mss || (!fullSegmentsOnly && buffered > 0)) {
            uint32_t outstandingData = state->snd_max - state->snd_una;

            // check conditions from RFC 3042
            if (outstandingData + state->snd_mss <= state->snd_wnd &&
                outstandingData + state->snd_mss <= congestionWindow + 2 * state->snd_mss)
            {
                // RFC 3042, page 3: "(...)the sender can only send two segments beyond the congestion window (cwnd)."
                uint32_t effectiveWin = std::min(state->snd_wnd, congestionWindow) - outstandingData + 2 * state->snd_mss;

                // bytes: number of bytes we're allowed to send now
                uint32_t bytes = std::min(effectiveWin, state->snd_mss);

                if (bytes >= state->snd_mss || (!fullSegmentsOnly && bytes > 0)) {
                    uint32_t old_snd_nxt = state->snd_nxt;
                    // we'll start sending from snd_max
                    state->snd_nxt = state->snd_max;

                    EV_DETAIL << "Limited Transmit algorithm enabled. Sending one new segment.\n";
                    uint32_t sentBytes = sendSegment(bytes);

                    if (seqGreater(state->snd_nxt, state->snd_max))
                        state->snd_max = state->snd_nxt;

                    emit(unackedSignal, state->snd_max - state->snd_una);

                    // reset snd_nxt if needed
                    if (state->afterRto)
                        state->snd_nxt = old_snd_nxt + sentBytes;

                    // notify
                    tcpAlgorithm->ackSent();
                    tcpAlgorithm->dataSent(old_snd_nxt);
                }
            }
        }
    }
}

uint32_t TcpConnection::convertSimtimeToTS(simtime_t simtime)
{
    ASSERT(SimTime::getScaleExp() <= -3);

    uint32_t timestamp = (uint32_t)(simtime.inUnit(SIMTIME_MS));
    return timestamp;
}

simtime_t TcpConnection::convertTSToSimtime(uint32_t timestamp)
{
    ASSERT(SimTime::getScaleExp() <= -3);

    simtime_t simtime(timestamp, SIMTIME_MS);
    return simtime;
}

bool TcpConnection::isSendQueueEmpty()
{
    return sendQueue->getBytesAvailable(state->snd_nxt) == 0;
}

} // namespace tcp
} // namespace inet

