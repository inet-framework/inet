//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2011 Thomas Reschka
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <string.h>

#include <algorithm> // min,max

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
// the SACK option of the data receiver (RFC 2018); the sender's SACK loss recovery
// is in Rfc6675Recovery
//

TcpHeader TcpConnection::addSacks(const Ptr<TcpHeader>& tcpHeader)
{
    B options_len = B(0);
    B used_options_len = tcpHeader->getHeaderOptionArrayLength();
    bool dsack_inserted = false; // set if dsack is subsets of a bigger sack block recently reported

    uint32_t start = state->start_seqno;
    uint32_t end = state->end_seqno;

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

        // reset flags:
        state->snd_sack = false;
        state->snd_dsack = false;
        state->start_seqno = 0;
        state->end_seqno = 0;
        return *tcpHeader;
    }

    if (start != end) {
        if (state->dsack_enabled && state->snd_dsack) { // SequenceNo < rcv_nxt
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
        // RFC 2018 section 4: the first block names the segment that triggered
        // this ACK "unless that segment advanced the Acknowledgment Number
        // field". A block below rcv_nxt would read as a D-SACK at the sender.
        else if (seqGreater(end, state->rcv_nxt)) {
            uint32_t contStart = receiveQueue->getLE(start);
            uint32_t contEnd = receiveQueue->getRE(end);

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

        for (; it != state->sacks_array.end(); it++) {
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

    uint maxnode = (((TCP_OPTIONS_MAX_SIZE - used_options_len).get<B>()) - 2) / 8; // 2: option header, 8: size of one sack entry

    if (n > maxnode)
        n = maxnode;

    // AccECN (RFC 9768 section 3.2.3.3): implementations "SHOULD generally prioritize"
    // the AccECN option "over other TCP Options", but if it "would leave insufficient
    // space for two SACK blocks ..., the Data Receiver MUST give precedence to the SACK
    // option". As Linux (tcp_options_fit_accecn()), fewer SACK blocks, but at least
    // two, make room for the fields that changed since the last option; if even two
    // blocks leave no room, all blocks stay, and the option is omitted.
    int accEcnOrder, accEcnWanted, accEcnRequired;
    if (n > 2 && getAccEcnOptionFields(accEcnOrder, accEcnWanted, accEcnRequired) && accEcnRequired > 0) {
        // the end of a SACK option of k blocks, in the layout that the fit uses
        uint sackStart = getAccEcnOptionSpaceUsed(tcpHeader);
        if (!tcpMain->accEcnOptionLinux)
            while (sackStart % 4 != 2)
                sackStart++;
        auto sackEnd = [&] (uint k) { return sackStart + (tcpMain->accEcnOptionLinux ? 4 : 2) + 8 * k; };
        uint k = n;
        while (k > 2 && !accEcnOptionFits(sackEnd(k), accEcnRequired, true))
            k--;
        if (accEcnOptionFits(sackEnd(k), accEcnRequired, true))
            n = k;
    }

    if (n == 0) {
        if (dsack_inserted)
            state->sacks_array.pop_front(); // delete DSACK entry

        // reset flags:
        state->snd_sack = false;
        state->snd_dsack = false;
        state->start_seqno = 0;
        state->end_seqno = 0;

        return *tcpHeader;
    }

    // alignOptions: NOPs in front of the SACK option align its blocks to 32 bits;
    // without it, the EOL padding at the end of the options fills the area
    if (tcpMain->alignOptions) {
        uint optArrSize = tcpHeader->getHeaderOptionArraySize();

        uint optArrSizeAligned = optArrSize;

        while (used_options_len.get<B>() % 4 != 2) {
            used_options_len++;
            optArrSizeAligned++;
        }

        while (optArrSize < optArrSizeAligned) {
            tcpHeader->appendHeaderOption(new TcpOptionNop());
            optArrSize++;
        }

        ASSERT(used_options_len.get<B>() % 4 == 2);
    }

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
    options_len = used_options_len + TCP_OPTION_SACK_ENTRY_SIZE * n + TCP_OPTION_HEAD_SIZE; // 8 bytes for each SACK (n) + 2 bytes for kind&length

    ASSERT(options_len <= TCP_OPTIONS_MAX_SIZE); // Options length allowed? - maximum: 40 Bytes

    tcpHeader->appendHeaderOption(option);
    tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + tcpHeader->getHeaderOptionArrayLength());
    tcpHeader->setChunkLength(tcpHeader->getHeaderLength());
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
                state->snd_dsack = true; // Note: Set snd_dsack to delete first sack from sacks_array
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

    return *tcpHeader;
}

} // namespace tcp
} // namespace inet

