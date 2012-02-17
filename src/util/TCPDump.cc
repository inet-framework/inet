//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "TCPDump.h"
#include "IPControlInfo_m.h"
#include "SCTPMessage.h"
#include "SCTPAssociation.h"
#include "IPSerializer.h"
#include "ICMPMessage.h"
#include "UDPPacket_m.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

#define MAXBUFLENGTH 65536

TCPDumper::TCPDumper(std::ostream& out)
{
    outp = &out;
}

TCPDumper::~TCPDumper()
{
}

void TCPDumper::ipDump(const char *label, IPDatagram *dgram, const char *comment)
{
    if (dynamic_cast<SCTPMessage *>(dgram->getEncapsulatedPacket()))
    {
        SCTPMessage *sctpmsg = check_and_cast<SCTPMessage *>(dgram->getEncapsulatedPacket());
        if (dgram->hasBitError())
            sctpmsg->setBitError(true);
        sctpDump(label, sctpmsg, dgram->getSrcAddress().str(), dgram->getDestAddress().str(), comment);
    }
    else
        delete dgram;
}

void TCPDumper::sctpDump(const char *label, SCTPMessage *sctpmsg, const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    std::ostream& out = *outp;
    uint32 numberOfChunks;
    SCTPChunk* chunk;
    uint8 type;
    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf, "[%.3f%s] ", simulation.getSimTime().dbl(), label);
    out << buf;

    // src/dest
    out << srcAddr  << "." << sctpmsg->getSrcPort()  << " > ";

    out << destAddr << "." << sctpmsg->getDestPort() << ": ";
    if (sctpmsg->hasBitError())
    {
        sctpmsg->setChecksumOk(false);
    }
    numberOfChunks = sctpmsg->getChunksArraySize();
    out << "numberOfChunks="<<numberOfChunks<<" VTag="<<sctpmsg->getTag()<<"\n";
    if (sctpmsg->hasBitError())
        out << "Packet has bit error!!\n";
    for (uint32 i=0; i<numberOfChunks; i++)
    {
        chunk = (SCTPChunk*)sctpmsg->getChunks(i);
        type = chunk->getChunkType();
        switch (type)
        {
            case INIT:
                out << "INIT ";
                break;
            case INIT_ACK:
                out << "INIT_ACK ";
                break;
            case COOKIE_ECHO:
                out << "COOKIE_ECHO ";
                break;
            case COOKIE_ACK:
                out << "COOKIE_ACK ";
                break;
            case DATA:
                out << "DATA ";
                break;
            case SACK:
                out << "SACK ";
                break;
            case HEARTBEAT:
                out << "HEARTBEAT ";
                break;
            case HEARTBEAT_ACK:
                out << "HEARTBEAT_ACK ";
                break;
            case ABORT:
                out << "ABORT ";
                break;
            case SHUTDOWN:
                out << "SHUTDOWN ";
                break;
            case SHUTDOWN_ACK:
                out << "SHUTDOWN_ACK ";
                break;
            case SHUTDOWN_COMPLETE:
                out << "SHUTDOWN_COMPLETE ";
                break;
            case ERRORTYPE:
                out << "ERROR";
                break;

        }
    }

    if (verbosity >= 1)
    {
        out << endl;
        for (uint32 i=0; i<numberOfChunks; i++)
        {
            chunk = (SCTPChunk*)sctpmsg->getChunks(i);
            type = chunk->getChunkType();

            sprintf(buf,  "   %3u: ", i + 1);
            out << buf;
            switch (type)
            {
                case INIT:
                {
                    SCTPInitChunk* initChunk;
                    initChunk = check_and_cast<SCTPInitChunk *>(chunk);
                    out << "INIT[InitiateTag=";
                    out << initChunk->getInitTag();
                    out << "; a_rwnd=";
                    out << initChunk->getA_rwnd();
                    out << "; OS=";
                    out << initChunk->getNoOutStreams();
                    out << "; IS=";
                    out << initChunk->getNoInStreams();
                    out << "; InitialTSN=";
                    out << initChunk->getInitTSN();
                    if (initChunk->getAddressesArraySize() > 0)
                    {
                        out <<"; Addresses=";
                        for (uint32 i = 0; i < initChunk->getAddressesArraySize(); i++)
                        {
                            if (i > 0)
                                out << ",";
                            if (initChunk->getAddresses(i).isIPv6())
                                out << initChunk->getAddresses(i).str();
                            else
                                out << initChunk->getAddresses(i);
                        }
                    }

                    out <<"]";
                    break;
                }
                case INIT_ACK:
                {
                    SCTPInitAckChunk* initackChunk;
                    initackChunk = check_and_cast<SCTPInitAckChunk *>(chunk);
                    out << "INIT_ACK[InitiateTag=";
                    out << initackChunk->getInitTag();
                    out << "; a_rwnd=";
                    out << initackChunk->getA_rwnd();
                    out << "; OS=";
                    out << initackChunk->getNoOutStreams();
                    out << "; IS=";
                    out << initackChunk->getNoInStreams();
                    out << "; InitialTSN=";
                    out << initackChunk->getInitTSN();
                    out << "; CookieLength=";
                    out << initackChunk->getCookieArraySize();
                    if (initackChunk->getAddressesArraySize() > 0)
                    {
                        out <<"; Addresses=";
                        for (uint32 i = 0; i < initackChunk->getAddressesArraySize(); i++)
                        {
                            if (i > 0)
                                out << ",";
                            out << initackChunk->getAddresses(i);
                        }
                    }
                    out <<"]";
                    break;
                }
                case COOKIE_ECHO:
                    out << "COOKIE_ECHO[CookieLength=";
                    out <<     chunk->getBitLength()/8 - 4;
                    out <<"]";
                    break;
                case COOKIE_ACK:
                    out << "COOKIE_ACK ";
                    break;
                case DATA:
                {
                    SCTPDataChunk* dataChunk;
                    dataChunk = check_and_cast<SCTPDataChunk *>(chunk);
                    out << "DATA[TSN=";
                    out << dataChunk->getTsn();
                    out << "; SID=";
                    out << dataChunk->getSid();
                    out << "; SSN=";
                    out << dataChunk->getSsn();
                    out << "; PPID=";
                    out << dataChunk->getPpid();
                    out << "; PayloadLength=";
                    out << dataChunk->getBitLength()/8 - 16;
                    out <<"]";
                    break;
                }
                case SACK:
                {
                    SCTPSackChunk* sackChunk;
                    sackChunk = check_and_cast<SCTPSackChunk *>(chunk);
                    out << "SACK[CumTSNAck=";
                    out << sackChunk->getCumTsnAck();
                    out << "; a_rwnd=";
                    out << sackChunk->getA_rwnd();
                    if (sackChunk->getGapStartArraySize() > 0)
                    {
                        out <<"; Gaps=";
                        for (uint32 i = 0; i < sackChunk->getGapStartArraySize(); i++)
                        {
                            if (i > 0)
                                out << ", ";
                            out << sackChunk->getGapStart(i) << "-" << sackChunk->getGapStop(i);
                        }
                    }
                    if (sackChunk->getDupTsnsArraySize() > 0)
                    {
                        out <<"; Dups=";
                        for (uint32 i = 0; i < sackChunk->getDupTsnsArraySize(); i++)
                        {
                            if (i > 0)
                                out << ", ";
                            out << sackChunk->getDupTsns(i);
                        }
                    }
                    out <<"]";
                    break;
                }
                case HEARTBEAT:
                    SCTPHeartbeatChunk* heartbeatChunk;
                    heartbeatChunk = check_and_cast<SCTPHeartbeatChunk *>(chunk);
                    out << "HEARTBEAT[InfoLength=";
                    out <<     chunk->getBitLength()/8 - 4;
                    out << "; time=";
                    out << heartbeatChunk->getTimeField();
                    out <<"]";
                    break;
                case HEARTBEAT_ACK:
                    out << "HEARTBEAT_ACK[InfoLength=";
                    out <<     chunk->getBitLength()/8 - 4;
                    out <<"]";
                    break;
                case ABORT:
                    SCTPAbortChunk* abortChunk;
                    abortChunk = check_and_cast<SCTPAbortChunk *>(chunk);
                    out << "ABORT[T-Bit=";
                    out << abortChunk->getT_Bit();
                    out << "]";
                    break;
                case SHUTDOWN:
                    SCTPShutdownChunk* shutdown;
                    shutdown = check_and_cast<SCTPShutdownChunk *>(chunk);
                    out << "SHUTDOWN[CumTSNAck=";
                    out << shutdown->getCumTsnAck();
                    out << "]";
                    break;
                case SHUTDOWN_ACK:
                    out << "SHUTDOWN_ACK ";
                    break;
                case SHUTDOWN_COMPLETE:
                    out << "SHUTDOWN_COMPLETE ";
                    break;
                case ERRORTYPE:
                {
                    SCTPErrorChunk* errorChunk;
                    errorChunk = check_and_cast<SCTPErrorChunk*>(chunk);
                    uint32 numberOfParameters = errorChunk->getParametersArraySize();
                    uint32 parameterType;
                    for (uint32 i=0; i<numberOfParameters; i++)
                    {
                        SCTPParameter* param = (SCTPParameter*)errorChunk->getParameters(i);
                        parameterType = param->getParameterType();
                    }

                    break;
                }

            }
            out << endl;
        }
    }
    // comment
    if (comment)
        out << "# " << comment;

    out << endl;
}

