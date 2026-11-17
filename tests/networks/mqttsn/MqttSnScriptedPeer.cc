// SPDX-License-Identifier: LGPL-3.0-or-later
// Test-only deterministic UDP peer. No production state or fault knobs are added.
#include "inet/applications/mqttsn/MqttSnPacketSerializer.h"
#include "inet/applications/mqttsn/MqttSnProtocol.h"
#include "inet/applications/mqttsn/MqttSnSession.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet::mqttsntest {
class MqttSnScriptedPeer : public cSimpleModule, public UdpSocket::ICallback
{
    UdpSocket socket;
    L3Address remote;
    int remotePort = 0;
    std::vector<cXMLElement *> steps;
    size_t cursor = 0;
    cMessage *timer = nullptr;
    simtime_t anchor;
    int numInitStages() const override { return NUM_INIT_STAGES; }
    int number(cXMLElement *step, const char *key, int fallback = 0) {
        const char *value = step->getAttribute(key); return value ? atoi(value) : fallback;
    }
    const char *text(cXMLElement *step, const char *key, const char *fallback = "") {
        const char *value = step->getAttribute(key); return value ? value : fallback;
    }
    void initialize(int stage) override {
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        timer = new cMessage("scriptDeadline"); socket.setOutputGate(gate("socketOut")); socket.setCallback(this); socket.bind(par("localPort"));
        if (*par("remoteAddress").stringValue()) remote = L3AddressResolver().resolve(par("remoteAddress"));
        remotePort = par("remotePort"); steps = par("script").xmlValue()->getChildren(); anchor = simTime(); drive();
    }
    void drive() {
        cancelEvent(timer);
        if (cursor == steps.size()) { emit(registerSignal("scenarioComplete"), 1L); return; }
        auto step = steps[cursor]; std::string action = step->getTagName();
        simtime_t delay = SimTime::parse(text(step, action == "send" ? "after" : "timeout", action == "send" ? "0s" : "1s"));
        scheduleAt(std::max(simTime(), anchor + delay), timer);
    }
    void advance() { cursor++; anchor = simTime(); drive(); }
    void handleMessage(cMessage *message) override {
        if (message != timer) { socket.processMessage(message); return; }
        auto step = steps.at(cursor); std::string action = step->getTagName();
        if (action == "expect") throw cRuntimeError("MQTT-SN script step %zu timed out waiting for type %d", cursor, number(step, "type"));
        if (action == "wait") { advance(); return; }
        if (action != "send") throw cRuntimeError("Unknown MQTT-SN script action");
        auto packet = mqttsn::MqttSnSession::makePacket(number(step, "type"), number(step, "messageId"));
        packet->setFlags(number(step, "flags")); packet->setTopicId(number(step, "topicId"));
        packet->setGatewayId(number(step, "gatewayId")); packet->setRadius(number(step, "radius", 1));
        packet->setReturnCode(number(step, "code")); packet->setProtocolId(number(step, "protocolId", 1));
        packet->setClientIdentifier(text(step, "clientId")); packet->setTopic(text(step, "topic"));
        packet->setDuration(number(step, "duration")); packet->setDurationPresent(step->getAttribute("duration") != nullptr);
        packet->setWillTopicPresent(step->getAttribute("topic") != nullptr);
        mqttsn::MqttSnSession::setPayload(packet, mqttsn::MqttSnSession::parseHex(text(step, "payload")));
        auto bytes = step->getAttribute("raw") ? mqttsn::MqttSnSession::parseHex(text(step, "raw")) : mqttsn::MqttSnPacketSerializer::encode(*packet);
        socket.sendTo(new Packet("script", makeShared<BytesChunk>(bytes)), remote, remotePort);
        advance();
    }
    void socketDataArrived(UdpSocket *, Packet *message) override {
        remote = message->getTag<L3AddressInd>()->getSrcAddress(); remotePort = message->getTag<L4PortInd>()->getSrcPort();
        auto bytes = message->peekDataAsBytes()->getBytes(); delete message;
        auto packet = mqttsn::MqttSnPacketSerializer::decode(bytes);
        if (cursor == steps.size()) throw cRuntimeError("Unexpected packet after MQTT-SN script completion: %d", packet->getType());
        auto step = steps[cursor]; std::string action = step->getTagName();
        if (action == "wait") {
            if (number(step, "forbidType", -1) == packet->getType()) throw cRuntimeError("Forbidden packet during MQTT-SN silence window");
            return;
        }
        if (action != "expect") throw cRuntimeError("Packet arrived before scripted send");
        auto check = [&](const char *field, int actual) {
            if (step->getAttribute(field) && actual != number(step, field))
                throw cRuntimeError("MQTT-SN script step %zu: %s is %d, expected %d", cursor, field, actual, number(step, field));
        };
        check("type", packet->getType()); check("messageId", packet->getMessageId()); check("topicId", packet->getTopicId());
        check("flags", packet->getFlags()); check("qos", mqttsn::qos(*packet)); check("code", packet->getReturnCode());
        check("dup", bool(packet->getFlags() & mqttsn::DUPLICATE));
        if (step->getAttribute("topic") && std::string(packet->getTopic()) != text(step, "topic")) throw cRuntimeError("Unexpected topic in script");
        if (step->getAttribute("payload") && mqttsn::MqttSnSession::payload(*packet) != mqttsn::MqttSnSession::parseHex(text(step, "payload")))
            throw cRuntimeError("Unexpected payload in script");
        advance();
    }
    void socketErrorArrived(UdpSocket *, Indication *indication) override { delete indication; }
    void socketClosed(UdpSocket *) override {}
    void finish() override { if (cursor != steps.size()) throw cRuntimeError("MQTT-SN script incomplete: %zu/%zu", cursor, steps.size()); }
  public:
    ~MqttSnScriptedPeer() { cancelAndDelete(timer); }
};
Define_Module(MqttSnScriptedPeer);
}
