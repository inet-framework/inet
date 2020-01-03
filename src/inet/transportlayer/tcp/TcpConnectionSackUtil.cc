//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009-2011 Thomas Reschka
// Copyright (C) 2011 Zoltan Bojthe
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>    // min,max
#include <string.h>

#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

//
// helper functions for SACK
//

bool TcpConnection::processSACKOption(const Ptr<const TcpHeader>& tcpseg, const TcpOptionSack& option)
{
    if (option.getLength() % 8 != 2) {
        EV_ERROR << "ERROR: option length incorrect\n";
        return false;
    }

    uint n = option.getSackItemArraySize();
    ASSERT(option.getLength() == 2 + n*8);

    if (!state->sack_enabled) {
        EV_ERROR << "ERROR: " << n << " SACK(s) received, but sack_enabled is set to false\n";
        return false;
    }

    if (fsm.getState() != TCP_S_SYN_RCVD && fsm.getState() != TCP_S_ESTABLISHED
        && fsm.getState() != TCP_S_FIN_WAIT_1 && fsm.getState() != TCP_S_FIN_WAIT_2)
    {
        EV_ERROR << "ERROR: Tcp Header Option SACK received, but in unexpected state\n";
        return false;
    }

    if (n > 0) {    // sacks present?
        EV_INFO << n << " SACK(s) received:\n";
        for (uint i = 0; i < n; i++) {
            Sack tmp;
            tmp.setStart(option.getSackItem(i).getStart());
            tmp.setEnd(option.getSackItem(i).getEnd());

            EV_INFO << (i + 1) << ". SACK: " << tmp.str() << endl;

            // check for D-SACK
            if (i == 0 && seqLE(tmp.getEnd(), tcpseg->getAckNo())) {
                // RFC 2883, page 8:
                // "In order for the sender to check that the first (D)SACK block of an
                // acknowledgement in fact acknowledges duplicate data, the sender
                // should compare the sequence space in the first SACK block to the
                // cumulative ACK which is carried IN THE SAME PACKET.  If the SACK
                // sequence space is less than this cumulative ACK, it is an indication
                // that the segment identified by the SACK block has been received more
                // than once by the receiver.  An implementation MUST NOT compare the
                // sequence space in the SACK block to the TCP state variable snd.una
                // (which carries the total cumulative ACK), as this may result in the
                // wrong conclusion if ACK packets are reordered."
                EV_DETAIL << "Received D-SACK below cumulative ACK=" << tcpseg->getAckNo()
                          << " D-SACK: " << tmp.str() << endl;
                // Note: RFC 2883 does not specify what should be done in this case.
                // RFC 2883, page 9:
                // "5. Detection of Duplicate Packets
                // (...) This document does not specify what action a TCP implementation should
                // take in these cases. The extension to the SACK option simply enables
                // the sender to detect each of these cases.(...)"
            }
            else if (i == 0 && n > 1 && seqGreater(tmp.getEnd(), tcpseg->getAckNo())) {
                // RFC 2883, page 8:
                // "If the sequence space in the first SACK block is greater than the
                // cumulative ACK, then the sender next compares the sequence space in
                // the first SACK block with the sequence space in the second SACK
                // block, if there is one.  This comparison can determine if the first
                // SACK block is reporting duplicate data that lies above the cumulative
                // ACK."
                Sack tmp2(option.getSackItem(1).getStart(), option.getSackItem(1).getEnd());

                if (tmp2.contains(tmp)) {
                    EV_DETAIL << "Received D-SACK above cumulative ACK=" << tcpseg->getAckNo()
                              << " D-SACK: " << tmp.str()
                              << ", SACK: " << tmp2.str() << endl;
                    // Note: RFC 2883 does not specify what should be done in this case.
                    // RFC 2883, page 9:
                    // "5. Detection of Duplicate Packets
                    // (...) This document does not specify what action a TCP implementation should
                    // take in these cases. The extension to the SACK option simply enables
                    // the sender to detect each of these cases.(...)"
                }
            }

            if (seqGreater(tmp.getEnd(), tcpseg->getAckNo()) && seqGreater(tmp.getEnd(), state->snd_una))
                rexmitQueue->setSackedBit(tmp.getStart(), tmp.getEnd());
            else
                EV_DETAIL << "Received SACK below total cumulative ACK snd_una=" << state->snd_una << "\n";
        }
        state->rcv_sacks += n;    // total counter, no current number

        emit(rcvSacksSignal, state->rcv_sacks);

        // update scoreboard
        state->sackedBytes_old = state->sackedBytes;    // needed for RFC 3042 to check if last dupAck contained new sack information
        state->sackedBytes = rexmitQueue->getTotalAmountOfSackedBytes();

        emit(sackedBytesSignal, state->sackedBytes);
    }
    return true;
}

