// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/dtlsapp/DtlsSocketIo.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/XMLUtils.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet::dtls {

Define_Module(DtlsSocketIo);

void DtlsSocketIo::handleStartOperation(LifecycleOperation *operation)
{
    server = par("server");
    socket.setEngine(getModuleFromPar<IDtls>(par("dtlsModule"), this));
    socket.setCallback(this);
    if (server)
        socket.listen();
    else {
        const char *address = par("destAddress");
        if (!*address)
            throw cRuntimeError("A DTLS client requires destAddress");
        clientId = socket.open(L3AddressResolver().resolve(address), par("destPort"), par("resumeSession"));
        if (clientId == 0)
            EV_WARN << "DTLS association was not admitted" << endl;
    }
}

void DtlsSocketIo::handleMessageWhenUp(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    if (server || clientId == 0) {
        EV_WARN << "Dropping an unaddressed DTLS packet" << endl;
        delete packet;
        return;
    }
    emit(packetSentSignal, packet);
    socket.send(clientId, packet);
}

void DtlsSocketIo::accepted(IDtls::AssociationId id, const L3Address&, int, const L3Address&, int)
{
    clientId = id;
}

void DtlsSocketIo::established(IDtls::AssociationId id)
{
    EV_INFO << "DTLS association " << id << " established, resumed=" << socket.isResumed(id) << endl;
}

void DtlsSocketIo::dataArrived(IDtls::AssociationId, Packet *packet)
{
    Enter_Method("dataArrived");
    take(packet);
    if (operationalState == NOT_OPERATING || operationalState == CRASHING_OPERATION) {
        delete packet;
        return;
    }
    emit(packetReceivedSignal, packet);
    send(packet, "trafficOut");
}

void DtlsSocketIo::authenticationResult(IDtls::AssociationId id, bool success, const char *identity)
{
    Enter_Method("authenticationResult");
    EV_INFO << "DTLS association " << id << " authentication " << (success ? "succeeded: " : "failed: ") << identity << endl;
    emit(registerSignal("authenticationResult"), success ? 1L : 0L);
}

void DtlsSocketIo::maximumDatagramSizeChanged(IDtls::AssociationId id, size_t bytes)
{
    Enter_Method("maximumDatagramSizeChanged");
    EV_INFO << "DTLS association " << id << " maximum application datagram: " << bytes << " bytes" << endl;
    emit(registerSignal("maximumDatagramSizeChanged"), long(bytes));
}

void DtlsSocketIo::sendError(IDtls::AssociationId id, long packetId, const char *reason)
{
    EV_WARN << "DTLS send " << packetId << " on association " << id << " failed: " << reason << endl;
}

void DtlsSocketIo::earlyDataArrived(IDtls::AssociationId, Packet *packet)
{
    Enter_Method("earlyDataArrived");
    take(packet);
    if (operationalState == NOT_OPERATING || operationalState == CRASHING_OPERATION || !par("acceptEarlyData").boolValue()) {
        delete packet;
        return;
    }
    emit(registerSignal("earlyPacketReceived"), packet);
    send(packet, "trafficOut");
}

void DtlsSocketIo::earlyDataStatus(IDtls::AssociationId id, bool accepted)
{
    EV_INFO << "DTLS association " << id << " early data " << (accepted ? "accepted" : "rejected") << endl;
}

void DtlsSocketIo::pathChanged(IDtls::AssociationId id, const DtlsPath& path)
{
    Enter_Method("pathChanged");
    EV_INFO << "DTLS association " << id << " path " << path.localAddress << ":" << path.localPort << " -> " << path.peerAddress << ":" << path.peerPort << endl;
    emit(registerSignal("pathChanged"), (long)id);
}

void DtlsSocketIo::error(IDtls::AssociationId id, const char *reason)
{
    if (id == clientId)
        clientId = 0;
    EV_WARN << "DTLS association " << id << ": " << reason << endl;
}

void DtlsSocketIo::closed(IDtls::AssociationId id)
{
    if (id == clientId)
        clientId = 0;
}

void DtlsSocketIo::handleStopOperation(LifecycleOperation *)
{
    clientId = 0;
}

void DtlsSocketIo::handleCrashOperation(LifecycleOperation *)
{
    clientId = 0;
}

void DtlsSocketIo::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (operationalState != OPERATING)
        throw cRuntimeError("DTLS scenario action requires an operating adapter");
    std::string command = node.getTagName();
    if (command == "open-dtls") {
        if (server || clientId != 0)
            throw cRuntimeError("open-dtls requires a client without a current association");
        DtlsOpenOptions options;
        options.resumeSession = par("resumeSession");
        auto early = node.getAttribute("early");
        if (early && strcmp(early, "true") && strcmp(early, "false"))
            throw cRuntimeError("open-dtls early must be true or false");
        options.offerEarlyData = early && !strcmp(early, "true");
        clientId = socket.open(L3AddressResolver().resolve(par("destAddress")), par("destPort"), options);
        if (clientId == 0)
            throw cRuntimeError("DTLS scenario open was not admitted");
        return;
    }
    if (clientId == 0)
        throw cRuntimeError("DTLS scenario action requires a current association");
    if (command == "close-dtls")
        socket.close(clientId);
    else if (command == "send-early-dtls") {
        if (server)
            throw cRuntimeError("send-early-dtls requires a client");
        auto packet = new Packet("scenario early data", makeShared<ByteCountChunk>(B(32)));
        socket.sendEarly(clientId, packet);
    }
    else if (command == "request-dtls-cid") {
        if (!socket.requestConnectionIds(clientId))
            throw cRuntimeError("DTLS scenario CID request was not admitted");
    }
    else if (command == "rebind-dtls") {
        std::string value = xmlutils::getMandatoryFilledAttribute(node, "port");
        size_t parsed = 0;
        auto port = std::stoi(value, &parsed);
        if (parsed != value.size() || port < 1 || port > 65535)
            throw cRuntimeError("rebind-dtls requires an integer port in 1..65535");
        if (!socket.rebindLocalPort(clientId, port))
            throw cRuntimeError("DTLS scenario rebind was not admitted");
    }
    else if (command == "authenticate-dtls") {
        if (!socket.requestClientAuthentication(clientId))
            throw cRuntimeError("DTLS scenario authentication was not admitted");
    }
    else
        throw cRuntimeError("Unknown DTLS scenario command: %s", command.c_str());
}

} // namespace inet::dtls
