#ifndef __INET_SCTPNATHOOK_H
#define __INET_SCTPNATHOOK_H

#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/sctp/SctpNatTable.h"
#include "inet/common/INETDefs.h"

namespace inet {

class IPv4;

namespace sctp {

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
   // void sendBackError(const Ptr<const Ipv4Header>& dgram, SctpHeader* sctp);
    void sendBackError(SctpHeader* sctp);

  public:
    SctpNatHook();
    virtual ~SctpNatHook();
    virtual Result datagramPreRoutingHook(Packet *packet) override;
    virtual Result datagramForwardHook(Packet *packet) override;
    virtual Result datagramPostRoutingHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramLocalInHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *packet) override { return ACCEPT; }
};

} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTPNATHOOK_H