bool TcpConnection::isLost(uint32 seqNum)
{
    ASSERT(state->sack_enabled);

    // RFC 3517, page 3: "This routine returns whether the given sequence number is
    // considered to be lost.  The routine returns true when either
    // DupThresh discontiguous SACKed sequences have arrived above
    // 'SeqNum' or (DupThresh * SMSS) bytes with sequence numbers greater
    // than 'SeqNum' have been SACKed.  Otherwise, the routine returns
    // false."
    ASSERT(seqGE(seqNum, state->snd_una));    // HighAck = snd_una

    bool isLost = (rexmitQueue->getNumOfDiscontiguousSacks(seqNum) >= DUPTHRESH    // DUPTHRESH = 3
                   || rexmitQueue->getAmountOfSackedBytes(seqNum) >= (DUPTHRESH * state->snd_mss));

    return isLost;
}

void TcpConnection::setPipe()
{
    ASSERT(state->sack_enabled);

    // RFC 3517, pages 1 and 2: "
    // "HighACK" is the sequence number of the highest byte of data that
    // has been cumulatively ACKed at a given point.
    //
    // "HighData" is the highest sequence number transmitted at a given
    // point.
    //
    // "HighRxt" is the highest sequence number which has been
    // retransmitted during the current loss recovery phase.
    //
    // "Pipe" is a sender's estimate of the number of bytes outstanding
    // in the network.  This is used during recovery for limiting the
    // sender's sending rate.  The pipe variable allows TCP to use a
    // fundamentally different congestion control than specified in
    // [RFC2581].  The algorithm is often referred to as the "pipe
    // algorithm"."
    // HighAck = snd_una
    // HighData = snd_max

    state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
    state->pipe = 0;
    uint32 length = 0;    // required for rexmitQueue->checkSackBlock()
    bool sacked;    // required for rexmitQueue->checkSackBlock()
    bool rexmitted;    // required for rexmitQueue->checkSackBlock()

    // RFC 3517, page 3: "This routine traverses the sequence space from HighACK to HighData
    // and MUST set the "pipe" variable to an estimate of the number of
    // octets that are currently in transit between the TCP sender and
    // the TCP receiver.  After initializing pipe to zero the following
    // steps are taken for each octet 'S1' in the sequence space between
    // HighACK and HighData that has not been SACKed:"
    for (uint32 s1 = state->snd_una; seqLess(s1, state->snd_max); s1 += length) {
        rexmitQueue->checkSackBlock(s1, length, sacked, rexmitted);

        if (!sacked) {
            // RFC 3517, page 3: "(a) If IsLost (S1) returns false:
            //
            //     Pipe is incremented by 1 octet.
            //
            //     The effect of this condition is that pipe is incremented for
            //     packets that have not been SACKed and have not been determined
            //     to have been lost (i.e., those segments that are still assumed
            //     to be in the network)."
            if (isLost(s1) == false)
                state->pipe += length;

            // RFC 3517, pages 3 and 4: "(b) If S1 <= HighRxt:
            //
            //     Pipe is incremented by 1 octet.
            //
            //     The effect of this condition is that pipe is incremented for
            //     the retransmission of the octet.
            //
            //  Note that octets retransmitted without being considered lost are
            //  counted twice by the above mechanism."
            if (seqLess(s1, state->highRxt))
                state->pipe += length;
        }
    }

    emit(pipeSignal, state->pipe);
}

