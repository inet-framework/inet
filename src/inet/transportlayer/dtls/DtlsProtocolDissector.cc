// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsProtocolDissector.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/dtls/Dtls.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/Dtls12CidHeader_m.h"

namespace inet::dtls {

Register_Protocol_Dissector(&dtlsProtocol, DtlsProtocolDissector);

void DtlsProtocolDissector::dissect(Packet *packet, const Protocol *, ICallback& callback) const
{
    callback.startProtocolDataUnit(&dtlsProtocol);
    while (packet->getDataLength() > b(0)) {
        try {
            auto first = packet->peekAtFront<BytesChunk>(B(1))->getByte(0);
            b bodyLength;
            if (first == 20 || first == 21 || first == 22 || first == 23 || first == 24 || first == 26) {
                if (packet->getDataLength() < B(13))
                    throw cRuntimeError("Truncated DTLS plaintext header");
                auto header = packet->popAtFront<DtlsPlaintextHeader>(B(13));
                callback.visitChunk(header, &dtlsProtocol);
                bodyLength = B(header->getLength());
            }
            else if (first == 25) {
                auto size = B(21);
                auto front = packet->peekData();
                if (auto sequence = dynamicPtrCast<const SequenceChunk>(front))
                    front = sequence->getChunks().front();
                if (auto native = dynamicPtrCast<const Dtls12CidHeader>(front))
                    size = B(native->getChunkLength());
                if (packet->getDataLength() < size)
                    throw cRuntimeError("Truncated DTLS 1.2 CID header");
                auto header = packet->popAtFront<Dtls12CidHeader>(size);
                callback.visitChunk(header, &dtlsProtocol);
                bodyLength = B(header->getLength());
            }
            else if ((first & 0xe0) == 0x20) {
                auto size = B(1 + ((first & 0x10) ? 8 : 0) + ((first & 8) ? 2 : 1) + ((first & 4) ? 2 : 0));
                auto front = packet->peekData();
                if (auto sequence = dynamicPtrCast<const SequenceChunk>(front))
                    front = sequence->getChunks().front();
                // Retained outgoing headers also describe a peer's different
                // CID length. Raw incoming bytes use the local eight-byte profile.
                if (auto native = dynamicPtrCast<const DtlsCiphertextHeader>(front))
                    size = B(native->getChunkLength());
                if (packet->getDataLength() < size)
                    throw cRuntimeError("Truncated DTLS unified header");
                auto header = packet->popAtFront<DtlsCiphertextHeader>(size);
                callback.visitChunk(header, &dtlsProtocol);
                bodyLength = header->getLengthPresent() ? b(B(header->getLength())) : packet->getDataLength();
            }
            else
                throw cRuntimeError("Unknown DTLS record header");
            if (bodyLength > packet->getDataLength())
                throw cRuntimeError("Truncated DTLS record");
            if (bodyLength > b(0))
                callback.visitChunk(packet->popAtFront(bodyLength), &dtlsProtocol);
        }
        catch (const cRuntimeError&) {
            callback.markIncorrect();
            if (packet->getDataLength() > b(0))
                callback.visitChunk(packet->popAtFront(packet->getDataLength()), &dtlsProtocol);
            break;
        }
    }
    callback.endProtocolDataUnit(&dtlsProtocol);
}

} // namespace inet::dtls
