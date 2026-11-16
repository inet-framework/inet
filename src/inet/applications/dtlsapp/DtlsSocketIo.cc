// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/dtlsapp/DtlsSocketIo.h"

#include "inet/common/ModuleAccess.h"
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

void DtlsSocketIo::accepted(IDtls::AssociationId, const L3Address&, int, const L3Address&, int)
{
}

void DtlsSocketIo::established(IDtls::AssociationId id)
{
    EV_INFO << "DTLS association " << id << " established" << endl;
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

} // namespace inet::dtls
