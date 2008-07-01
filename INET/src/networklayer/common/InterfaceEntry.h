//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INTERFACEENTRY_H
#define __INTERFACEENTRY_H

#include <vector>
#include <omnetpp.h>
#include "INETDefs.h"
#include "MACAddress.h"
#include "InterfaceToken.h"


// Forward declarations. Do NOT #include the corresponding header files
// since that would create dependence on IPv4 and IPv6 stuff!
class IPv4InterfaceData;
class IPv6InterfaceData;
class InterfaceTable;

/**
 * Interface entry for the interface table in InterfaceTable.
 *
 * @see InterfaceTable
 */
//FIXME remove interfaceId? pointer or name can be used instead
//FIXME use cNamedObject as base class (instead of separate ifname?)
class INET_API InterfaceEntry : public cObject
{
    friend class InterfaceTable; //only this guy is allowed to set interfaceId and owner

  protected:
    InterfaceTable *ownerp; ///< InterfaceTable that contains this interface, or NULL
    int interfaceId;      ///< identifies the interface in the InterfaceTable
    std::string ifname;   ///< interface name (must be unique)
    int nwLayerGateIndex; ///< index of ifIn[],ifOut[] gates to that interface (or -1 if virtual interface)
    int nodeOutputGateId; ///< id of the output gate of this host/router (or -1 if this is a virtual interface)
    int nodeInputGateId;  ///< id of the input gate of this host/router (or -1 if this is a virtual interface)
    int peernamid;        ///< used only when writing ns2 nam traces
    int mtu;              ///< Maximum Transmission Unit (e.g. 1500 on Ethernet)
    bool down;            ///< current state (up or down)
    bool broadcast;       ///< interface supports broadcast
    bool multicast;       ///< interface supports multicast
    bool pointToPoint;    ///< interface is point-to-point link
    bool loopback;        ///< interface is loopback interface
    double datarate;      ///< data rate in bit/s
    MACAddress macAddr;   ///< link-layer address (for now, only IEEE 802 MAC addresses are supported)
    InterfaceToken token; ///< for IPv6 stateless autoconfig (RFC 1971)

    IPv4InterfaceData *ipv4data;   ///< IPv4-specific interface info (IP address, etc)
    IPv6InterfaceData *ipv6data;   ///< IPv6-specific interface info (IPv6 addresses, etc)
    cPolymorphic *protocol3data;   ///< extension point: data for a 3rd network-layer protocol
    cPolymorphic *protocol4data;   ///< extension point: data for a 4th network-layer protocol

  private:
    // copying not supported: following are private and also left undefined
    InterfaceEntry(const InterfaceEntry& obj);
    InterfaceEntry& operator=(const InterfaceEntry& obj);

  protected:
    // change notifications
    virtual void configChanged();
    virtual void stateChanged();

    // to be invoked from InterfaceTable only
    virtual void setInterfaceTable(InterfaceTable *t) {ownerp = t;}

  public:
    InterfaceEntry();
    virtual ~InterfaceEntry() {}
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    /**
     * Returns the InterfaceTable this interface is in, or NULL
     */
    InterfaceTable *getInterfaceTable() const {return ownerp;}

    /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
    //@{
    int getInterfaceId() const        {return interfaceId;}   //FIXME remove on the long term! (clients should use interface pointer)
    const char *getName() const       {return ifname.c_str();}
    int getNetworkLayerGateIndex() const {return nwLayerGateIndex;}
    int getNodeOutputGateId() const   {return nodeOutputGateId;}
    int getNodeInputGateId() const    {return nodeInputGateId;}
    int getPeerNamId() const          {return peernamid;}
    int getMTU() const                {return mtu;}
    bool isDown() const               {return down;}
    bool isBroadcast() const          {return broadcast;}
    bool isMulticast() const          {return multicast;}
    bool isPointToPoint() const       {return pointToPoint;}
    bool isLoopback() const           {return loopback;}
    double getDatarate() const        {return datarate;}
    const MACAddress& getMacAddress() const  {return macAddr;}
    const InterfaceToken& getInterfaceToken() const {return token;}
    //@}

    /** @name Field setters */
    //@{
    virtual void setName(const char *s)  {ifname = s; configChanged();}
    virtual void setNetworkLayerGateIndex(int i) {nwLayerGateIndex = i; configChanged();}
    virtual void setNodeOutputGateId(int i) {nodeOutputGateId = i; configChanged();}
    virtual void setNodeInputGateId(int i)  {nodeInputGateId = i; configChanged();}
    virtual void setPeerNamId(int ni)    {peernamid = ni; configChanged();}
    virtual void setMtu(int m)           {mtu = m; configChanged();}
    virtual void setDown(bool b)         {down = b; stateChanged();}
    virtual void setBroadcast(bool b)    {broadcast = b; configChanged();}
    virtual void setMulticast(bool b)    {multicast = b; configChanged();}
    virtual void setPointToPoint(bool b) {pointToPoint = b; configChanged();}
    virtual void setLoopback(bool b)     {loopback = b; configChanged();}
    virtual void setDatarate(double d)   {datarate = d; configChanged();}
    virtual void setMACAddress(const MACAddress& addr) {macAddr = addr; configChanged();}
    virtual void setInterfaceToken(const InterfaceToken& t) {token = t; configChanged();}
    //@}

    /** @name Accessing protocol-specific interface data. Note methods are non-virtual, for performance reasons. */
    //@{
    IPv4InterfaceData *ipv4()       {return ipv4data;}
    IPv6InterfaceData *ipv6()       {return ipv6data;}
    cPolymorphic *getProtocol3()    {return protocol3data;}
    cPolymorphic *getProtocol4()    {return protocol4data;}
    //@}

    /** @name Installing protocol-specific interface data */
    //@{
    virtual void setIPv4Data(IPv4InterfaceData *p)  {ipv4data = p; configChanged();}
    virtual void setIPv6Data(IPv6InterfaceData *p)  {ipv6data = p; configChanged();}
    virtual void setProtocol3Data(cPolymorphic *p)  {protocol3data = p; configChanged();}
    virtual void setProtocol4Data(cPolymorphic *p)  {protocol4data = p; configChanged();}
    //@}
};

#endif

