// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/transportlayer/dtls/Dtls.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/Dtls12CidHeader_m.h"

namespace inet::dtls {

Register_Protocol_Printer(&dtlsProtocol, DtlsProtocolPrinter);

void DtlsProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *, const cMessagePrinter::Options *, Context& context) const
{
    context.typeColumn << "DTLS";
    if (auto header = dynamicPtrCast<const DtlsPlaintextHeader>(chunk))
        context.infoColumn << "epoch=" << header->getEpoch() << " sequence=" << header->getSequenceNumber() << " type=" << (int)header->getContentType();
    else if (auto header = dynamicPtrCast<const Dtls12CidHeader>(chunk))
        context.infoColumn << "epoch=" << header->getEpoch() << " sequence=" << header->getSequenceNumber() << " CID bytes=" << header->getConnectionIdArraySize();
    else if (auto header = dynamicPtrCast<const DtlsCiphertextHeader>(chunk)) {
        context.infoColumn << "epoch bits=" << (int)header->getEpochBits() << " truncated sequence=" << header->getSequenceNumber();
        if (header->getConnectionIdArraySize())
            context.infoColumn << " CID bytes=" << header->getConnectionIdArraySize();
    }
    else
        context.infoColumn << chunk;
}

} // namespace inet::dtls