TCPDump::~TCPDump()
{
}

const char *TCPDumper::intToChunk(int32 type)
{
    switch (type)
    {
        case 0: return "DATA";
        case 1: return "INIT";
        case 2: return "INIT_ACK";
        case 3: return "SACK";
        case 4: return "HEARTBEAT";
        case 5: return "HEARTBEAT_ACK";
        case 6: return "ABORT";
        case 7: return "SHUTDOWN";
        case 8: return "SHUTDOWN_ACK";
        case 9: return "ERRORTYPE";
        case 10: return "COOKIE_ECHO";
        case 11: return "COOKIE_ACK";
        case 14: return "SHUTDOWN_COMPLETE";
    }
    return "";
}

void TCPDumper::dump(const char *label, const char *msg)
{
    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf, "[%.3f%s] ", simulation.getSimTime().dbl(), label);
    out << buf;

    out << msg << "\n";
}

//----

Define_Module(TCPDump);

TCPDump::TCPDump() : cSimpleModule(), tcpdump(ev.getOStream())
{


}

void TCPDumper::udpDump(bool l2r, const char *label, IPDatagram *dgram, const char *comment)
{
    cMessage *encapmsg = dgram->getEncapsulatedPacket();
    if (dynamic_cast<UDPPacket *>(encapmsg))
    {
        std::ostream& out = *outp;

        // seq and time (not part of the tcpdump format)
        char buf[30];
        sprintf(buf, "[%.3f%s] ", simulation.getSimTime().dbl(), label);
        out << buf;
        UDPPacket* udppkt = check_and_cast<UDPPacket*>(encapmsg);

        // src/dest
        if (l2r)
        {
            out << dgram->getSrcAddress().str() << "." << udppkt->getSourcePort() << " > ";
            out << dgram->getDestAddress().str() << "." << udppkt->getDestinationPort() << ": ";
        }
        else
        {
            out << dgram->getDestAddress().str() << "." << udppkt->getDestinationPort() << " < ";
            out << dgram->getSrcAddress().str() << "." << udppkt->getSourcePort() << ": ";
        }

        //out << endl;
        out << "UDP: Payload length=" << udppkt->getByteLength()-8 << endl;
        if (udppkt->getSourcePort()==9899 || udppkt->getDestinationPort() == 9899)
        {
            if (dynamic_cast<SCTPMessage *>(udppkt->getEncapsulatedPacket()))
                sctpDump("", (SCTPMessage *)(udppkt->getEncapsulatedPacket()), std::string(l2r?"A":"B"), std::string(l2r?"B":"A"));
        }
    }
}

