// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsProtocolDissector.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/dtls/Dtls.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

Register_Protocol_Dissector(&dtlsProtocol, DtlsProtocolDissector);

void DtlsProtocolDissector::dissect(Packet *packet, const Protocol *, ICallback& callback) const
{
    callback.startProtocolDataUnit(&dtlsProtocol);
    while (packet->getDataLength() > b(0)) {
        try {
            auto first = packet->peekAtFront<BytesChunk>(B(1))->getByte(0);
            b bodyLength;
            if (first == 21 || first == 22 || first == 26) {
                if (packet->getDataLength() < B(13))
                    throw cRuntimeError("Truncated DTLS plaintext header");
                auto header = packet->popAtFront<DtlsPlaintextHeader>(B(13));
                callback.visitChunk(header, &dtlsProtocol);
                bodyLength = B(header->getLength());
            }
            else if ((first & 0xe0) == 0x20 && !(first & 0x10)) {
                auto size = B(1 + ((first & 8) ? 2 : 1) + ((first & 4) ? 2 : 0));
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
