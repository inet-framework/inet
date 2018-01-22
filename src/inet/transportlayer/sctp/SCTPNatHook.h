#ifndef __INET_SCTPNATHOOK_H
#define __INET_SCTPNATHOOK_H

#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/sctp/SctpNatTable.h"
#include "inet/common/INETDefs.h"

namespace inet {

class IPv4;

namespace sctp {
#if 0
class INET_API SctpNatHook : public cSimpleModule, NetfilterBase::HookBase
{
  protected:
    IPv4 *ipLayer;    // IPv4 module
    SctpNatTable *natTable;
    IRoutingTable *rt;
    IInterfaceTable *ift;
    uint64 nattedPackets;
    void initialize() override;
    void finish() override;

  protected:
    void sendBackError(Ipv4Header *dgram);

  public:
    SctpNatHook();
    virtual ~SctpNatHook();
    IHook::Result datagramPreRoutingHook(INetworkHeader *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr) override;
    IHook::Result datagramForwardHook(INetworkHeader *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr) override;
    IHook::Result datagramPostRoutingHook(INetworkHeader *datagram, const InterfaceEntry *inIE, const InterfaceEntry *& outIE, L3Address& nextHopAddr) override;
    IHook::Result datagramLocalInHook(INetworkHeader *datagram, const InterfaceEntry *inIE) override;
    IHook::Result datagramLocalOutHook(INetworkHeader *datagram, const InterfaceEntry *& outIE, L3Address& nextHopAddr) override;
};
#endif
} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTPNATHOOK_H