void TCPDumper::tcpDump(bool l2r, const char *label, IPDatagram *dgram, const char *comment)
{
    cMessage *encapmsg = dgram->getEncapsulatedPacket();
    if (dynamic_cast<TCPSegment *>(encapmsg))
    {
        // if TCP, dump as TCP
        tcpDump(l2r, label, (TCPSegment *)encapmsg, dgram->getSrcAddress().str(), dgram->getDestAddress().str(), comment);
    }
    else
    {
        // some other packet, dump what we can
        std::ostream& out = *outp;

        // seq and time (not part of the tcpdump format)
        char buf[30];
        sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
        out << buf;

        // packet class and name
        out << "? " << encapmsg->getClassName() << " \"" << encapmsg->getName() << "\"\n";
    }
}

//FIXME: Temporary hack for Ipv6 support
void TCPDumper::dumpIPv6(bool l2r, const char *label, IPv6Datagram_Base *dgram, const char *comment)
{
    cMessage *encapmsg = dgram->getEncapsulatedPacket();
    if (dynamic_cast<TCPSegment *>(encapmsg))
    {
        // if TCP, dump as TCP
        tcpDump(l2r, label, (TCPSegment *)encapmsg, dgram->getSrcAddress().str(), dgram->getDestAddress().str(), comment);
    }
    else
    {
        // some other packet, dump what we can
        std::ostream& out = *outp;

        // seq and time (not part of the tcpdump format)
        char buf[30];
        sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
        out << buf;

        // packet class and name
        out << "? " << encapmsg->getClassName() << " \"" << encapmsg->getName() << "\"\n";
    }
}

