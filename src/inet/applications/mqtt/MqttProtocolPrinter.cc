// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttPacket_m.h"
#include "inet/applications/mqtt/MqttProtocol.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
namespace inet::mqtt {
class MqttProtocolPrinter : public ProtocolPrinter
{
  public:
    void print(const Ptr<const Chunk>& chunk, const Protocol *, const cMessagePrinter::Options *, Context& context) const override {
        auto packet = staticPtrCast<const MqttPacket>(chunk);
        context.typeColumn << "MQTT";
        context.infoColumn << "type=" << packet->getType() << " id=" << packet->getPacketIdentifier();
        if (packet->getType() == PUBLISH)
            context.infoColumn << " topic=" << packet->getTopic() << " QoS=" << packet->getQos();
    }
};
Register_Protocol_Printer(&mqttProtocol, MqttProtocolPrinter);
}