bool TcpConnection::nextSeg(uint32& seqNum)
{
    ASSERT(state->sack_enabled);

    // RFC 3517, page 5: "This routine uses the scoreboard data structure maintained by the
    // Update() function to determine what to transmit based on the SACK
    // information that has arrived from the data receiver (and hence
    // been marked in the scoreboard).  NextSeg () MUST return the
    // sequence number range of the next segment that is to be
    // transmitted, per the following rules:"

    state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
    uint32 highestSackedSeqNum = rexmitQueue->getHighestSackedSeqNum();
    uint32 shift = state->snd_mss;
    bool sacked = false;    // required for rexmitQueue->checkSackBlock()
    bool rexmitted = false;    // required for rexmitQueue->checkSackBlock()

    seqNum = 0;

    if (state->ts_enabled)
        shift -= B(TCP_OPTION_TS_SIZE).get();

    // RFC 3517, page 5: "(1) If there exists a smallest unSACKed sequence number 'S2' that
    // meets the following three criteria for determining loss, the
    // sequence range of one segment of up to SMSS octets starting
    // with S2 MUST be returned.
    //
    // (1.a) S2 is greater than HighRxt.
    //
    // (1.b) S2 is less than the highest octet covered by any
    //       received SACK.
    //
    // (1.c) IsLost (S2) returns true."

    // Note: state->highRxt == RFC.HighRxt + 1
    for (uint32 s2 = state->highRxt;
         seqLess(s2, state->snd_max) && seqLess(s2, highestSackedSeqNum);
         s2 += shift
         )
    {
        rexmitQueue->checkSackBlock(s2, shift, sacked, rexmitted);

        if (!sacked) {
            if (isLost(s2)) {    // 1.a and 1.b are true, see above "for" statement
                seqNum = s2;

                return true;
            }

            break;    // !isLost(x) --> !isLost(x + d)
        }
    }

    // RFC 3517, page 5: "(2) If no sequence number 'S2' per rule (1) exists but there
    // exists available unsent data and the receiver's advertised
    // window allows, the sequence range of one segment of up to SMSS
    // octets of previously unsent data starting with sequence number
    // HighData+1 MUST be returned."
    {
        // check how many unsent bytes we have
        ulong buffered = sendQueue->getBytesAvailable(state->snd_max);
        ulong maxWindow = state->snd_wnd;
        // effectiveWindow: number of bytes we're allowed to send now
        ulong effectiveWin = maxWindow - state->pipe;

        if (buffered > 0 && effectiveWin >= state->snd_mss) {
            seqNum = state->snd_max;    // HighData = snd_max

            return true;
        }
    }

    // RFC 3517, pages 5 and 6: "(3) If the conditions for rules (1) and (2) fail, but there exists
    // an unSACKed sequence number 'S3' that meets the criteria for
    // detecting loss given in steps (1.a) and (1.b) above
    // (specifically excluding step (1.c)) then one segment of up to
    // SMSS octets starting with S3 MAY be returned.
    //
    // Note that rule (3) is a sort of retransmission "last resort".
    // It allows for retransmission of sequence numbers even when the
    // sender has less certainty a segment has been lost than as with
    // rule (1).  Retransmitting segments via rule (3) will help
    // sustain TCP's ACK clock and therefore can potentially help
    // avoid retransmission timeouts.  However, in sending these
    // segments the sender has two copies of the same data considered
    // to be in the network (and also in the Pipe estimate).  When an
    // ACK or SACK arrives covering this retransmitted segment, the
    // sender cannot be sure exactly how much data left the network
    // (one of the two transmissions of the packet or both
    // transmissions of the packet).  Therefore the sender may
    // underestimate Pipe by considering both segments to have left
    // the network when it is possible that only one of the two has.
    //
    // We believe that the triggering of rule (3) will be rare and
    // that the implications are likely limited to corner cases
    // relative to the entire recovery algorithm.  Therefore we leave
    // the decision of whether or not to use rule (3) to
    // implementors."
    {
        for (uint32 s3 = state->highRxt;
             seqLess(s3, state->snd_max) && seqLess(s3, highestSackedSeqNum);
             s3 += shift
             )
        {
            rexmitQueue->checkSackBlock(s3, shift, sacked, rexmitted);

            if (!sacked) {
                // 1.a and 1.b are true, see above "for" statement
                seqNum = s3;

                return true;
            }
        }
    }

    // RFC 3517, page 6: "(4) If the conditions for each of (1), (2), and (3) are not met,
    // then NextSeg () MUST indicate failure, and no segment is
    // returned."
    seqNum = 0;

    return false;
}