void TCPDumper::tcpDump(bool l2r, const char *label, TCPSegment *tcpseg, const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf;
    // src/dest ports
    if (l2r)
    {
        out << srcAddr << "." << tcpseg->getSrcPort() << " > ";
        out << destAddr << "." << tcpseg->getDestPort() << ": ";
    }
    else
    {
        out << destAddr << "." << tcpseg->getDestPort() << " < ";
        out << srcAddr << "." << tcpseg->getSrcPort() << ": ";
    }

    // flags
    bool flags = false;
    if (tcpseg->getUrgBit()) {flags = true; out << "U ";}
    if (tcpseg->getAckBit()) {flags = true; out << "A ";}
    if (tcpseg->getPshBit()) {flags = true; out << "P ";}
    if (tcpseg->getRstBit()) {flags = true; out << "R ";}
    if (tcpseg->getSynBit()) {flags = true; out << "S ";}
    if (tcpseg->getFinBit()) {flags = true; out << "F ";}
    if (!flags) {out << ". ";}

    // data-seqno
    if (tcpseg->getPayloadLength()>0 || tcpseg->getSynBit())
    {
        out << tcpseg->getSequenceNo() << ":" << tcpseg->getSequenceNo()+tcpseg->getPayloadLength();
        out << "(" << tcpseg->getPayloadLength() << ") ";
    }

    // ack
    if (tcpseg->getAckBit())
        out << "ack " << tcpseg->getAckNo() << " ";

    // window
    out << "win " << tcpseg->getWindow() << " ";

    // urgent
    if (tcpseg->getUrgBit())
        out << "urg " << tcpseg->getUrgentPointer() << " ";

    // options present?
    if (tcpseg->getHeaderLength() > 20)
    {
        std::string direction = "sent";
        if (l2r) // change direction
        {direction = "received";}

        unsigned short numOptions = tcpseg->getOptionsArraySize();
        out << "\nTCP Header Option(s) " << direction << ":\n";
        for (int i=0; i<numOptions; i++)
        {
            TCPOption option = tcpseg->getOptions(i);
            unsigned short kind = option.getKind();
            unsigned short length = option.getLength();
            out << (i+1) << ". option kind=" << kind << " length=" << length << "\n";
        }
    }

    // comment
    if (comment)
        out << "# " << comment;

    out << endl;
}

void TCPDump::initialize()
{
    struct pcap_hdr fh;
    const char* file = this->par("dumpFile");
    snaplen = this->par("snaplen");
    tcpdump.setVerbosity(par("verbosity"));


    if (strcmp(file, "")!=0)
    {
        tcpdump.dumpfile = fopen(file, "wb");
        if (!tcpdump.dumpfile)
        {
            fprintf(stderr, "Cannot open file [%s] for writing: %s\n", file, strerror(errno));
            exit(-1);
        }

        fh.magic = PCAP_MAGIC;
        fh.version_major = 2;
        fh.version_minor = 4;
        fh.thiszone = 0;
        fh.sigfigs = 0;
        fh.snaplen = snaplen;
        fh.network = 0;
        fwrite(&fh, sizeof(fh), 1, tcpdump.dumpfile);
    }
    else
        tcpdump.dumpfile = NULL;
}

