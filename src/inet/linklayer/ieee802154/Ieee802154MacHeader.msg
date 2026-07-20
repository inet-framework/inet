//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//**************************************************************************
// * file:        Ieee802154MacHeader.msg
// *
// * author:      Daniel Willkomm
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// **************************************************************************
// * part of:     framework implementation developed by tkn
// * description: - base class for MAC layer packets
// *              - subclass to create your own MAC layer packets
// **************************************************************************/

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace inet;

//
// A CSMA mac header format definition
//
// @author Daniel Willkomm
//
// TODO: This is a simplified, INET-internal representation, NOT the real
// IEEE 802.15.4 MAC header. Ieee802154MacHeaderSerializer emits a single fixed
// format (Frame Control hard-coded to 0xCC01: a data frame with 64-bit extended
// addressing for both source and destination) and misuses the Source PAN ID
// field to carry networkProtocol, because this model has no way to indicate the
// payload protocol otherwise. Real captures cannot be represented or round-tripped:
//   - The Frame Control field is fixed, so beacon/ack/MAC-command frames and any
//     other frame type are rejected (the deserializer asserts frameControl == 0xCC01).
//   - Only 64-bit extended addressing is written; the short (16-bit) and
//     "no address" addressing modes, the per-mode PAN ID / PAN ID Compression
//     rules, and the variable header length they imply are not modelled.
//   - srcAddr/destAddr are 48-bit MacAddress values (padded to 8 octets), not the
//     64-bit EUI-64 extended addresses of real 802.15.4.
//   - The auxiliary security header (present when the Security Enabled bit is set)
//     and the Frame Version field are absent.
//   - The FCS is a 2-octet CRC (DLT_IEEE802_15_4_WITHFCS), not the 4-octet FCS this
//     model/serializer assumes.
// Making real 802.15.4 frames round-trip requires modelling the actual on-the-wire
// header (full Frame Control parsing, variable addressing, aux security header),
// which is a change to the 802.15.4 simulation model with its own fingerprint
// implications -- a separate task, out of scope of the serializer round-trip work.
//
class Ieee802154MacHeader extends FieldsChunk
{
    MacAddress srcAddr;
    MacAddress destAddr;
    int networkProtocol = -1;
    long       sequenceId; // Sequence Number to detect duplicate messages
}