void TcpConnection::sendDataDuringLossRecoveryPhase(uint32 congestionWindow)
{
    ASSERT(state->sack_enabled && state->lossRecovery);

    // RFC 3517 pages 7 and 8: "(5) In order to take advantage of potential additional available
    // cwnd, proceed to step (C) below.
    // (...)
    // (C) If cwnd - pipe >= 1 SMSS the sender SHOULD transmit one or more
    // segments as follows:
    // (...)
    // (C.5) If cwnd - pipe >= 1 SMSS, return to (C.1)"
    while (((int)congestionWindow - (int)state->pipe) >= (int)state->snd_mss) {    // Note: Typecast needed to avoid prohibited transmissions
        // RFC 3517 pages 7 and 8: "(C.1) The scoreboard MUST be queried via NextSeg () for the
        // sequence number range of the next segment to transmit (if any),
        // and the given segment sent.  If NextSeg () returns failure (no
        // data to send) return without sending anything (i.e., terminate
        // steps C.1 -- C.5)."

        uint32 seqNum;

        if (!nextSeg(seqNum)) // if nextSeg() returns false (=failure): terminate steps C.1 -- C.5
            break;

        sendSegmentDuringLossRecoveryPhase(seqNum);
        // RFC 3517 page 8: "(C.4) The estimate of the amount of data outstanding in the
        // network must be updated by incrementing pipe by the number of
        // octets transmitted in (C.1)."
        state->pipe += state->sentBytes;
    }
}

void TcpConnection::sendSegmentDuringLossRecoveryPhase(uint32 seqNum)
{
    ASSERT(state->sack_enabled && state->lossRecovery);

    // start sending from seqNum
    state->snd_nxt = seqNum;

    uint32 old_highRxt = rexmitQueue->getHighestRexmittedSeqNum();

    // no need to check cwnd and rwnd - has already be done before
    // no need to check nagle - sending mss bytes
    sendSegment(state->snd_mss);

    uint32 sentSeqNum = seqNum + state->sentBytes;

    if(state->send_fin && sentSeqNum == state->snd_fin_seq)
        sentSeqNum = sentSeqNum + 1;

    ASSERT(seqLE(state->snd_nxt, sentSeqNum));

    // RFC 3517 page 8: "(C.2) If any of the data octets sent in (C.1) are below HighData,
    // HighRxt MUST be set to the highest sequence number of the
    // retransmitted segment."
    if (seqLess(seqNum, state->snd_max)) {    // HighData = snd_max
        state->highRxt = rexmitQueue->getHighestRexmittedSeqNum();
    }

    // RFC 3517 page 8: "(C.3) If any of the data octets sent in (C.1) are above HighData,
    // HighData must be updated to reflect the transmission of
    // previously unsent data."
    if (seqGreater(sentSeqNum, state->snd_max)) // HighData = snd_max
        state->snd_max = sentSeqNum;

    emit(unackedSignal, state->snd_max - state->snd_una);

    // RFC 3517, page 9: "6   Managing the RTO Timer
    //
    // The standard TCP RTO estimator is defined in [RFC2988].  Due to the
    // fact that the SACK algorithm in this document can have an impact on
    // the behavior of the estimator, implementers may wish to consider how
    // the timer is managed.  [RFC2988] calls for the RTO timer to be
    // re-armed each time an ACK arrives that advances the cumulative ACK
    // point.  Because the algorithm presented in this document can keep the
    // ACK clock going through a fairly significant loss event,
    // (comparatively longer than the algorithm described in [RFC2581]), on
    // some networks the loss event could last longer than the RTO.  In this
    // case the RTO timer would expire prematurely and a segment that need
    // not be retransmitted would be resent.
    //
    // Therefore we give implementers the latitude to use the standard
    // [RFC2988] style RTO management or, optionally, a more careful variant
    // that re-arms the RTO timer on each retransmission that is sent during
    // recovery MAY be used.  This provides a more conservative timer than
    // specified in [RFC2988], and so may not always be an attractive
    // alternative.  However, in some cases it may prevent needless
    // retransmissions, go-back-N transmission and further reduction of the
    // congestion window."
    tcpAlgorithm->ackSent();

    if (old_highRxt != state->highRxt) {
        // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
        EV_INFO << "Retransmission sent during recovery, restarting REXMIT timer.\n";
        tcpAlgorithm->restartRexmitTimer();
    }
    else // don't measure RTT for retransmitted packets
        tcpAlgorithm->dataSent(seqNum); // seqNum = old_snd_nxt
}

