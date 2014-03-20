#ifndef __INET_SCTPNATHOOK_H
#define __INET_SCTPNATHOOK_H

#include "INetfilter.h"
#include "SCTPNatTable.h"
#include "INETDefs.h"

class IPv4;

class INET_API SCTPNatHook : public cSimpleModule, INetfilter::IHook
{
    protected:
        IPv4 *ipLayer;      // IPv4 module
        SCTPNatTable* natTable;
        IRoutingTable *rt;
        IInterfaceTable *ift;
        uint64 nattedPackets;
        void initialize();
        void finish();

    protected:
      void sendBackError(IPv4Datagram* dgram);

    public:
      SCTPNatHook();
      virtual ~SCTPNatHook();
      IHook::Result datagramPreRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      IHook::Result datagramForwardHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      IHook::Result datagramPostRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, Address& nextHopAddr);
      IHook::Result datagramLocalInHook(INetworkDatagram* datagram, const InterfaceEntry* inIE);
      IHook::Result datagramLocalOutHook(INetworkDatagram* datagram, const InterfaceEntry*& outIE, Address& nextHopAddr);
};

#endif
