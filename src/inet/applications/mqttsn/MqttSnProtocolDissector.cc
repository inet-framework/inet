// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqttsn/MqttSnPacketSerializer.h"
#include "inet/applications/mqttsn/MqttSnProtocol.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet::mqttsn {
class MqttSnProtocolDissector : public ProtocolDissector
{
  public:
    void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override {
        callback.startProtocolDataUnit(protocol);
        Ptr<MqttSnPacket> decoded;
        try { decoded = MqttSnPacketSerializer::decode(packet->peekDataAsBytes()->getBytes()); }
        catch (cRuntimeError&) { callback.markIncorrect(); }
        if (decoded) {
            packet->popAtFront(decoded->getChunkLength());
            callback.visitChunk(decoded, protocol);
        }
        else callback.dissectPacket(packet, nullptr);
        callback.endProtocolDataUnit(protocol);
    }
};
Register_Protocol_Dissector(&mqttSnProtocol, MqttSnProtocolDissector);
}
