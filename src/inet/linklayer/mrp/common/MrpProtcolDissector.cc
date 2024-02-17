#include "inet/linklayer/mrp/common/MrpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/mrp/common/MrpPdu_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::mrp, MrpProtocolDissector);

void MrpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback &callback) const {
    /**
     auto version = packet->popAtFront<mrpVersionField>();
     auto firstTLV=packet->popAtFront<tlvHeader>();
     auto endTLV=packet->popAtBack<tlvHeader>();
     callback.startProtocolDataUnit(&Protocol::mrp);
     callback.visitChunk(version, &Protocol::mrp);
     callback.visitChunk(firstTLV, &Protocol::mrp);
     callback.endProtocolDataUnit(&Protocol::mrp);
     **/
}

} // namespace inet
