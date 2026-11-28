// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttPacket_m.h"
#include "inet/applications/mqtt/MqttPacketSerializer.h"
#include "inet/applications/mqtt/MqttProtocol.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
namespace inet::mqtt {
class MqttProtocolDissector : public ProtocolDissector
{
  public:
    void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override {
        callback.startProtocolDataUnit(protocol);
        while (packet->getDataLength() > b(0)) {
            // TCP captures can begin or end in the middle of a control packet.
            // Inspection must not turn an incomplete capture into a simulation error.
            Ptr<MqttPacket> decoded;
            size_t length = 0;
            try {
                auto bytes = packet->peekDataAsBytes()->getBytes();
                length = MqttPacketSerializer::frameLength(bytes);
                if (length && length <= bytes.size())
                    decoded = MqttPacketSerializer::decode(std::vector<uint8_t>(bytes.begin(), bytes.begin() + length));
            }
            catch (cRuntimeError&) { callback.markIncorrect(); }
            if (!decoded) {
                callback.dissectPacket(packet, nullptr);
                break;
            }
            packet->popAtFront(B(length));
            callback.visitChunk(decoded, protocol);
        }
        callback.endProtocolDataUnit(protocol);
    }
};
Register_Protocol_Dissector(&mqttProtocol, MqttProtocolDissector);
}
