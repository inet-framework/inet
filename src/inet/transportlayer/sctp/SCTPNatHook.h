#ifndef __INET_SCTPNATHOOK_H
#define __INET_SCTPNATHOOK_H

#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/sctp/SCTPNatTable.h"
#include "inet/common/INETDefs.h"

namespace inet {

class IPv4;

namespace sctp {

class INET_API SCTPNatHook : public cSimpleModule, INetfilter::IHook
{
  protected:
    IPv4 *ipLayer;    // IPv4 module
    SCTPNatTable *natTable;
    IRoutingTable *rt;
    IInterfaceTable *ift;
    uint64 nattedPackets;
    void initialize() override;
    void finish() override;

  protected:
    void sendBackError(IPv4Datagram *dgram);

  public:
    SCTPNatHook();
    virtual ~SCTPNatHook();
    IHook::Result datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr) override;
    IHook::Result datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr) override;
    IHook::Result datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr) override;
    IHook::Result datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inIE) override;
    IHook::Result datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outIE, L3Address& nextHopAddr) override;
};

} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTPNATHOOK_H

