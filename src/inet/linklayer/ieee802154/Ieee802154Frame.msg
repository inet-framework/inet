// SPDX-License-Identifier: LGPL-3.0-or-later
//
import inet.common.packet.chunk.Chunk;
import inet.linklayer.ieee802154.Ieee802154Address;
namespace inet;

// Complete selected O-QPSK MPDU. No upper protocol number is carried on wire.
class Ieee802154Frame extends FieldsChunk
{
    chunkLength = B(5);
    int frameType = 1; // 0 Beacon; 1 Data; 2 immediate ACK; 3 Command.
    int frameVersion = 1;
    bool ackRequest = false;
    bool securityEnabled = false; // Protected remainder is opaque; no security transformation is implied.
    bool reservedBit7 = false; // Retained for forensic decoding; ignored on reception.
    bool framePending = false;
    bool panCompression = false;
    uint8_t dsn = 0;
    Ieee802154Address destinationAddress;
    Ieee802154Address sourceAddress;
    uint16_t destinationPanId = 65535;
    uint16_t sourcePanId = 65535;
    uint8_t payload[];
    bool fcsPresent = true;
    bool computeFcs = true;
    uint16_t fcs = 0;
    bool fcsValid = false; // Meaningful on decoded frames with FCS present.
    int decodeStatus = 0; // Local codec status, never serialized.
    uint8_t rawBytes[]; // Preserved unsupported/malformed input only.
}