void TCPDump::handleMessage(cMessage *msg)
{

    if (!ev.disable_tracing)
    {
        bool l2r;

        if (dynamic_cast<IPDatagram *>(msg))
        {
            if (((IPDatagram *)msg)->getTransportProtocol()==132)
            {
                tcpdump.ipDump("", (IPDatagram *)msg);
            }
            else
            {
                if (PK(msg)->hasBitError())
                {
                    delete msg;
                    return;
                }
                l2r = msg->arrivedOn("in1");
                if (((IPDatagram *)msg)->getTransportProtocol()==6)
                {
                    tcpdump.tcpDump(l2r, "", (IPDatagram *)msg, "");
                }
                else if (((IPDatagram *)msg)->getTransportProtocol()==17)
                    tcpdump.udpDump(l2r, "", (IPDatagram *)msg, "");
            }
        }
        else if (dynamic_cast<SCTPMessage *>(msg))
        {
            l2r = msg->arrivedOn("in1");
            tcpdump.sctpDump("", (SCTPMessage *)msg, std::string(l2r?"A":"B"), std::string(l2r?"B":"A"));
        }
        else if (dynamic_cast<TCPSegment *>(msg))
        {
            if (PK(msg)->hasBitError())
            {
                delete msg;
                return;
            }
            l2r = msg->arrivedOn("in1");
            tcpdump.tcpDump(l2r, "", (TCPSegment *)msg, std::string(l2r?"A":"B"), std::string(l2r?"B":"A"));
        }
        else if (dynamic_cast<ICMPMessage *>(msg))
        {
            if (PK(msg)->hasBitError())
            {
                delete msg;
                return;
            }
            std::cout<<"ICMPMessage\n";
        }
        else
        {
            // search for encapsulated IP[v6]Datagram in it
            cPacket *encapmsg = PK(msg);
            while (encapmsg && dynamic_cast<IPDatagram *>(encapmsg)==NULL && dynamic_cast<IPv6Datagram_Base *>(encapmsg)==NULL)
                encapmsg = encapmsg->getEncapsulatedPacket();
            l2r = msg->arrivedOn("in1");
            if (!encapmsg)
            {
                //We do not want this to end in an error if EtherAutoconf messages
                //are passed, so just print a warning. -WEI
                EV << "CANNOT DECODE: packet " << msg->getName() << " doesn't contain either IP or IPv6 Datagram\n";
            }
            else
            {
                if (dynamic_cast<IPDatagram *>(encapmsg))
                    tcpdump.tcpDump(l2r, "", (IPDatagram *)encapmsg);
                else if (dynamic_cast<IPv6Datagram_Base *>(encapmsg))
                    tcpdump.dumpIPv6(l2r, "", (IPv6Datagram_Base *)encapmsg);
                else
                    ASSERT(0); // cannot get here
            }
        }
    }


    if (tcpdump.dumpfile!=NULL && dynamic_cast<IPDatagram *>(msg))
    {
        uint8 buf[MAXBUFLENGTH];
        memset((void*)&buf, 0, sizeof(buf));

        const simtime_t stime = simulation.getSimTime();
        // Write PCap header

        struct pcaprec_hdr ph;
        ph.ts_sec = (int32)stime.dbl();
        ph.ts_usec = (uint32)((stime.dbl() - ph.ts_sec)*1000000);
        // Write Ethernet header
        uint32 hdr = 2; //AF_INET
        //We do not want this to end in an error if EtherAutoconf messages
        IPDatagram *ipPacket = check_and_cast<IPDatagram *>(msg);
        // IP header:
        //struct sockaddr_in *to = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
        //int32 tosize = sizeof(struct sockaddr_in);
        int32 serialized_ip = IPSerializer().serialize(ipPacket, buf, sizeof(buf));
        ph.incl_len = serialized_ip + sizeof(uint32);

        ph.orig_len = ph.incl_len;
        fwrite(&ph, sizeof(ph), 1, tcpdump.dumpfile);
        fwrite(&hdr, sizeof(uint32), 1, tcpdump.dumpfile);
        fwrite(buf, serialized_ip, 1, tcpdump.dumpfile);
    }


    // forward
    int32 index = msg->getArrivalGate()->getIndex();
    int32 id;
    if (msg->getArrivalGate()->isName("ifIn"))
        id = findGate("out2", index);
    else
        id = findGate("ifOut", index);

    send(msg, id);
}

void TCPDump::finish()
{
    tcpdump.dump("", "tcpdump finished");
    if (strcmp(this->par("dumpFile"), "")!=0)
        fclose(tcpdump.dumpfile);
}

