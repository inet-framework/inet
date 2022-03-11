//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2011 Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_UDP_H
#define __INET_UDP_H

#include <list>
#include <map>

#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/base/TransportProtocolBase.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"

namespace inet {

class IInterfaceTable;
class Icmp;
class Icmpv6;
class UdpHeader;
class NetworkInterface;

const bool DEFAULT_MULTICAST_LOOP = true;
const uint16_t UDP_MAX_MESSAGE_SIZE = 65535; // bytes

/**
 * Implements the Udp protocol: encapsulates/decapsulates user data into/from Udp.
 *
 * More info in the NED file.
 */
class INET_API Udp : public TransportProtocolBase
{
  public:
    class CrcInsertion : public NetfilterBase::HookBase {
      public:
        virtual Result datagramPreRoutingHook(Packet *packet) override { return ACCEPT; }
        virtual Result datagramForwardHook(Packet *packet) override { return ACCEPT; }
        virtual Result datagramPostRoutingHook(Packet *packet) override;
        virtual Result datagramLocalInHook(Packet *packet) override { return ACCEPT; }
        virtual Result datagramLocalOutHook(Packet *packet) override { return ACCEPT; }
    };

    enum PortRange {
        EPHEMERAL_PORTRANGE_START = 1024,
        EPHEMERAL_PORTRANGE_END   = 5000
    };

    cMessage *uselessMsg = nullptr;

    struct MulticastMembership
    {
        L3Address multicastAddress;
        int interfaceId = -1;    // -1 = all
        UdpSourceFilterMode filterMode = static_cast<UdpSourceFilterMode>(0);
        std::vector<L3Address> sourceList;

        bool isSourceAllowed(L3Address sourceAddr);
    };

    // For a given multicastAddress and interfaceId there is at most one membership record.
    // Records are ordered first by multicastAddress, then by interfaceId (-1 interfaceId is the last)
    typedef std::vector<MulticastMembership *> MulticastMembershipTable;

    struct SockDesc
    {
        SockDesc(int sockId);
        ~SockDesc();
        int sockId = -1;
        bool isBound = false;
        bool onlyLocalPortIsSet = false;
        bool reuseAddr = false;
        L3Address localAddr;
        L3Address remoteAddr;
        int localPort = -1;
        int remotePort = -1;
        bool isBroadcast = false;
        int multicastOutputInterfaceId = -1;
        bool multicastLoop = DEFAULT_MULTICAST_LOOP;
        int ttl = -1;
        short dscp = -1;
        short tos = -1;
        MulticastMembershipTable multicastMembershipTable;

        MulticastMembershipTable::iterator findFirstMulticastMembership(const L3Address& multicastAddress);
        MulticastMembership *findMulticastMembership(const L3Address& multicastAddress, int interfaceId);
        void addMulticastMembership(MulticastMembership *membership);
        void deleteMulticastMembership(MulticastMembership *membership);
    };

    friend std::ostream& operator<<(std::ostream& os, const Udp::SockDesc& sd);

    typedef std::list<SockDesc *> SockDescList;    // might contain duplicated local addresses if their reuseAddr flag is set
    typedef std::map<int, SockDesc *> SocketsByIdMap;
    typedef std::map<int, SockDescList> SocketsByPortMap;

  protected:
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    CrcInsertion crcInsertion;

    // sockets
    SocketsByIdMap socketsByIdMap;
    SocketsByPortMap socketsByPortMap;

    // other state vars
    ushort lastEphemeralPort = EPHEMERAL_PORTRANGE_START;
    IInterfaceTable *ift = nullptr;
    Icmp *icmp = nullptr;
    Icmpv6 *icmpv6 = nullptr;

    // statistics
    int numSent = 0;
    int numPassedUp = 0;
    int numDroppedWrongPort = 0;
    int numDroppedBadChecksum = 0;

  protected:
    // utility: show current statistics above the icon
    virtual void refreshDisplay() const override;