TcpHeader TcpConnection::addSacks(const Ptr<TcpHeader>& tcpseg)
{
    B options_len = B(0);
    B used_options_len = tcpseg->getHeaderOptionArrayLength();
    bool dsack_inserted = false;    // set if dsack is subsets of a bigger sack block recently reported

    uint32 start = state->start_seqno;
    uint32 end = state->end_seqno;

    // delete old sacks (below rcv_nxt), delete duplicates and print previous status of sacks_array:
    auto it = state->sacks_array.begin();
    EV_INFO << "Previous status of sacks_array: \n" << ((it != state->sacks_array.end()) ? "" : "\t EMPTY\n");

    while (it != state->sacks_array.end()) {
        if (seqLE(it->getEnd(), state->rcv_nxt) || it->empty()) {
            EV_DETAIL << "\t SACK in sacks_array: " << " " << it->str() << " delete now\n";
            it = state->sacks_array.erase(it);
        }
        else {
            EV_DETAIL << "\t SACK in sacks_array: " << " " << it->str() << endl;

            ASSERT(seqGE(it->getStart(), state->rcv_nxt));

            it++;
        }
    }

    if (used_options_len > TCP_OPTIONS_MAX_SIZE - TCP_OPTION_SACK_MIN_SIZE) {
        EV_ERROR << "ERROR: Failed to addSacks - at least 10 free bytes needed for SACK - used_options_len=" << used_options_len << endl;

        //reset flags:
        state->snd_sack = false;
        state->snd_dsack = false;
        state->start_seqno = 0;
        state->end_seqno = 0;
        return *tcpseg;
    }

    if (start != end) {
        if (state->snd_dsack) {    // SequenceNo < rcv_nxt
            // RFC 2883, page 3:
            // "(3) The left edge of the D-SACK block specifies the first sequence
            // number of the duplicate contiguous sequence, and the right edge of
            // the D-SACK block specifies the sequence number immediately following
            // the last sequence in the duplicate contiguous sequence."
            if (seqLess(start, state->rcv_nxt) && seqLess(state->rcv_nxt, end))
                end = state->rcv_nxt;

            dsack_inserted = true;
            Sack nSack(start, end);
            state->sacks_array.push_front(nSack);
            EV_DETAIL << "inserted DSACK entry: " << nSack.str() << "\n";
        }
        else {
            uint32 contStart = receiveQueue->getLE(start);
            uint32 contEnd = receiveQueue->getRE(end);

            Sack newSack(contStart, contEnd);
            state->sacks_array.push_front(newSack);
            EV_DETAIL << "Inserted SACK entry: " << newSack.str() << "\n";
        }

        // RFC 2883, page 3:
        // "(3) The left edge of the D-SACK block specifies the first sequence
        // number of the duplicate contiguous sequence, and the right edge of
        // the D-SACK block specifies the sequence number immediately following
        // the last sequence in the duplicate contiguous sequence."

        // RFC 2018, page 4:
        // "* The first SACK block (i.e., the one immediately following the
        // kind and length fields in the option) MUST specify the contiguous
        // block of data containing the segment which triggered this ACK,
        // unless that segment advanced the Acknowledgment Number field in
        // the header.  This assures that the ACK with the SACK option
        // reflects the most recent change in the data receiver's buffer
        // queue."

        // RFC 2018, page 4:
        // "* The first SACK block (i.e., the one immediately following the
        // kind and length fields in the option) MUST specify the contiguous
        // block of data containing the segment which triggered this ACK,"

        // RFC 2883, page 3:
        // "(4) If the D-SACK block reports a duplicate contiguous sequence from
        // a (possibly larger) block of data in the receiver's data queue above
        // the cumulative acknowledgement, then the second SACK block in that
        // SACK option should specify that (possibly larger) block of data.
        //
        // (5) Following the SACK blocks described above for reporting duplicate
        // segments, additional SACK blocks can be used for reporting additional
        // blocks of data, as specified in RFC 2018."

        // RFC 2018, page 4:
        // "* The SACK option SHOULD be filled out by repeating the most
        // recently reported SACK blocks (based on first SACK blocks in
        // previous SACK options) that are not subsets of a SACK block
        // already included in the SACK option being constructed."

        it = state->sacks_array.begin();
        if (dsack_inserted)
            it++;

        for ( ; it != state->sacks_array.end(); it++) {
            ASSERT(!it->empty());

            auto it2 = it;
            it2++;
            while (it2 != state->sacks_array.end()) {
                if (it->contains(*it2)) {
                    EV_DETAIL << "sack matched, delete contained : a=" << it->str() << ", b=" << it2->str() << endl;
                    it2 = state->sacks_array.erase(it2);
                }
                else
                    it2++;
            }
        }
    }

    uint n = state->sacks_array.size();

    uint maxnode = ((B(TCP_OPTIONS_MAX_SIZE - used_options_len).get()) - 2) / 8;    // 2: option header, 8: size of one sack entry

    if (n > maxnode)
        n = maxnode;

    if (n == 0) {
        if (dsack_inserted)
            state->sacks_array.pop_front(); // delete DSACK entry

        // reset flags:
        state->snd_sack = false;
        state->snd_dsack = false;
        state->start_seqno = 0;
        state->end_seqno = 0;

        return *tcpseg;
    }

    uint optArrSize = tcpseg->getHeaderOptionArraySize();

    uint optArrSizeAligned = optArrSize;

    while (B(used_options_len).get() % 4 != 2) {
        used_options_len++;
        optArrSizeAligned++;
    }

    while (optArrSize < optArrSizeAligned) {
        tcpseg->insertHeaderOption(new TcpOptionNop());
        optArrSize++;
    }

    ASSERT(B(used_options_len).get() % 4 == 2);

    TcpOptionSack *option = new TcpOptionSack();
    option->setLength(8 * n + 2);
    option->setSackItemArraySize(n);

    // write sacks from sacks_array to options
    uint counter = 0;

    for (it = state->sacks_array.begin(); it != state->sacks_array.end() && counter < n; it++) {
        ASSERT(it->getStart() != it->getEnd());
        option->setSackItem(counter++, *it);
    }

    // independent of "n" we always need 2 padding bytes (NOP) to make: (used_options_len % 4 == 0)
    options_len = used_options_len + TCP_OPTION_SACK_ENTRY_SIZE * n + TCP_OPTION_HEAD_SIZE;    // 8 bytes for each SACK (n) + 2 bytes for kind&length

    ASSERT(options_len <= TCP_OPTIONS_MAX_SIZE);    // Options length allowed? - maximum: 40 Bytes

    tcpseg->insertHeaderOption(option);
    tcpseg->setHeaderLength(TCP_MIN_HEADER_LENGTH + tcpseg->getHeaderOptionArrayLength());
    tcpseg->setChunkLength(tcpseg->getHeaderLength());
    // update number of sent sacks
    state->snd_sacks += n;

    emit(sndSacksSignal, state->snd_sacks);

    EV_INFO << n << " SACK(s) added to header:\n";

    for (uint t = 0; t < n; t++) {
        EV_INFO << t << ". SACK:" << " [" << option->getSackItem(t).getStart() << ".." << option->getSackItem(t).getEnd() << ")";

        if (t == 0) {
            if (state->snd_dsack)
                EV_INFO << " (D-SACK)";
            else if (seqLE(option->getSackItem(t).getEnd(), state->rcv_nxt)) {
                EV_INFO << " (received segment filled out a gap)";
                state->snd_dsack = true;    // Note: Set snd_dsack to delete first sack from sacks_array
            }
        }

        EV_INFO << endl;
    }

    // RFC 2883, page 3:
    // "(1) A D-SACK block is only used to report a duplicate contiguous
    // sequence of data received by the receiver in the most recent packet.
    //
    // (2) Each duplicate contiguous sequence of data received is reported
    // in at most one D-SACK block.  (I.e., the receiver sends two identical
    // D-SACK blocks in subsequent packets only if the receiver receives two
    // duplicate segments.)//
    //
    // In case of d-sack: delete first sack (d-sack) and move old sacks by one to the left
    if (dsack_inserted)
        state->sacks_array.pop_front(); // delete DSACK entry

    // reset flags:
    state->snd_sack = false;
    state->snd_dsack = false;
    state->start_seqno = 0;
    state->end_seqno = 0;

    return *tcpseg;
}

} // namespace tcp
} // namespace inet

