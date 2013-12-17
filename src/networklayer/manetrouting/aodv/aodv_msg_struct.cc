

#include  <string.h>

#include "aodv_msg_struct.h"
#include "InterfaceTable.h"
#include "UDPPacket.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"
#include "ICMPMessage_m.h"
#include "IPv4Datagram.h"
#include "RoutingTable.h"
#include "ICMPAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"
#include "ProtocolMap.h"



Register_Class(AODV_msg);


AODV_msg::AODV_msg(const AODV_msg& m) : cPacket(m)
{
    copy(m);
}


AODV_msg& AODV_msg::operator=(const AODV_msg& m)
{
    if (this==&m) return *this;

    clearExtension();
    cPacket::operator=(m);
    copy(m);
    return *this;
}


void AODV_msg::copy(const AODV_msg& m)
{
    type = m.type;
    ttl = m.ttl;
    prevFix = m.prevFix;
    extensionsize = m.extensionsize;
    if (extensionsize==0)
    {
        extension = NULL;
        return;
    }
    extension = new AODV_ext [extensionsize];
    for (int i=0; i < extensionsize; i++)
    {
        extension[i].type = m.extension[i].type;
        extension[i].length = m.extension[i].length;
        extension[i].pointer = new char[extension[i].length];
        memcpy(extension[i].pointer, m.extension[i].pointer, extension[i].length);
    }
}

void AODV_msg:: clearExtension()
{
    if (extensionsize==0)
    {
        return;
    }
    for (int i=0; i < extensionsize; i++)
    {
        delete extension[i].pointer;
    }
    delete [] extension;
    extensionsize = 0;
    extension = NULL;
}

AODV_msg::~AODV_msg()
{
    clearExtension();
}

AODV_ext * AODV_msg::addExtension(int type, int len, char *data)
{
    AODV_ext* extension_aux;
    if (len<0)
    {
        return NULL;
    }
    extension_aux =   new AODV_ext [extensionsize+1];
    for (int i=0; i < extensionsize; i++)
    {
        extension_aux[i].type = extension[i].type;
        extension_aux[i].length = extension[i].length;
        extension_aux[i].pointer = extension[i].pointer;
    }
    if (extensionsize>0)
        delete [] extension;
    extension =  extension_aux;
    extensionsize++;
    extension[extensionsize-1].type = type;
    extension[extensionsize-1].length = len;
    extension[extensionsize-1].pointer  =  new char[len];
    memcpy(extension_aux[extensionsize-1].pointer, data, len);
    setBitLength(getBitLength()+((AODV_EXT_HDR_SIZE+len) *8));
    return & extension[extensionsize-1];
}


AODV_ext * AODV_msg::getNexExtension(AODV_ext* aodv_ext)
{
    if ((&extension[extensionsize-1]>aodv_ext+1) && (aodv_ext >= extension))
        return aodv_ext+1;
    else
        return NULL;
}

//=== registration
Register_Class(RERR);

//=========================================================================

RERR::RERR(const RERR& m) : AODV_msg(m)
{
    copy(m);
}

RERR& RERR::operator=(const RERR& m)
{
    if (this==&m) return *this;
    clearUdest();
    AODV_msg::operator=(m);
    copy(m);
    return *this;
}

void RERR::copy(const RERR& m)
{
    n = m.n;
    res1 = m.res1;
    res2 = m.res2;
    dest_count = m.dest_count;
    _udest = new RERR_udest [dest_count];
    for (int i=0; i < dest_count; i++)
    {
        _udest[i].dest_addr = m._udest[i].dest_addr;
        _udest[i].dest_seqno = m._udest[i].dest_seqno;
    }
}

RERR::~RERR()
{
    clearUdest();
}

