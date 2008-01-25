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
#include "InterfaceIdentifier.h"


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
class INET_API InterfaceEntry : public cPolymorphic
{
    friend class InterfaceTable; //only this guy is allowed to set _interfaceId

  private:
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

  public:
    InterfaceEntry();
    virtual ~InterfaceEntry() {}
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    int interfaceId() const        {return _interfaceId;}
    const char *name() const       {return _name.c_str();}
    int networkLayerGateIndex() const {return _nwLayerGateIndex;}
    int nodeOutputGateId() const   {return _nodeOutputGateId;}
    int nodeInputGateId() const    {return _nodeInputGateId;}
    int peerNamId() const          {return _peernamid;}
    int mtu() const                {return _mtu;}
    bool isDown() const            {return _down;}
    bool isBroadcast() const       {return _broadcast;}
    bool isMulticast() const       {return _multicast;}
    bool isPointToPoint() const    {return _pointToPoint;}
    bool isLoopback() const        {return _loopback;}
    double datarate() const        {return _datarate;}
    const MACAddress& macAddress() const  {return _macAddr;}
    const InterfaceToken& interfaceToken() const {return _token;}//FIXME: Shouldn't this be interface identifier?

    void setName(const char *s)  {_name = s;}
    void setNetworkLayerGateIndex(int i) {_nwLayerGateIndex = i;}
    void setNodeOutputGateId(int i) {_nodeOutputGateId = i;}
    void setNodeInputGateId(int i)  {_nodeInputGateId = i;}
    void setPeerNamId(int ni)    {_peernamid = ni;}
    void setMtu(int m)           {_mtu = m;}
    void setDown(bool b)         {_down = b;}
    void setBroadcast(bool b)    {_broadcast = b;}
    void setMulticast(bool b)    {_multicast = b;}
    void setPointToPoint(bool b) {_pointToPoint = b;}
    void setLoopback(bool b)     {_loopback = b;}
    void setDatarate(double d)   {_datarate = d;}
    void setMACAddress(const MACAddress& macAddr) {_macAddr=macAddr;}
    void setInterfaceToken(const InterfaceToken& token) {_token=token;}

    IPv4InterfaceData *ipv4()    {return _ipv4data;}
    IPv6InterfaceData *ipv6()    {return _ipv6data;}
    cPolymorphic *protocol3()    {return _protocol3data;}
    cPolymorphic *protocol4()    {return _protocol4data;}

    void setIPv4Data(IPv4InterfaceData *p)  {_ipv4data = p;}
    void setIPv6Data(IPv6InterfaceData *p)  {_ipv6data = p;}
    void setProtocol3Data(cPolymorphic *p)  {_protocol3data = p;}
    void setProtocol4Data(cPolymorphic *p)  {_protocol4data = p;}
};

#endif

