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
class INET_API InterfaceEntry : public cObject
{
    friend class InterfaceTable; //only this guy is allowed to set _interfaceId and owner

  protected:
    InterfaceTable *ownerp; ///< InterfaceTable that contains this interface, or NULL
    int _interfaceId;      ///< identifies the interface in the InterfaceTable
    std::string _name;     ///< interface name (must be unique)
    int _nwLayerGateIndex; ///< index of ifIn[],ifOut[] gates to that interface (or -1 if virtual interface)
    int _nodeOutputGateId; ///< id of the output gate of this host/router (or -1 if this is a virtual interface)
    int _nodeInputGateId;  ///< id of the input gate of this host/router (or -1 if this is a virtual interface)
    int _peernamid;        ///< used only when writing ns2 nam traces
    int _mtu;              ///< Maximum Transmission Unit (e.g. 1500 on Ethernet)
    bool _down;            ///< current state (up or down)
    bool _broadcast;       ///< interface supports broadcast
    bool _multicast;       ///< interface supports multicast
    bool _pointToPoint;    ///< interface is point-to-point link
    bool _loopback;        ///< interface is loopback interface
    double _datarate;      ///< data rate in bit/s
    MACAddress _macAddr;   ///< link-layer address (for now, only IEEE 802 MAC addresses are supported)
    InterfaceToken _token; ///< for IPv6 stateless autoconfig (RFC 1971)

    IPv4InterfaceData *_ipv4data;   ///< IPv4-specific interface info (IP address, etc)
    IPv6InterfaceData *_ipv6data;   ///< IPv6-specific interface info (IPv6 addresses, etc)
    cPolymorphic *_protocol3data;   ///< extension point: data for a 3rd network-layer protocol
    cPolymorphic *_protocol4data;   ///< extension point: data for a 4th network-layer protocol

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
    int getInterfaceId() const        {return _interfaceId;}   //FIXME remove on the long term! (clients should use interface pointer)
    const char *getName() const       {return _name.c_str();}
    int getNetworkLayerGateIndex() const {return _nwLayerGateIndex;}
    int getNodeOutputGateId() const   {return _nodeOutputGateId;}
    int getNodeInputGateId() const    {return _nodeInputGateId;}
    int getPeerNamId() const          {return _peernamid;}
    int getMTU() const                {return _mtu;}
    bool isDown() const               {return _down;}
    bool isBroadcast() const          {return _broadcast;}
    bool isMulticast() const          {return _multicast;}
    bool isPointToPoint() const       {return _pointToPoint;}
    bool isLoopback() const           {return _loopback;}
    double getDatarate() const        {return _datarate;}
    const MACAddress& getMacAddress() const  {return _macAddr;}
    const InterfaceToken& getInterfaceToken() const {return _token;}
    //@}

    /** @name Field setters */
    //@{
    virtual void setName(const char *s)  {_name = s; configChanged();}
    virtual void setNetworkLayerGateIndex(int i) {_nwLayerGateIndex = i; configChanged();}
    virtual void setNodeOutputGateId(int i) {_nodeOutputGateId = i; configChanged();}
    virtual void setNodeInputGateId(int i)  {_nodeInputGateId = i; configChanged();}
    virtual void setPeerNamId(int ni)    {_peernamid = ni; configChanged();}
    virtual void setMtu(int m)           {_mtu = m; configChanged();}
    virtual void setDown(bool b)         {_down = b; stateChanged();}
    virtual void setBroadcast(bool b)    {_broadcast = b; configChanged();}
    virtual void setMulticast(bool b)    {_multicast = b; configChanged();}
    virtual void setPointToPoint(bool b) {_pointToPoint = b; configChanged();}
    virtual void setLoopback(bool b)     {_loopback = b; configChanged();}
    virtual void setDatarate(double d)   {_datarate = d; configChanged();}
    virtual void setMACAddress(const MACAddress& macAddr) {_macAddr=macAddr; configChanged();}
    virtual void setInterfaceToken(const InterfaceToken& token) {_token=token; configChanged();}
    //@}

    /** @name Accessing protocol-specific interface data. Note methods are non-virtual, for performance reasons. */
    //@{
    IPv4InterfaceData *ipv4()       {return _ipv4data;}
    IPv6InterfaceData *ipv6()       {return _ipv6data;}
    cPolymorphic *getProtocol3()    {return _protocol3data;}
    cPolymorphic *getProtocol4()    {return _protocol4data;}
    //@}

    /** @name Installing protocol-specific interface data */
    //@{
    virtual void setIPv4Data(IPv4InterfaceData *p)  {_ipv4data = p; configChanged();}
    virtual void setIPv6Data(IPv6InterfaceData *p)  {_ipv6data = p; configChanged();}
    virtual void setProtocol3Data(cPolymorphic *p)  {_protocol3data = p; configChanged();}
    virtual void setProtocol4Data(cPolymorphic *p)  {_protocol4data = p; configChanged();}
    //@}
};

#endif