void RERR::addUdest(const ManetAddress & src_addr, unsigned int udest_seqno)
{

    RERR_udest *temp_udest;
    temp_udest = new RERR_udest [dest_count+1];
    for (int i=0; i < dest_count; i++)
    {
        temp_udest[i].dest_addr = _udest[i].dest_addr;
        temp_udest[i].dest_seqno = _udest[i].dest_seqno;
    }
    delete []  _udest;
    temp_udest[dest_count].dest_addr = src_addr;
    temp_udest[dest_count].dest_seqno = udest_seqno;
    _udest = temp_udest;
    dest_count++;
    setBitLength(getBitLength()+(RERR_UDEST_SIZE));
}

RERR_udest * RERR::getUdest(int i)
{
    if (i < dest_count)
    {
        return  &(_udest[i]);
    }
    else
    {
        return NULL;
    }
}

void RERR::clearUdest()
{
    if (_udest!=NULL)
    {
        delete []  _udest;
    }
    _udest = NULL;
}


Register_Class(RREP);
RREP::RREP(const RREP& m) : AODV_msg(m)
{
    copy(m);
}

RREP& RREP::operator=(const RREP& m)
{
    if (this==&m) return *this;
    AODV_msg::operator=(m);
    copy(m);
    return *this;
}

void RREP::copy(const RREP& m)
{
    res1 = m.res1;
    a = m.a;
    r = m.r;
    prefix = m.prefix;
    res2 = m.res2;
    hcnt = m.hcnt;
    dest_addr = m.dest_addr;
    dest_seqno = m.dest_seqno;
    orig_addr = m.orig_addr;
    lifetime = m.lifetime;
    cost = m.cost;
    hopfix = m.hopfix;
    totalHops = m.totalHops;
}

std::string RREP::detailedInfo() const
{
    std::stringstream out;
    int timeToLive = ttl;
    int hops = hcnt;
    out << " RREP "  << "\n"; // Khmm...
    out <<" Source :"<< orig_addr << "\n";
    out <<" Destination :"<< dest_addr << "\n";
    out <<" Destination seq num:"<< dest_seqno << "\n";
    out <<" TTL :"<< timeToLive << "\n";
    out <<" hops :"<< hops << "\n";
    return out.str();
}



Register_Class(RREP_ack);
RREP_ack::RREP_ack(const RREP_ack& m) : AODV_msg(m)
{
    copy(m);
}

RREP_ack& RREP_ack::operator=(const RREP_ack& m)
{
    if (this==&m) return *this;
    AODV_msg::operator=(m);
    copy(m);
    return *this;
}



Register_Class(RREQ);
RREQ::RREQ(const RREQ& m) : AODV_msg(m)
{
    copy(m);
}

RREQ& RREQ::operator=(const RREQ& m)
{
    if (this==&m) return *this;
    AODV_msg::operator=(m);
    copy(m);
    return *this;
}

void RREQ::copy(const RREQ& m)
{
    j = m.j;      /* Join flag (multicast) */
    r = m.r;      /* Repair flag */
    g = m.g;      /* Gratuitous RREP flag */
    d = m.d;      /* Destination only respond */
    res1 = m.res1;
    res2 = m.res2;
    hcnt = m.hcnt;
    rreq_id = m.rreq_id;
    dest_addr = m.dest_addr;
    dest_seqno = m.dest_seqno;
    orig_addr = m.orig_addr;
    orig_seqno = m.orig_seqno;
    cost = m.cost;
    hopfix = m.hopfix;
}

std::string RREQ::detailedInfo() const
{
    std::stringstream out;
    int timeToLive = ttl;
    int hops = hcnt;
    out << " RREQ "  << "\n"; // Khmm...
    out <<" Source :"<< orig_addr << "\n";
    out <<" Source seq num:"<< orig_seqno << "\n";
    out <<" Destination :"<< dest_addr << "\n";
    out <<" Destination seq num:"<< dest_seqno << "\n";
    out <<" TTL :"<< timeToLive << "\n";
    out <<" hops :"<< hops << "\n";
    return out.str();
}


