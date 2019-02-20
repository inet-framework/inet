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
#include "inet/routing/extras/dsr/DsrUuPktOmnet.h"
#include "inet/routing/extras/dsr/DsrProtocolTag_m.h"
#include "inet/common/ProtocolTools.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-pkt.h"
#include "inet/networklayer/common/HopLimitTag_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"


namespace inet {

namespace inetmanet {

#define SIZE_COST_BITS 16
DSRPkt::~DSRPkt()
{
    clean();
}

void DSRPkt::clean()
{
    dsrOptions.clear();
    costVector.clear();
}

DSRPkt::DSRPkt(const DSRPkt& m) : FieldsChunk(m)
{
    costVector.clear();
    dsrOptions.clear();
    copy(m);
}

DSRPkt& DSRPkt::operator=(const DSRPkt& m)
{
    if (this==&m) return *this;
    clean();
    FieldsChunk::operator=(m);
    copy(m);
    return *this;
}

void DSRPkt::copy(const DSRPkt& m)
{
    dsr_ttl = m.dsr_ttl;
    encapProtocol = m.encapProtocol;
    previous = m.previous;
    next = m.next;
    dsrOptions = m.dsrOptions;
    costVector = m.costVector;
}

std::string DSRPkt::str() const
{
    std::stringstream out;
    struct dsr_opt *dopt;
    int l = DSR_OPT_HDR_LEN;
    out << " DSR Options "  << "\n"; // Khmm...

    for (unsigned int i = 0; i < dsrOptions.size(); i++)
    {
        for (unsigned int j = 0; j < dsrOptions[i].option.size(); j++)
        {
            dopt = dsrOptions[i].option[j];
            //DEBUG("dsr_len=%d l=%d\n", dsr_len, l);
            switch (dopt->type)
            {
                case DSR_OPT_PADN:
                    out << " DSR_OPT_PADN " << "\n"; // Khmm...
                    break;
                case DSR_OPT_RREQ:
                {
                    out << " DSR_OPT_RREQ " << "\n"; // Khmm...
                    dsr_rreq_opt *rreq_opt = check_and_cast<dsr_rreq_opt*>(dopt);
                    L3Address add(rreq_opt->target);
                    out << " Target :" << add << "\n"; // Khmm
                    for (unsigned int m = 0; m < rreq_opt->addrs.size(); m++)
                    {
                        L3Address add(rreq_opt->addrs[m]);
                        out << add << "\n"; // Khmm
                    }
                }
                    break;
                case DSR_OPT_RREP:
                {
                    out << " DSR_OPT_RREP " << "\n"; // Khmm...Q
                    dsr_rrep_opt *rrep_opt = check_and_cast<dsr_rrep_opt*>(dopt);
                    for (unsigned int m = 0; m < rrep_opt->addrs.size(); m++)
                    {
                        L3Address add(rrep_opt->addrs[m]);
                        out << add << "\n"; // Khmm
                    }

                }
                    break;
                case DSR_OPT_RERR:
                    out << " DSR_OPT_RERR " << "\n"; // Khmm...

                    break;
                case DSR_OPT_PREV_HOP:
                    out << " DSR_OPT_PREV_HOP " << "\n"; // Khmm...
                    break;
                case DSR_OPT_ACK:
                    out << " DSR_OPT_ACK " << "\n"; // Khmm...
                    break;
                case DSR_OPT_SRT:
                {
                    out << " DSR_OPT_SRT " << "\n"; // Khmm...
                    dsr_srt_opt *srt_opt = check_and_cast<dsr_srt_opt*>(dopt);
                    out << "next hop : " << next << "  previous : " << previous << "\n Route \n";
                    for (unsigned int j = 0; j < srt_opt->addrs.size(); j++)
                    {
                        L3Address add(srt_opt->addrs[j]);
                        out << add << "\n"; // Khmm
                    }
                }
                    break;
                case DSR_OPT_TIMEOUT:
                    out << " DSR_OPT_TIMEOUT " << "\n"; // Khmm...
                    break;
                case DSR_OPT_FLOWID:
                    out << " DSR_OPT_FLOWID " << "\n"; // Khmm...
                    break;
                case DSR_OPT_ACK_REQ:
                    out << " DSR_OPT_ACK_REQ " << "\n"; // Khmm...
                    break;
                case DSR_OPT_PAD1:
                    out << " DSR_OPT_PAD1 " << "\n"; // Khmm...
                    l++;
                    dopt++;
                    continue;
                default:
                    out << " Unknown DSR option type " << "\n"; // Khmm...
            }
        }
    }
    return out.str();
}

void DSRPkt::getCostVector(std::vector<EtxCost> &cost) // Copy
{
    cost = costVector;
}

// The pointer *cost is the new pointer
void DSRPkt::setCostVector(std::vector<EtxCost> &cost)
{
    if (costVector.size() > 0)
    {
        // setBitLength(getBitLength()-(costVector.size() * SIZE_COST_BITS));
        this->setChunkLength(this->getChunkLength()-b(costVector.size() * SIZE_COST_BITS));
        costVector.clear();
    }
    costVector = cost;
    this->setChunkLength(this->getChunkLength()+b(costVector.size() * SIZE_COST_BITS));
}


void DSRPkt::setCostVectorSize(EtxCost newLinkCost)
{
    this->setChunkLength(this->getChunkLength()+b(SIZE_COST_BITS));
    costVector.push_back(newLinkCost);
}

void DSRPkt::setCostVectorSize(L3Address addr, double cost)
{
    EtxCost newLinkCost;
    newLinkCost.address = addr;
    newLinkCost.cost = cost;
    setCostVectorSize(newLinkCost);
}

void DSRPkt::resetCostVector()
{
    this->setChunkLength(this->getChunkLength()-b(costVector.size() * SIZE_COST_BITS));
    costVector.clear();
}

DSRPktExt::DSRPktExt(const DSRPktExt& m) : DSRPkt(m)
{
    copy(m);
}


DSRPktExt& DSRPktExt::operator=(const DSRPktExt& msg)
{
    if (this==&msg) return *this;
    clean();
    DSRPkt::operator=(msg);
    copy(msg);
    return *this;
}

void DSRPktExt::copy(const DSRPktExt& msg)
{
    size = msg.size;
    if (size==0)
    {
        extension = nullptr;
        return;
    }
    extension = new EtxList[size];
    memcpy(extension, msg.extension, size*sizeof(EtxList));
}

void DSRPktExt:: clearExtension()
{
    DSRPkt::cleanAll();
    if (size==0)
    {
        return;
    }
    delete [] extension;
    size = 0;
}

DSRPktExt::~DSRPktExt()
{
    DSRPkt::cleanAll();
    clearExtension();
}

EtxList * DSRPktExt::addExtension(int len)
{
    EtxList * extension_aux;
    if (len<0)
    {
        return nullptr;
    }
    extension_aux = new EtxList [size+len];
    memcpy(extension_aux, extension, size*sizeof(EtxList));
    delete [] extension;
    extension = extension_aux;
    size += len;
    this->setChunkLength(this->getChunkLength()+b((len*8*8)));
    //setBitLength(getBitLength()+(len*8*8)); // 4 ip-address 4 cost
    return extension;
}

EtxList * DSRPktExt::delExtension(int len)
{
    EtxList * extension_aux;
    if (len<0)
    {
        return nullptr;
    }
    if (size-len<=0)
    {
        delete [] extension;
        extension = nullptr;
        size = 0;
        return extension;
    }

    extension_aux = new EtxList [size-len];
    memcpy(extension_aux, extension, (size-len)*sizeof(EtxList));
    delete [] extension;
    extension = extension_aux;

    size -= len;
    //setBitLength(getBitLength()-(len*8*8));
    this->setChunkLength(this->getChunkLength() - b((len*8*8)));
    return extension;
}

const Ptr<const DSRPkt> findDsrProtocolHeader(Packet *packet)
{
    auto dsrProtocolInd = packet->findTag<DsrProtocolInd>();
    return dsrProtocolInd == nullptr ? nullptr : dynamicPtrCast<const DSRPkt>(dsrProtocolInd->getDsrProtocolHeader());
}

const Ptr<const DSRPkt> getDsrProtocolHeader(Packet *packet)
{
    const auto& header = findDsrProtocolHeader(packet);
    if (header == nullptr)
        throw cRuntimeError("Network protocol header not found");
    else
        return header;
}

void insertDsrProtocolHeader(Packet *packet, const Ptr<DSRPkt>& header)
{
    auto networkProtocolInd = packet->addTagIfAbsent<DsrProtocolInd>();
    networkProtocolInd->setDsrProtocolHeader(header);
    insertProtocolHeader(packet, Protocol::dsr, header);
}

const Ptr<DSRPkt> removeDsrProtocolHeader(Packet *packet)
{

    delete packet->removeTagIfPresent<DsrProtocolInd>();
    return removeProtocolHeader<DSRPkt>(packet);
}

} // namespace inetmanet

} // namespace inet

