//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <openssl/pem.h>
#include "picotls/openssl_opp.h"

#include "Quic.h"
#include "inet/common/Protocol.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/quic/QuicCommand_m.h"
#include "UdpSocket.h"
#include "AppSocket.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/IcmpErrorTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "exception/ConnectionDiedException.h"
#include "packet/ConnectionId.h"
#include "packet/EncryptedQuicPacketChunk.h"

namespace inet {
namespace quic {

Define_Module(Quic);

void random_bytes(void *buf, size_t len) {
    cModule *contextModule = cSimulation::getActiveSimulation()->getContextModule();
    uint8_t *bytebuf = (uint8_t *)buf;
    for (int i = 0; i < len; i++)
        bytebuf[i] = contextModule->intrand(256);
    std::cout << "random bytes: ";
    for (int i = 0; i < len; i++)
        std::cout << std::hex << (int)bytebuf[i] << " ";
    std::cout << std::dec << std::endl;
}

uint64_t get_simtime(ptls_get_time_t *self)
{
    return cSimulation::getActiveSimulation()->getSimTime().inUnit(SIMTIME_MS);
}

ptls_get_time_t opp_get_time = {get_simtime};

static int on_update_traffic_key(ptls_update_traffic_key_t *self, ptls_t *tls, int is_enc, size_t epoch, const void *secret)
{
    Connection *conn = (Connection *)(*ptls_get_data_ptr(tls));

    std::cout << "updating " << (is_enc ? "egress" : "ingress") << " traffic key for epoch " << epoch << " at " << (conn->is_server ? "server" : "client") << std::endl;
    char secret_hex[65];
    ptls_hexdump(secret_hex, secret, 32);
    //std::cout << "new secret: " << secret_hex << std::endl;

    ptls_iovec_t secret_iovec = ptls_iovec_init((uint8_t *)secret, 32);

    EncryptionKey encryptionKey = EncryptionKey::deriveFromSecret(secret_iovec);

    if (is_enc)
        conn->egressKeys[epoch] = encryptionKey;
    else
        conn->ingressKeys[epoch] = encryptionKey;

    ptls_iovec_t client_random = ptls_get_client_random(tls);
    char client_random_hex[65];
    ptls_hexdump(client_random_hex, client_random.base, client_random.len);
    //std::cout << "client random: " << client_random_hex << std::endl;

    static const char *log_labels[2][4] = {
        {NULL, "CLIENT_EARLY_TRAFFIC_SECRET", "CLIENT_HANDSHAKE_TRAFFIC_SECRET", "CLIENT_TRAFFIC_SECRET_0"},
        {NULL, NULL, "SERVER_HANDSHAKE_TRAFFIC_SECRET", "SERVER_TRAFFIC_SECRET_0"}};
    const char *log_label = log_labels[ptls_is_server(tls) == is_enc][epoch];

    Quic *quicSimpleMod = conn->getModule();

    std::string tlsClientRandomLine = log_label;
    tlsClientRandomLine += " ";
    tlsClientRandomLine += client_random_hex;
    tlsClientRandomLine += " ";
    tlsClientRandomLine += secret_hex;
    tlsClientRandomLine += "\n";
    std::cout << "tls line: " << tlsClientRandomLine << std::endl;
    quicSimpleMod->emit(quicSimpleMod->tlsKeyLogLineSignal, tlsClientRandomLine.c_str());


    return 0;
}

ptls_update_traffic_key_t update_traffic_key = {on_update_traffic_key};


static int override_verify_certificate(ptls_openssl_opp_override_verify_certificate_t *self, ptls_t *tls, int ret, int ossl_ret, X509 *cert, STACK_OF(X509) * chain)
{
    std::cout << "override_verify_certificate called with ret=" << ret << ", ossl_ret=" << ossl_ret << std::endl;
    return 0;
}

ptls_openssl_opp_override_verify_certificate_t override_verify = {override_verify_certificate};


#define RSA_PRIVATE_KEY                                                                                                            \
    "-----BEGIN RSA PRIVATE KEY-----\n"                                                                                            \
    "MIIEpAIBAAKCAQEA7zZheZ4ph98JaedBNv9kqsVA9CSmhd69kBc9ZAfVFMA4VQwp\n"                                                           \
    "rOj3ZGrxf20HB3FkvqGvew9ZogUF6NjbPumeiUObGpP21Y5wcYlPL4aojlrwMB/e\n"                                                           \
    "OxOCpuRyQTRSSe1hDPvdJABQdmshDP5ZSEBLdUSgrNn4KWhIDjFj1AHXIMqeqTXe\n"                                                           \
    "tFuRgNzHdtbXQx+UWBis2B6qZJuqSArb2msVOC8D5gNznPPlQw7FbdPCaLNXSb6G\n"                                                           \
    "nI0E0uj6QmYlAw9s6nkgP/zxjfFldqPNUprGcEqTwmAb8VVtd7XbANYrzubZ4Nn6\n"                                                           \
    "/WXrCrVxWUmh/7Spgdwa/I4Nr1JHv9HHyL2z/wIDAQABAoIBAEVPf2zKrAPnVwXt\n"                                                           \
    "cJLr6xIj908GM43EXS6b3TjXoCDUFT5nOMgV9GCPMAwY3hmE/IjTtlG0v+bXB8BQ\n"                                                           \
    "3S3caQgio5VO3A1CqUfsXhpKLRqaNM/s2+pIG+oZdRV5gIJVGnK1o3yj7qxxG/F0\n"                                                           \
    "3Q+3OWXwDZIn0eTFh2M9YkxygA/KtkREZWv8Q8qZpdOpJSBYZyGE97Jqy/yGc+DQ\n"                                                           \
    "Vpoa9B8WwnIdUn47TkZfsbzqGIYZxatJQDC1j7Y+F8So7zBbUhpz7YqATQwf5Efm\n"                                                           \
    "K2xwvlwfdwykq6ffEr2M/Xna0220G2JZlGq3Cs2X9GT9Pt9OS86Bz+EL46ELo0tZ\n"                                                           \
    "yfHQe/kCgYEA+zh4k2be6fhQG+ChiG3Ue5K/kH2prqyGBus61wHnt8XZavqBevEy\n"                                                           \
    "4pdmvJ6Q1Ta9Z2YCIqqNmlTdjZ6B35lvAK8YFITGy0MVV6K5NFYVfhALWCQC2r3B\n"                                                           \
    "6uH39FQ0mDo3gS5ZjYlUzbu67LGFnyX+pyMr2oxlhI1fCY3VchXQAOsCgYEA88Nt\n"                                                           \
    "CwSOaZ1fWmyNAgXEAX1Jx4XLFYgjcA/YBXW9gfQ0AfufB346y53PsgjX1lB+Bbcg\n"                                                           \
    "cY/o5W7F0b3A0R4K5LShlPCq8iB2DC+VnpKwTgo8ylh+VZCPy2BmMK0jrrmyqWeg\n"                                                           \
    "PzwgP0lp+7l/qW8LDImeYi8nWoqd6f1ye4iJdD0CgYEAlIApJljk5EFYeWIrmk3y\n"                                                           \
    "EKoKewsNRqfNAkICoh4KL2PQxaAW8emqPq9ol47T5nVZOMnf8UYINnZ8EL7l3psA\n"                                                           \
    "NtNJ1Lc4G+cnsooKGJnaUo6BZjTDSzJocsPoopE0Fdgz/zS60yOe8Y5LTKcTaaQ4\n"                                                           \
    "B+yOe74KNHSs/STOS4YBUskCgYAIqaRBZPsOo8oUs5DbRostpl8t2QJblIf13opF\n"                                                           \
    "v2ZprN0ASQngwUqjm8sav5e0BQ5Fc7mSb5POO36KMp0ckV2/vO+VFGxuyFqJmlNN\n"                                                           \
    "3Fapn1GDu1tZ/RYvGxDmn/CJsA26WXVnaeKXfStoB7KSueCBpI5dXOGgJRbxjtE3\n"                                                           \
    "tKV13QKBgQCtmLtTJPJ0Z+9n85C8kBonk2MCnD9JTYWoDQzNMYGabthzSqJqcEek\n"                                                           \
    "dvhr82XkcHM+r6+cirjdQr4Qj7/2bfZesHl5XLvoJDB1YJIXnNJOELwbktrJrXLc\n"                                                           \
    "dJ+MMvPvBAMah/tqr2DqgTGfWLDt9PJiCJVsuN2kD9toWHV08pY0Og==\n"                                                                   \
    "-----END RSA PRIVATE KEY-----\n"

#define RSA_CERTIFICATE                                                                                                            \
    "-----BEGIN CERTIFICATE-----\n"                                                                                                \
    "MIIDOjCCAiKgAwIBAgIBATANBgkqhkiG9w0BAQsFADAWMRQwEgYDVQQDEwtIMk8g\n"                                                           \
    "VGVzdCBDQTAeFw0xNDEyMTAxOTMzMDVaFw0yNDEyMDcxOTMzMDVaMBsxGTAXBgNV\n"                                                           \
    "BAMTEDEyNy4wLjAuMS54aXAuaW8wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"                                                           \
    "AoIBAQDvNmF5nimH3wlp50E2/2SqxUD0JKaF3r2QFz1kB9UUwDhVDCms6PdkavF/\n"                                                           \
    "bQcHcWS+oa97D1miBQXo2Ns+6Z6JQ5sak/bVjnBxiU8vhqiOWvAwH947E4Km5HJB\n"                                                           \
    "NFJJ7WEM+90kAFB2ayEM/llIQEt1RKCs2fgpaEgOMWPUAdcgyp6pNd60W5GA3Md2\n"                                                           \
    "1tdDH5RYGKzYHqpkm6pICtvaaxU4LwPmA3Oc8+VDDsVt08Jos1dJvoacjQTS6PpC\n"                                                           \
    "ZiUDD2zqeSA//PGN8WV2o81SmsZwSpPCYBvxVW13tdsA1ivO5tng2fr9ZesKtXFZ\n"                                                           \
    "SaH/tKmB3Br8jg2vUke/0cfIvbP/AgMBAAGjgY0wgYowCQYDVR0TBAIwADAsBglg\n"                                                           \
    "hkgBhvhCAQ0EHxYdT3BlblNTTCBHZW5lcmF0ZWQgQ2VydGlmaWNhdGUwHQYDVR0O\n"                                                           \
    "BBYEFJXhddVQ68vtPvxoHWHsYkLnu3+4MDAGA1UdIwQpMCehGqQYMBYxFDASBgNV\n"                                                           \
    "BAMTC0gyTyBUZXN0IENBggkAmqS1V7DvzbYwDQYJKoZIhvcNAQELBQADggEBAJQ2\n"                                                           \
    "uvzL/lZnrsF4cvHhl/mg+s/RjHwvqFRrxOWUeWu2BQOGdd1Izqr8ZbF35pevPkXe\n"                                                           \
    "j3zQL4Nf8OxO/gx4w0165KL4dYxEW7EaxsDQUI2aXSW0JNSvK2UGugG4+E4aT+9y\n"                                                           \
    "cuBCtfWbL4/N6IMt2QW17B3DcigkreMoZavnnqRecQWkOx4nu0SmYg1g2QV4kRqT\n"                                                           \
    "nvLt29daSWjNhP3dkmLTxn19umx26/JH6rqcgokDfHHO8tlDbc9JfyxYH01ZP2Ps\n"                                                           \
    "esIiGa/LBXfKiPXxyHuNVQI+2cMmIWYf+Eu/1uNV3K55fA8806/FeklcQe/vvSCU\n"                                                           \
    "Vw6RN5S/14SQnMYWr7E=\n"                                                                                                       \
    "-----END CERTIFICATE-----\n"


Quic::Quic() : OperationalBase()
{
    memset(&ctx, 0, sizeof(ctx));
    ctx.random_bytes = random_bytes;
    ctx.key_exchanges = ptls_openssl_opp_key_exchanges;
    ctx.cipher_suites = ptls_openssl_opp_cipher_suites;
    ctx.get_time = &opp_get_time;
    ctx.update_traffic_key = &update_traffic_key;

    {
        BIO *bio = BIO_new_mem_buf(RSA_CERTIFICATE, strlen(RSA_CERTIFICATE));
        X509 *x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
        assert(x509 != NULL || !!"failed to load certificate");
        BIO_free(bio);
        cert.len = i2d_X509(x509, &cert.base);
        X509_free(x509);
    }

    {
        BIO *bio = BIO_new_mem_buf(RSA_PRIVATE_KEY, strlen(RSA_PRIVATE_KEY));
        EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
        assert(pkey != NULL || !"failed to load private key");
        BIO_free(bio);
        ptls_openssl_opp_init_sign_certificate(&cert_signer, pkey);
        EVP_PKEY_free(pkey);
    }

    ptls_openssl_opp_init_verify_certificate(&verifier, NULL);
    verifier.override_callback = &override_verify;
    ctx.verify_certificate = &verifier.super;

    ctx.certificates = {&cert, 1};

    tlsKeyLogLineSignal = registerSignal("tlsKeyLogLine");
}

Quic::~Quic() {
    std::set<Connection *> connections;
    for (std::map<uint64_t, Connection *>::iterator it = connectionIdConnectionMap.begin(); it != connectionIdConnectionMap.end(); ++it) {
        connections.insert(it->second);
    }
    for (Connection *connection : connections) {
        delete connection;
    }
    for (std::map<int, UdpSocket *>::iterator it = udpSocketIdUdpSocketMap.begin(); it != udpSocketIdUdpSocketMap.end(); ++it) {
        delete it->second;
    }
    for (std::map<int, AppSocket *>::iterator it = socketIdAppSocketMap.begin(); it != socketIdAppSocketMap.end(); ++it) {
        delete it->second;
    }
}

void Quic::handleStartOperation(LifecycleOperation *operation)
{
    EV_DEBUG << "start operation" << endl;
    registerService(Protocol::quic, gate("appIn"), gate("udpIn"));
    registerProtocol(Protocol::quic, gate("udpOut"), gate("appOut"));
}

void Quic::handleStopOperation(LifecycleOperation *operation)
{
    EV_DEBUG << "stop operation" << endl;
    //removeAllConnections();
}

void Quic::handleCrashOperation(LifecycleOperation *operation)
{
    EV_DEBUG << "crash operation" << endl;
    //removeAllConnections();
}

void Quic::handleMessageWhenUp(cMessage *msg)
{
    EV_DEBUG << "handle message when up of kind " << msg->getKind() << endl;

    if (msg->isSelfMessage()) {
        handleTimeout(msg);
    } else if (msg->arrivedOn("appIn")) {
        handleMessageFromApp(msg);
        delete msg;
    } else if (msg->arrivedOn("udpIn")) {
        handleMessageFromUdp(msg);
        delete msg;
    } else {
        throw cRuntimeError("Message arrived from unknown gate.");
    }
}

void Quic::handleTimeout(cMessage *msg)
{
    Connection *connection = (Connection *)msg->getContextPointer();
    try {
        connection->processTimeout(msg);
    } catch (ConnectionDiedException& e) {
        EV_WARN << e.what() << endl;

        AppSocket *appSocket = connection->getAppSocket();

        // delete connection
        connectionIdConnectionMap.erase(connection->getSrcConnectionIds()[0]->getId());
        delete connection;

        destroySockets(appSocket);
    }
}

void Quic::handleMessageFromApp(cMessage *msg)
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    int socketId = tags.getTag<SocketReq>()->getSocketId();