    // socket handling
    virtual SockDesc *getSocketById(int sockId);
    virtual SockDesc *getOrCreateSocket(int sockId);
    virtual SockDesc *createSocket(int sockId, const L3Address& localAddr, int localPort);
    virtual void bind(int sockId, int gateIndex, const L3Address& localAddr, int localPort);
    virtual void connect(int sockId, int gateIndex, const L3Address& remoteAddr, int remotePort);
    virtual void close(int sockId);
    virtual void destroySocket(int sockId);
    void destroySocket(SocketsByIdMap::iterator it);
    virtual void clearAllSockets();
    virtual void setTimeToLive(SockDesc *sd, int ttl);
    virtual void setDscp(SockDesc *sd, short dscp);
    virtual void setTos(SockDesc *sd, short tos);
    virtual void setBroadcast(SockDesc *sd, bool broadcast);
    virtual void setMulticastOutputInterface(SockDesc *sd, int interfaceId);
    virtual void setMulticastLoop(SockDesc *sd, bool loop);
    virtual void setReuseAddress(SockDesc *sd, bool reuseAddr);
    virtual void joinMulticastGroups(SockDesc *sd, const std::vector<L3Address>& multicastAddresses, const std::vector<int> interfaceIds);
    virtual void leaveMulticastGroups(SockDesc *sd, const std::vector<L3Address>& multicastAddresses);
    virtual void blockMulticastSources(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList);
    virtual void unblockMulticastSources(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList);
    virtual void joinMulticastSources(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList);
    virtual void leaveMulticastSources(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, const std::vector<L3Address>& sourceList);
    virtual void setMulticastSourceFilter(SockDesc *sd, NetworkInterface *ie, L3Address multicastAddress, UdpSourceFilterMode filterMode, const std::vector<L3Address>& sourceList);

    virtual void addMulticastAddressToInterface(NetworkInterface *ie, const L3Address& multicastAddr);

    // ephemeral port
    virtual ushort getEphemeralPort();

    virtual SockDesc *findSocketForUnicastPacket(const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort);
    virtual std::vector<SockDesc *> findSocketsForMcastBcastPacket(const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort, bool isMulticast, bool isBroadcast);
    virtual SockDesc *findFirstSocketByLocalAddress(const L3Address& localAddr, ushort localPort);
    virtual void sendUp(Ptr<const UdpHeader>& header, Packet *payload, SockDesc *sd, ushort srcPort, ushort destPort);
    virtual void processUndeliverablePacket(Packet *udpPacket);
    virtual void sendUpErrorIndication(SockDesc *sd, const L3Address& localAddr, ushort localPort, const L3Address& remoteAddr, ushort remotePort);

    // process an ICMP error packet
    virtual void processICMPv4Error(Packet *icmpPacket);
    virtual void processICMPv6Error(Packet *icmpPacket);

    // process Udp packets coming from IP
    virtual void processUDPPacket(Packet *udpPacket);

    virtual void handleSelfMessage(cMessage *message) override;

    // process packets from application
    virtual void handleUpperPacket(Packet *appData) override;

    // process packets from network layr
    virtual void handleLowerPacket(Packet *appData) override;

    // process commands from application
    virtual void handleUpperCommand(cMessage *msg) override;

    // create a blank Udp packet; override to subclass UdpHeader
    virtual UdpHeader *createUDPPacket();

    // ILifeCycle:
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    // crc
    static void insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<UdpHeader>& udpHeader, Packet *udpPayload);
    static bool verifyCrc(const Protocol *networkProtocol, const Ptr<const UdpHeader>& udpHeader, Packet *packet);
    static uint16_t computeCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const UdpHeader>& udpHeader, const Ptr<const Chunk>& udpData);

  public:
    Udp();
    virtual ~Udp();

    static bool isCorrectPacket(Packet *packet, const Ptr<const UdpHeader>& udpHeader);

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

private:
    void udpConnect(omnetpp::cMessage *msg);
};

} // namespace inet

#endif // ifndef __INET_UDP_H

