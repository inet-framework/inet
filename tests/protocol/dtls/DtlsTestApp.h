// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_PROTOCOLTEST_DTLS_DTLSTESTAPP_H
#define __INET_PROTOCOLTEST_DTLS_DTLSTESTAPP_H

#include <set>

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/dtls/IDtls.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet::protocoltest::dtls {

/** Drives the real engine; it contains no handshake/replay/recovery implementation. */
class DtlsTestDriver : public cSimpleModule, public inet::dtls::IDtls::ICallback,
                       public UdpSocket::ICallback
{
  public:
    using cSimpleModule::error;
    using AssociationId = inet::dtls::IDtls::AssociationId;
    enum Event { ACCEPTED = 1, ESTABLISHED = 2, ERROR = 3, CLOSED = 4, SEND_ERROR = 5 };
    inet::dtls::IDtls *engine = nullptr;
    std::vector<AssociationId> handles;
    struct Endpoint {
        AssociationId id;
        L3Address localAddress;
        int localPort;
        L3Address peerAddress;
        int peerPort;
    };
    std::vector<Endpoint> acceptedEndpoints;
    std::vector<int> receivedMarkers;
    std::vector<AssociationId> receivedHandles;
    std::vector<AssociationId> establishedHandles;
    std::vector<AssociationId> closedHandles;
    std::vector<bool> updateResults;
    std::vector<bool> cidResults;
    std::vector<bool> migrationResults;
    std::vector<bool> authenticationAdmissions;
    std::vector<bool> authenticationResults;
    std::vector<AssociationId> authenticationHandles;
    std::vector<std::string> authenticationIdentities;
    std::vector<inet::dtls::DtlsPath> confirmedPaths;
    std::vector<bool> earlyResults;
    std::vector<int> earlyMarkers;
    bool closeOnEarlyData = false;
    int establishedCount = 0;
    int errorCount = 0;
    int closedCount = 0;
    int sendErrorCount = 0;
    std::string lastError;
    simtime_t establishedAt;
    UdpSocket rawSocket;

  protected:
    class Command : public cMessage {
      public:
        std::function<void()> call;
        explicit Command(std::function<void()> call) : cMessage("dtlsTestStimulus"), call(std::move(call)) {}
    };
    cMessage *start = nullptr;
    std::set<cMessage *> commands;
    simsignal_t stateSignal;
    void scheduleCommand(std::function<void()> call)
    {
        auto command = new Command(std::move(call));
        commands.insert(command);
        scheduleAt(simTime(), command);
    }
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override
    {
        if (stage == INITSTAGE_LOCAL) {
            stateSignal = registerSignal("controlStateChanged");
            start = new cMessage("startDtlsTest");
        }
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            engine = dynamic_cast<inet::dtls::IDtls *>(getParentModule()->getSubmodule("dtls"));
            if (!engine)
                throw cRuntimeError("DTLS test requires an IDtls production engine");
            rawSocket.setOutputGate(gate("socketOut"));
            rawSocket.setCallback(this);
            rawSocket.bind(par("rawPort"));
            if (par("active").boolValue())
                scheduleAt(par("startTime"), start);
        }
    }
    void handleMessage(cMessage *message) override
    {
        if (message == start) {
            if (par("server").boolValue())
                engine->listen(this);
            else
                connectPeer();
        }
        else if (auto command = dynamic_cast<Command *>(message)) {
            commands.erase(command);
            auto call = std::move(command->call);
            delete command;
            call();
        }
        else if (rawSocket.belongsToSocket(message))
            rawSocket.processMessage(message);
        else
            delete message;
    }

  public:
    ~DtlsTestDriver() override
    {
        cancelAndDelete(start);
        for (auto command : commands)
            cancelAndDelete(command);
    }
    AssociationId connectPeer()
    {
        auto id = engine->open(L3AddressResolver().resolve(par("peerAddress")), par("peerPort"), this);
        handles.push_back(id);
        return id;
    }
    void reopenPeer() { scheduleCommand([this]() { connectPeer(); }); }
    void resumePeer()
    {
        scheduleCommand([this]() {
            handles.push_back(engine->open(L3AddressResolver().resolve(par("peerAddress")), par("peerPort"), this, true));
        });
    }
    bool isResumed() const { return engine->isResumed(lastHandle()); }
    void resumeEarly(int marker = 46, int bytes = 32)
    {
        scheduleCommand([this, marker, bytes]() {
            auto id = engine->open(L3AddressResolver().resolve(par("peerAddress")), par("peerPort"), this, inet::dtls::DtlsOpenOptions{true, true});
            handles.push_back(id);
            auto packet = new Packet("dtls-test-early");
            packet->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>(bytes, marker)));
            engine->sendEarly(id, packet, this);
        });
    }
    void earlyDataStatus(AssociationId, bool accepted) override
    {
        earlyResults.push_back(accepted);
        emit(registerSignal("earlyDataStatus"), accepted ? 1L : 0L);
    }
    void pathChanged(AssociationId, const inet::dtls::DtlsPath& path) override
    {
        confirmedPaths.push_back(path);
        emit(registerSignal("pathChanged"), (long)confirmedPaths.size());
    }
    void authenticationResult(AssociationId id, bool success, const char *identity) override
    {
        authenticationResults.push_back(success);
        authenticationHandles.push_back(id);
        authenticationIdentities.push_back(identity);
        emit(registerSignal("authenticationResult"), success ? 1L : 0L);
    }
    void requestAuthentication() { scheduleCommand([this]() { authenticationAdmissions.push_back(engine->requestClientAuthentication(lastHandle())); }); }
    inet::dtls::DtlsPeerAuthentication peerAuthentication() const
    {
        inet::dtls::DtlsPeerAuthentication result;
        if (!engine->getPeerAuthentication(lastHandle(), result))
            throw cRuntimeError("no established peer authentication state");
        return result;
    }
    void sendEarlyMarker(int marker, int bytes = 32)
    {
        scheduleCommand([this, marker, bytes]() {
            auto packet = new Packet("dtls-test-early");
            packet->insertAtBack(makeShared<BytesChunk>(std::vector<uint8_t>(bytes, marker)));
            engine->sendEarly(lastHandle(), packet, this);
        });
    }
    void earlyDataArrived(AssociationId id, Packet *packet) override
    {
        earlyMarkers.push_back(packet->peekDataAsBytes()->getBytes().at(0));
        emit(registerSignal("earlyPacketReceived"), packet);
        delete packet;
        emit(registerSignal("earlyDataReceived"), (long)earlyMarkers.size());
        if (closeOnEarlyData)
            engine->close(id);
    }
    void listenPeer() { scheduleCommand([this]() { engine->listen(this); }); }
    void openPeer(const char *address, int port = 4433)
    {
        std::string peer(address);
        scheduleCommand([this, peer, port]() { handles.push_back(engine->open(L3AddressResolver().resolve(peer.c_str()), port, this)); });
    }
    AssociationId lastHandle() const
    {
        if (handles.empty())
            throw cRuntimeError("No DTLS association has been opened/accepted");
        return handles.back();
    }
    void sendMarker(int marker, int bytes = 32, AssociationId id = 0)
    {
        auto target = id ? id : lastHandle();
        scheduleCommand([this, marker, bytes, target]() {
            auto packet = new Packet("dtls-test-data");
            if (bytes > 0) {
                std::vector<uint8_t> data(bytes, marker);
                packet->insertAtBack(makeShared<BytesChunk>(data));
            }
            engine->send(target, packet, this);
        });
    }
    void update(bool requestPeer = false)
    {
        scheduleCommand([this, requestPeer]() { updateResults.push_back(engine->requestKeyUpdate(lastHandle(), requestPeer)); });
    }
    void requestCids(int count)
    {
        scheduleCommand([this, count]() { cidResults.push_back(engine->requestConnectionIds(lastHandle(), count)); });
    }
    void provideCids(int count, bool immediate)
    {
        scheduleCommand([this, count, immediate]() { cidResults.push_back(engine->provideConnectionIds(lastHandle(), count, immediate)); });
    }
    void activateCid()
    {
        scheduleCommand([this]() { cidResults.push_back(engine->activateSpareConnectionId(lastHandle())); });
    }
    void rebindPort(int port)
    {
        scheduleCommand([this, port]() { migrationResults.push_back(engine->rebindLocalPort(lastHandle(), port)); });
    }
    void migrateAddress(const L3Address& address, int interfaceId = -1)
    {
        scheduleCommand([this, address, interfaceId]() { migrationResults.push_back(engine->migrateLocal(lastHandle(), address, interfaceId)); });
    }
    void sendBits(int bitCount)
    {
        scheduleCommand([this, bitCount]() {
            auto packet = new Packet("dtls-test-unaligned-data");
            packet->insertAtBack(makeShared<BitsChunk>(std::vector<bool>(bitCount, true)));
            engine->send(lastHandle(), packet, this);
        });
    }
    void closePeer(AssociationId id = 0)
    {
        auto target = id ? id : lastHandle();
        scheduleCommand([this, target]() { engine->close(target); });
    }
    void sendRaw(Packet *packet, const char *destination = "host2", int port = 4433)
    {
        rawSocket.sendTo(packet, L3AddressResolver().resolve(destination), port);
    }
    void accepted(AssociationId id, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort) override
    {
        handles.push_back(id);
        acceptedEndpoints.push_back({id, localAddress, localPort, peerAddress, peerPort});
        emit(stateSignal, (long)ACCEPTED);
    }
    void established(AssociationId id) override
    {
        establishedCount++;
        establishedHandles.push_back(id);
        establishedAt = simTime();
        emit(stateSignal, (long)ESTABLISHED);
    }
    void dataArrived(AssociationId id, Packet *packet) override
    {
        int marker = -1;
        if (packet->getDataLength() > B(0)) {
            auto bytes = packet->peekDataAsBytes();
            marker = bytes->getBytes().at(0);
        }
        receivedMarkers.push_back(marker);
        receivedHandles.push_back(id);
        emit(registerSignal("packetSentToUpper"), packet);
        delete packet;
    }
    void sendError(AssociationId, long, const char *reason) override
    {
        sendErrorCount++;
        lastError = reason;
        emit(stateSignal, (long)SEND_ERROR);
    }
    void error(AssociationId, const char *reason) override
    {
        errorCount++;
        lastError = reason;
        emit(stateSignal, (long)ERROR);
    }
    void closed(AssociationId id) override
    {
        closedCount++;
        closedHandles.push_back(id);
        emit(stateSignal, (long)CLOSED);
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override { delete packet; }
    void socketErrorArrived(UdpSocket *, Indication *indication) override { delete indication; }
    void socketClosed(UdpSocket *) override {}
};

Define_Module(DtlsTestDriver);

inline void require(bool condition, const char *description)
{
    if (!condition)
        throw cRuntimeError("DTLS protocol assertion failed: %s", description);
}

} // namespace inet::protocoltest::dtls

#endif
