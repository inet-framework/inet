//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IPSEC_H_
#define __INET_IPSEC_H_

#include "inet/common/INETDefs.h"
#include "inet/common/SimpleModule.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "SecurityPolicy.h"
#include "SecurityAssociation.h"
#include "inet/networklayer/ipv4/ipsec/SecurityAssociationDatabase.h"
#include "inet/networklayer/ipv4/ipsec/SecurityPolicyDatabase.h"

namespace inet {
namespace ipsec {

/**
 * Implements IPsec for IPv4. Supports AH and ESP in transport mode.
 * See the NED documentation for more info.
 */
class INET_API IPsec : public SimpleModule, NetfilterBase::HookBase
{
  public:
    typedef IPsecRule::Action Action;
    typedef IPsecRule::Direction Direction;
    typedef IPsecRule::Protection Protection;
    typedef IPsecRule::EspMode EspMode;
    typedef IPsecRule::EncryptionAlg EncryptionAlg;
    typedef IPsecRule::AuthenticationAlg AuthenticationAlg;

  private:
    Ipv4 *ipLayer;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    SecurityPolicyDatabase *spdModule;
    SecurityAssociationDatabase *sadModule;

    cPar *ahProtectOutDelay;
    cPar *ahProtectInDelay;

    cPar *espProtectOutDelay;
    cPar *espProtectInDelay;

    simtime_t lastProtectedIn = 0;
    simtime_t lastProtectedOut = 0;

    int inAccept = 0;
    int inDrop = 0;
    int inBypass = 0;

    int outProtect = 0;
    int outDrop = 0;
    int outBypass = 0;

  protected:
    static simsignal_t inProtectedAcceptSignal;
    static simsignal_t inProtectedDropSignal;
    static simsignal_t inUnprotectedBypassSignal;
    static simsignal_t inUnprotectedDropSignal;

    static simsignal_t outBypassSignal;
    static simsignal_t outProtectSignal;
    static simsignal_t outDropSignal;

    static simsignal_t inProcessDelaySignal;
    static simsignal_t outProcessDelaySignal;

  private:
    virtual void initSecurityDBs(omnetpp::cXMLElement *spdConfig);
    virtual void parseSelector(const cXMLElement *selectorElem, PacketSelector& selector);
    static unsigned int parseProtocol(const std::string& value);
    template<typename E>
    static E parseEnumElem(const Enum<E>& enum_, const cXMLElement *parentElem, const char *childElemName, E defaultValue=(E)-1, E defaultValue2=(E)-1);

    virtual PacketInfo extractIngressPacketInfo(Packet *packet);
    virtual PacketInfo extractEgressPacketInfo(Packet *ipv4datagram, const Ipv4Address& localAddress);

    virtual void espProtect(Packet *transport, SecurityAssociation *sadEntry, int transportType, bool tfcEnabled);
    virtual void ahProtect(Packet *transport, SecurityAssociation *sadEntry, int transportType);

    virtual INetfilter::IHook::Result protectDatagram(Packet *ipv4datagram, const PacketInfo& packetInfo, SecurityPolicy *spdEntry);

    virtual int getIntegrityCheckValueBitLength(EncryptionAlg alg);
    virtual int getInitializationVectorBitLength(EncryptionAlg alg);
    virtual int getBlockSizeBytes(EncryptionAlg alg);

    virtual int getIntegrityCheckValueBitLength(AuthenticationAlg alg);
    virtual int getInitializationVectorBitLength(AuthenticationAlg alg);

  public:
    IPsec();
    virtual ~IPsec();

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void refreshDisplay() const override;

    INetfilter::IHook::Result processEgressPacket(Packet *ipv4datagram, const Ipv4Address& localAddress);
    INetfilter::IHook::Result processIngressPacket(Packet *ipv4datagram);

    /* Netfilter hooks */
    virtual IHook::Result datagramPreRoutingHook(Packet *datagram) override;
    virtual IHook::Result datagramForwardHook(Packet *datagram) override;
    virtual IHook::Result datagramPostRoutingHook(Packet *datagram) override;
    virtual IHook::Result datagramLocalInHook(Packet *datagram) override;
    virtual IHook::Result datagramLocalOutHook(Packet *datagram) override;
};

}    // namespace ipsec
}    // namespace inet

#endif    // ifndef __INET_IPSEC_H_

