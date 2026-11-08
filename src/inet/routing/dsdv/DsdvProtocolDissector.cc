// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/routing/dsdv/DsdvProtocolDissector.h"
#include "inet/routing/dsdv/DsdvProtocol.h"
#include "inet/routing/dsdv/DsdvHeader_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
namespace inet {
namespace dsdv {
Register_Protocol_Dissector(&dsdvProtocol, DsdvProtocolDissector);
void DsdvProtocolDissector::dissect(Packet *packet, const Protocol *, ICallback& callback) const
{
    const auto& header = packet->popAtFront<DsdvHeader>();
    callback.startProtocolDataUnit(&dsdvProtocol);
    callback.visitChunk(header, &dsdvProtocol);
    callback.endProtocolDataUnit(&dsdvProtocol);
}
} // namespace dsdv
} // namespace inet