    AppSocket *appSocket = findOrCreateAppSocket(socketId);

    Connection *connection = appSocket->getConnection();
    if (connection) {
        connection->processAppCommand(msg);
    } else {
        // no connection found, process message in app socket
        appSocket->processAppCommand(msg);
        if (msg->getKind() == QUIC_C_CLOSE) {
            destroySockets(appSocket);
        }
    }
}

void Quic::handleMessageFromUdp(cMessage *msg)
{
    if (msg->getKind() == UDP_I_SOCKET_CLOSED) {
        EV_WARN << "Socket closed message received from UDP" << endl;
    } else if (msg->getKind() == UDP_I_ERROR) {
        EV_WARN << "Error message received from UDP" << endl;
        Indication *ind = check_and_cast<Indication *>(msg);
        auto quotedPacket = ind->getTag<IcmpErrorInd>()->getQuotedPacket()->dup();
        auto icmpHeader = quotedPacket->popAtFront<IcmpHeader>();
        if (icmpHeader != nullptr) {
            if (icmpHeader->getType() == ICMP_DESTINATION_UNREACHABLE && icmpHeader->getCode() == ICMP_DU_FRAGMENTATION_NEEDED) {
                // Packet Too Big (PTB) message received
                Ptr<const IcmpPtb> icmpPtb = dynamicPtrCast<const IcmpPtb>(icmpHeader);

                quotedPacket->popAtFront<Ipv4Header>(); // skip IP header of reflected packet
                quotedPacket->popAtFront<UdpHeader>(); // skip UDP header of reflected packet
                if (quotedPacket->getByteLength() > 0) {
                    bool readSrcConnectionId = true;
                    uint64_t connectionId = extractConnectionId(quotedPacket, &readSrcConnectionId);
                    Connection *connection = nullptr;
                    if (readSrcConnectionId) {
                        // could read source connection ID, finding connection should be easy
                        connection = findConnection(connectionId);
                    } else {
                        // could only read destination connection ID, try to find connection
                        connection = findConnectionByDstConnectionId(connectionId, quotedPacket);
                    }

                    if (connection) {
                        connection->processIcmpPtb(quotedPacket, icmpPtb->getMtu());
                    } else {
                        EV_WARN << "Could not find connection for a ICMP PTB" << endl;
                    }
                } else {
                    EV_WARN << "ICMP PTB message reflects not enough data from the original packet." << endl;
                }
            } else {
                EV_WARN << "ICMP types other than PTB not handled" << endl;
            }
        } else {
            EV_WARN << "ICMPv6 not handled" << endl;
        }
        delete quotedPacket;
    } else if (msg->getKind() == UDP_I_DATA) {
        Packet *pkt = check_and_cast<Packet *>(msg);
        Ptr<const EncryptedQuicPacketChunk> encPkt = pkt->popAtFront<EncryptedQuicPacketChunk>();
        auto chunks = check_and_cast<SequenceChunk *>(encPkt->getChunk().get())->getChunks();

        for (int i = chunks.size() - 1; i >= 0; i--) {
            pkt->insertAtFront(chunks[i]);
        }

        uint64_t connectionId = extractConnectionId(pkt);
        Connection *connection = findConnection(connectionId);
        if (connection) {
            connection->processPackets(pkt);
        } else {
            EV_DEBUG << "no connection found for packet, use udpSocket to process it" << endl;
            auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
            int udpSocketId = tags.getTag<SocketInd>()->getSocketId();
            UdpSocket *udpSocket = findUdpSocket(udpSocketId);

            udpSocket->processPacket(pkt);
        }
    } else {
        EV_WARN << "Message with unknown kind received from UDP" << endl;
    }
}

AppSocket *Quic::findOrCreateAppSocket(int socketId)
{
    auto it = socketIdAppSocketMap.find(socketId);
    // return appSocket, if found
    if (it != socketIdAppSocketMap.end()) {
        return it->second;
    }
    // create new appSocket
    AppSocket *appSocket = new AppSocket(this, socketId);
    socketIdAppSocketMap.insert({ socketId, appSocket });
    return appSocket;
}

Connection *Quic::findConnection(uint64_t srcConnectionId)
{
    EV_DEBUG << "find connection for ID " << srcConnectionId << endl;
    auto it = connectionIdConnectionMap.find(srcConnectionId);
    if (it == connectionIdConnectionMap.end()) {
        return nullptr;
    }
    return it->second;
}

Connection *Quic::findConnectionByDstConnectionId(uint64_t dstConnectionId, Packet *pkt)
{
    EV_DEBUG << "find connection for destination connection ID " << dstConnectionId << " and packet " << pkt << endl;
    for (std::map<uint64_t, Connection *>::iterator it = connectionIdConnectionMap.begin(); it != connectionIdConnectionMap.end(); ++it) {
        Connection *connection = it->second;
        if (connection->belongsPacketTo(pkt, dstConnectionId)) {
            return connection;
        }
    }
    return nullptr;
}

UdpSocket *Quic::findUdpSocket(int socketId)
{
    auto it = udpSocketIdUdpSocketMap.find(socketId);
    if (it == udpSocketIdUdpSocketMap.end()) {
        return nullptr;
    }
    return it->second;
}

UdpSocket *Quic::findUdpSocket(L3Address addr, uint16_t port)
{
    for (std::map<int, UdpSocket *>::iterator it = udpSocketIdUdpSocketMap.begin(); it != udpSocketIdUdpSocketMap.end(); ++it) {
        if (it->second->match(addr, port)) {
            return it->second;
        }
    }
    return nullptr;
}

void Quic::addConnection(uint64_t connectionId, Connection *connection)
{
    EV_DEBUG << "Quic::addConnection: add connection for ID " << connectionId << endl;
    auto result = connectionIdConnectionMap.insert({ connectionId, connection });
    if (!result.second) {
        throw cRuntimeError("Cannot insert connection. A connection for the connection id already exists.");
    }
}

void Quic::addConnection(Connection *connection)
{
    for (ConnectionId *srcConnectionId : connection->getSrcConnectionIds()) {
        addConnection(srcConnectionId->getId(), connection);
    }
}

void Quic::removeConnectionId(uint64_t connectionId)
{
    EV_DEBUG << "Quic::removeConnectionId: remove connection ID " << connectionId << endl;
    connectionIdConnectionMap.erase(connectionId);
}

Connection *Quic::createConnection(bool is_server, UdpSocket *udpSocket, AppSocket *appSocket, L3Address remoteAddr, uint16_t remotePort)
{
    uint64_t connectionId = nextConnectionId;
    nextConnectionId++;
    Connection *connection = new Connection(this, is_server, udpSocket, appSocket, remoteAddr, remotePort, connectionId);
    appSocket->setConnection(connection);
    addConnection(connection);
    return connection;
}

void Quic::destroySockets(AppSocket *appSocket)
{
    EV_TRACE << "enter Quic::destroySockets" << endl;

    UdpSocket *udpSocket = appSocket->getUdpSocket();

    // send destroy notification and delete AppSocket
    appSocket->sendDestroyed();
    socketIdAppSocketMap.erase(appSocket->getSocketId());
    delete appSocket;

    // destroy and delete UdpSocket if not used by another connection of app socket
    if (udpSocket != nullptr && !isUdpSocketInUse(udpSocket)) {
        udpSocket->destroy();
        udpSocketIdUdpSocketMap.erase(udpSocket->getSocketId());
        delete udpSocket;
    }

    EV_TRACE << "leave Quic::destroySockets" << endl;
}

uint64_t Quic::extractConnectionId(Packet *pkt, bool *readSourceConnectionId)
{
    auto packetHeader = pkt->peekAtFront<PacketHeader>();
    EV_DEBUG << "extract connection ID from: " << packetHeader << endl;

    switch (packetHeader->getHeaderForm()) {
        case PACKET_HEADER_FORM_LONG: {
            auto longPacketHeader = staticPtrCast<const LongPacketHeader>(packetHeader);
            if (readSourceConnectionId != nullptr && *readSourceConnectionId) {
                return longPacketHeader->getSrcConnectionId();
            }
            return longPacketHeader->getDstConnectionId();
        }
        case PACKET_HEADER_FORM_SHORT: {
            auto shortPacketHeader = staticPtrCast<const ShortPacketHeader>(packetHeader);
            if (readSourceConnectionId != nullptr) {
                *readSourceConnectionId = false;
            }
            return shortPacketHeader->getDstConnectionId();
        }
        default: {
            throw cRuntimeError("Quic::extractConnectionId: Unknown header form.");
        }
    }
}

void Quic::addUdpSocket(UdpSocket *udpSocket)
{
    auto result = udpSocketIdUdpSocketMap.insert({ udpSocket->getSocketId(), udpSocket });
    if (!result.second) {
        throw cRuntimeError("Cannot insert udp socket. A udp socket for the socket id already exists.");
    }
}

UdpSocket *Quic::createUdpSocket()
{
    UdpSocket *udpSocket = new UdpSocket(this);
    addUdpSocket(udpSocket);
    return udpSocket;
}

bool Quic::isUdpSocketInUse(UdpSocket *udpSocket)
{
    // does a connection exists that uses this udp socket?
    for (std::map<uint64_t, Connection *>::iterator it = connectionIdConnectionMap.begin(); it != connectionIdConnectionMap.end(); ++it) {
        Connection *connection = it->second;
        if (udpSocket == connection->getUdpSocket()) {
            return true;
        }
    }

    // does a app socket exists that uses this udp socket?
    // we might have an app socket with that udp socket that has not (yet) created a connection
    for (std::map<int, AppSocket *>::iterator it = socketIdAppSocketMap.begin(); it != socketIdAppSocketMap.end(); ++it) {
        AppSocket *appSocket = it->second;
        if (udpSocket == appSocket->getUdpSocket()) {
            return true;
        }
    }
    return false;
}

IRoutingTable *Quic::getRoutingTable() {
    return getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
}

} //namespace quic
} //namespace inet
