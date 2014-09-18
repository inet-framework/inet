/*****************************************************************************
 *
 * Copyright (C) 2002 Uppsala University.
 * Copyright (C) 2007 Malaga University.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Erik Nordstr� <erik.nordstrom@it.uu.se>
 * Authors: Alfonso Ariza Quintana.<aarizaq@uma.ea>
 *
 *****************************************************************************/
#include "inet/routing/extras/dsr/dsr-pkt_omnet.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"

namespace inet {

namespace inetmanet {

#define DSR_RREQ_HDR_LEN sizeof(struct dsr_rreq_opt)
#define DSR_RREQ_OPT_LEN (DSR_RREQ_HDR_LEN - 2)
#define DSR_RREQ_TOT_LEN IP_HDR_LEN + sizeof(struct dsr_opt_hdr) + sizeof(struct dsr_rreq_opt)
#define DSR_RREQ_ADDRS_LEN(rreq_opt) (rreq_opt->length - 6)

struct dsr_rreq_opt
{
    u_int8_t type;
    u_int8_t length;
    u_int16_t id;
    u_int32_t target;
    u_int32_t addrs[0];
};

struct dsr_srt_opt
{
    u_int8_t type;
    u_int8_t length;
#if defined(__LITTLE_ENDIAN_BITFIELD)
    /* TODO: Fix bit/byte order */
    u_int16_t f:1;
    u_int16_t l:1;
    u_int16_t res:4;
    u_int16_t salv:4;
    u_int16_t sleft:6;
#elif defined (__BIG_ENDIAN_BITFIELD)
    u_int16_t f:1;
    u_int16_t l:1;
    u_int16_t res:4;
    u_int16_t salv:4;
    u_int16_t sleft:6;
#endif
    u_int32_t addrs[0];
};

#define SIZE_COST_BITS 16
DSRPkt::~DSRPkt()
{
    clean();
}

void DSRPkt::clean()
{
    if (this->options)
        delete [] this->options;
    if (costVectorSize>0)
        delete [] costVector;
    this->options = NULL;
    this->costVectorSize = 0;
}

DSRPkt::DSRPkt(const DSRPkt& m) : IPv4Datagram(m)
{

    costVector = NULL;
    options = NULL;
    costVectorSize = 0;
    copy(m);
}

DSRPkt& DSRPkt::operator=(const DSRPkt& m)
{
    if (this==&m) return *this;
    clean();
    IPv4Datagram::operator=(m);
    copy(m);
    return *this;
}

void DSRPkt::copy(const DSRPkt& m)
{
    encap_protocol = m.encap_protocol;
    previous = m.previous;
    next = m.next;

    struct dsr_opt_hdr *opth;

    opth = m.options;

    int dsr_opts_len = opth->p_len + DSR_OPT_HDR_LEN;

    options = (struct dsr_opt_hdr *) new char[dsr_opts_len];

    memcpy((char*)options, (char*)m.options, dsr_opts_len);

    costVectorSize = m.costVectorSize;
    if (m.costVectorSize>0)
    {
        costVector = new EtxCost[m.costVectorSize];
        memcpy((char*)costVector, (char*)m.costVector, m.costVectorSize*sizeof(EtxCost));
    }
}
// Constructor
DSRPkt::DSRPkt(struct dsr_pkt *dp, int interface_id) : IPv4Datagram()
{
    costVectorSize = 0;
    costVector = NULL;
    options = NULL;


    setEncapProtocol((IPProtocolId)0);

    if (dp)
    {
        IPv4Address destAddress_var((uint32_t)dp->dst.s_addr);
        setDestAddress(destAddress_var);
        IPv4Address srcAddress_var((uint32_t)dp->src.s_addr);
        setSrcAddress(srcAddress_var);
        setHeaderLength(dp->nh.iph->ihl); // Header length
        setVersion(dp->nh.iph->version); // Ip version
        setTypeOfService(dp->nh.iph->tos); // ToS
        setIdentification(dp->nh.iph->id); // Identification
        setMoreFragments(dp->nh.iph->frag_off & 0x2000);
        setDontFragment(dp->nh.iph->frag_off & 0x4000);
        setTimeToLive(dp->nh.iph->ttl); // TTL
        setTransportProtocol(IP_PROT_DSR); // Transport protocol
        setBitLength(getHeaderLength()*8);
        // ¿como gestionar el MAC
        // dp->mac.raw = p->access(hdr_mac::offset_);

        struct dsr_opt_hdr *opth;
        opth = dp->dh.opth;
        int dsr_opts_len = opth->p_len + DSR_OPT_HDR_LEN;

        options = (dsr_opt_hdr *)new char[dsr_opts_len];

        memcpy((char*)options, (char*)opth, dsr_pkt_opts_len(dp));
        setBitLength(getBitLength()+((DSR_OPT_HDR_LEN+options->p_len)*8));
        setHeaderLength(getByteLength());
#ifdef NEWFRAGMENT
        setTotalPayloadLength(dp->totalPayloadLength);
#endif
        if (dp->payload)
        {
            encapsulate(dp->payload);
            dp->payload = NULL;
            setEncapProtocol((IPProtocolId)dp->encapsulate_protocol);

        }
        if (interface_id>=0)
        {
            IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
            //ipControlInfo->setProtocol(IP_PROT_UDP);
            ipControlInfo->setProtocol(IP_PROT_DSR);
            ipControlInfo->setInterfaceId(interface_id); // If broadcast packet send to interface
            ipControlInfo->setSrcAddr(srcAddress_var);
            ipControlInfo->setDestAddr(destAddress_var);
            ipControlInfo->setTimeToLive(dp->nh.iph->ttl);
            setControlInfo(ipControlInfo);
        }
        if (dp->costVectorSize>0)
        {
            setCostVector(dp->costVector, dp->costVectorSize);
            dp->costVector = NULL;
            dp->costVectorSize = 0;
        }
    }
}

void DSRPkt::ModOptions(struct dsr_pkt *dp, int interface_id)
{
    setEncapProtocol((IPProtocolId)0);
    if (dp)
    {
        IPv4Address destAddress_var((uint32_t)dp->dst.s_addr);
        setDestAddress(destAddress_var);
        IPv4Address srcAddress_var((uint32_t)dp->src.s_addr);
        setSrcAddress(srcAddress_var);
        // ¿como gestionar el MAC
        // dp->mac.raw = p->access(hdr_mac::offset_);
        setHeaderLength(dp->nh.iph->ihl); // Header length
        setVersion(dp->nh.iph->version); // Ip version
        setTypeOfService(dp->nh.iph->tos); // ToS
        setIdentification(dp->nh.iph->id); // Identification

        setMoreFragments(dp->nh.iph->frag_off & 0x2000);
        setDontFragment(dp->nh.iph->frag_off & 0x4000);

        setTimeToLive(dp->nh.iph->ttl); // TTL
        setTransportProtocol(IP_PROT_DSR); // Transport protocol
        struct dsr_opt_hdr *opth;
        opth = dp->dh.opth;
        int dsr_opts_len = opth->p_len + DSR_OPT_HDR_LEN;

        if (options != NULL)
            delete [] options;

        options = (dsr_opt_hdr *)new char[dsr_opts_len];
        memcpy((char*)options, (char*)opth, dsr_pkt_opts_len(dp));

        setBitLength((DSR_OPT_HDR_LEN+IP_HDR_LEN+options->p_len)*8);

        if (dp->payload)
        {
            cPacket *msg = this->decapsulate();
            if (msg)
                delete msg;
            encapsulate(dp->payload);
            dp->payload = NULL;
            setEncapProtocol((IPProtocolId)dp->encapsulate_protocol);

        }

        if (interface_id>=0)
        {
            IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
            //ipControlInfo->setProtocol(IP_PROT_UDP);
            ipControlInfo->setProtocol(IP_PROT_DSR);

            ipControlInfo->setInterfaceId(interface_id); // If broadcast packet send to interface

            ipControlInfo->setSrcAddr(srcAddress_var);
            ipControlInfo->setDestAddr(destAddress_var);
            ipControlInfo->setTimeToLive(dp->nh.iph->ttl);
            setControlInfo(ipControlInfo);
        }
        if (costVectorSize>0)
            delete [] costVector;
        costVectorSize = 0;
        costVector = NULL;

        if (dp->costVectorSize>0)
        {
            setCostVector(dp->costVector, dp->costVectorSize);
            dp->costVector = NULL;
            dp->costVectorSize = 0;
        }

    }
}



std::string DSRPkt::detailedInfo() const
{
    std::stringstream out;
    struct dsr_opt *dopt;
    int dsr_len = options->p_len + DSR_OPT_HDR_LEN;
    int l = DSR_OPT_HDR_LEN;
    out << " DSR Options "  << "\n"; // Khmm...
    dopt = (struct dsr_opt *)(((char *)options) + DSR_OPT_HDR_LEN);
    while (l < dsr_len && (dsr_len - l) > 2)
    {
        //DEBUG("dsr_len=%d l=%d\n", dsr_len, l);
        switch (dopt->type)
        {
        case DSR_OPT_PADN:
            out << " DSR_OPT_PADN "  << "\n"; // Khmm...
            break;
        case DSR_OPT_RREQ:
        {
            out << " DSR_OPT_RREQ "  << "\n"; // Khmm...
            dsr_rreq_opt *rreq_opt = (dsr_rreq_opt*)dopt;
            IPv4Address add(rreq_opt->target);
            out <<" Target :"<< add << "\n"; // Khmm
            int j = 0;
            for (int m=0; m<DSR_RREQ_ADDRS_LEN(rreq_opt); m += sizeof(u_int32_t))
            {
                IPv4Address add(rreq_opt->addrs[j]);
                out << add << "\n"; // Khmm
                j++;
            }
        }
        break;
        case DSR_OPT_RREP:
            out << " DSR_OPT_RREP "  << "\n"; // Khmm...Q

            break;
        case DSR_OPT_RERR:
            out << " DSR_OPT_RERR "  << "\n"; // Khmm...

            break;
        case DSR_OPT_PREV_HOP:
            out << " DSR_OPT_PREV_HOP "  << "\n"; // Khmm...
            break;
        case DSR_OPT_ACK:
            out << " DSR_OPT_ACK "  << "\n"; // Khmm...
            break;
        case DSR_OPT_SRT:
        {
            out << " DSR_OPT_SRT "  << "\n"; // Khmm...
            dsr_srt_opt *srt_opt = (dsr_srt_opt *)dopt;
            out << "next hop : "<< next << "  previous : "<< previous << "\n Route \n";
            int length = srt_opt->length/sizeof(u_int32_t);
            for (int j=0; j<length; j++)
            {
                IPv4Address add(srt_opt->addrs[j]);
                out << add << "\n"; // Khmm
            }
        }
        break;
        case DSR_OPT_TIMEOUT:
            out << " DSR_OPT_TIMEOUT "  << "\n"; // Khmm...
            break;
        case DSR_OPT_FLOWID:
            out << " DSR_OPT_FLOWID "  << "\n"; // Khmm...
            break;
        case DSR_OPT_ACK_REQ:
            out << " DSR_OPT_ACK_REQ "  << "\n"; // Khmm...
            break;
        case DSR_OPT_PAD1:
            out << " DSR_OPT_PAD1 "  << "\n"; // Khmm...
            l++;
            dopt++;
            continue;
        default:
            out << " Unknown DSR option type "  << "\n"; // Khmm...
        }
        l += dopt->length + 2;
        dopt = DSR_GET_NEXT_OPT(dopt);
    }
    return out.str();
}





void DSRPkt::getCostVector(EtxCost &cost, int &size) // Copy
{
    if (size<=0 || costVectorSize==0)
    {
        size = 0;
        return;
    }

    if ((unsigned int) size>costVectorSize)
    {
        memcpy(&cost, costVector, sizeof(EtxCost)*costVectorSize);
        size = costVectorSize;
    }
    else
    {
        memcpy(&cost, costVector, sizeof(EtxCost)*size);
    }
}

// Copy the information of cost in the packet
void DSRPkt::setCostVector(EtxCost &cost, int size)
{
    if (costVectorSize>0)
    {
        setBitLength(getBitLength()-(costVectorSize*SIZE_COST_BITS));
        delete [] costVector;
        costVector = NULL;
        costVectorSize = 0;
    }

    if (size>0)
    {
        costVector = new EtxCost[size];
        costVectorSize = size;
        memcpy(costVector, &cost, sizeof(EtxCost)*size);
        setBitLength(getBitLength()+(costVectorSize*SIZE_COST_BITS));
    }
}


// The pointer *cost is the new pointer
void DSRPkt::setCostVector(EtxCost *cost, int size)
{
    if (costVectorSize>0)
    {
        setBitLength(getBitLength()-(costVectorSize*SIZE_COST_BITS));
        delete [] costVector;
    }

    costVector = NULL;
    costVectorSize = 0;

    if (size>0)
    {
        costVector = cost;
        costVectorSize = size;
        setBitLength(getBitLength()+(costVectorSize*SIZE_COST_BITS));
    }
}


void DSRPkt::setCostVectorSize(unsigned n)
{
    if (n>0)
    {
        setBitLength(getBitLength()-(costVectorSize*SIZE_COST_BITS));
        EtxCost *cost = new EtxCost[n];
        memset(cost, 0, sizeof(EtxCost)*n);
        if (costVectorSize>0)
        {
            if (n>costVectorSize)
            {
                memcpy(&cost, costVector, sizeof(EtxCost)*costVectorSize);
            }
            else
            {
                memcpy(&cost, costVector, sizeof(EtxCost)*n);

            }
            delete [] costVector;

        }
        costVectorSize = n;
        setBitLength(getBitLength()+(costVectorSize*SIZE_COST_BITS));
        costVector = cost;
    }
    else
    {
        setBitLength(getBitLength()-(costVectorSize*SIZE_COST_BITS));
        delete [] costVector;
        costVector = NULL;
        costVectorSize = 0;
        return;
    }

}

void DSRPkt::setCostVectorSize(EtxCost newLinkCost)
{
    setBitLength(getBitLength()+SIZE_COST_BITS);
    EtxCost *cost = new EtxCost[costVectorSize+1];
    if (costVectorSize>0)
    {
        memcpy(cost, costVector, sizeof(EtxCost)*costVectorSize);
        delete [] costVector;
    }
    cost[costVectorSize].address = newLinkCost.address;
    cost[costVectorSize].cost = newLinkCost.cost;
    costVector = cost;
    costVectorSize++;
    setBitLength(getBitLength()+SIZE_COST_BITS);
}

void DSRPkt::setCostVectorSize(u_int32_t addr, double cost)
{
    EtxCost newLinkCost;
    IPv4Address address(addr);
    newLinkCost.address = address;
    newLinkCost.cost = cost;
    setCostVectorSize(newLinkCost);
}

void DSRPkt::resetCostVector()
{
    costVector = NULL;
    setBitLength(getBitLength()-(costVectorSize*SIZE_COST_BITS));
    costVectorSize = 0;
}

DSRPktExt::DSRPktExt(const DSRPktExt& m) : IPv4Datagram(m)
{
    copy(m);
}


DSRPktExt& DSRPktExt::operator=(const DSRPktExt& msg)
{
    if (this==&msg) return *this;
    clean();
    IPv4Datagram::operator=(msg);
    copy(msg);
    return *this;
}

void DSRPktExt::copy(const DSRPktExt& msg)
{
    size = msg.size;
    if (size==0)
    {
        extension = NULL;
        return;
    }
    extension = new EtxList[size];
    memcpy(extension, msg.extension, size*sizeof(EtxList));
}

void DSRPktExt:: clearExtension()
{
    if (size==0)
    {
        return;
    }
    delete [] extension;
    size = 0;
}

DSRPktExt::~DSRPktExt()
{
    clearExtension();
}

EtxList * DSRPktExt::addExtension(int len)
{
    EtxList * extension_aux;
    if (len<0)
    {
        return NULL;
    }
    extension_aux = new EtxList [size+len];
    memcpy(extension_aux, extension, size*sizeof(EtxList));
    delete [] extension;
    extension = extension_aux;
    size += len;
    setBitLength(getBitLength()+(len*8*8)); // 4 ip-address 4 cost
    return extension;
}

EtxList * DSRPktExt::delExtension(int len)
{
    EtxList * extension_aux;
    if (len<0)
    {
        return NULL;
    }
    if (size-len<=0)
    {
        delete [] extension;
        extension = NULL;
        size = 0;
        return extension;
    }

    extension_aux = new EtxList [size-len];
    memcpy(extension_aux, extension, (size-len)*sizeof(EtxList));
    delete [] extension;
    extension = extension_aux;

    size -= len;
    setBitLength(getBitLength()-(len*8*8));
    return extension;
}

} // namespace inetmanet

} // namespace inet

