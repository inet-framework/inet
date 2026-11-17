// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqttsn/MqttSnProtocol.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"

namespace inet::mqttsn {
class MqttSnProtocolPrinter : public ProtocolPrinter
{
  public:
    void print(const Ptr<const Chunk>& chunk, const Protocol *, const cMessagePrinter::Options *, Context& context) const override {
        auto packet = staticPtrCast<const MqttSnPacket>(chunk);
        context.typeColumn << "MQTT-SN";
        context.infoColumn << "type=" << packet->getType() << " id=" << packet->getMessageId()
            << " topic=" << packet->getTopicId() << " kind=" << topicKind(*packet)
            << " QoS=" << qos(*packet) << " code=" << packet->getReturnCode();
    }
};
Register_Protocol_Printer(&mqttSnProtocol, MqttSnProtocolPrinter);
}
