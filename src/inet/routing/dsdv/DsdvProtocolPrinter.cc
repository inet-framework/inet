// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/routing/dsdv/DsdvProtocolPrinter.h"
#include "inet/routing/dsdv/DsdvProtocol.h"
#include "inet/routing/dsdv/DsdvHeader_m.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
namespace inet {
namespace dsdv {
Register_Protocol_Printer(&dsdvProtocol, DsdvProtocolPrinter);
void DsdvProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *, const cMessagePrinter::Options *, Context& context) const
{
    const auto& header = staticPtrCast<const DsdvHeader>(chunk);
    context.infoColumn << "DSDV v" << int(header->getVersion()) << " " << header->getSender()
                       << " seq=" << header->getSelfSequence() << " kind=" << int(header->getKind())
                       << " entries=" << header->getEntriesArraySize();
}
} // namespace dsdv
} // namespace inet
