//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "SCTP.h"
#include "SCTPAssociation.h"
#include "SCTPCommand_m.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"

#ifdef WITH_IPv4
#include "IPv4Datagram.h"
#endif

#include "UDPControlInfo_m.h"
#include "UDPSocket.h"

Define_Module(SCTP);


bool SCTP::testing;
bool SCTP::logverbose;

int32 SCTP::nextConnId = 0;


void SCTP::printInfoConnMap()
{
    SCTPAssociation* assoc;
    SockPair      key;
    sctpEV3<<"Number of Assocs: "<<sizeConnMap<<"\n";
    if (sizeConnMap>0)
    {
        for (SctpConnMap::iterator i = sctpConnMap.begin(); i!=sctpConnMap.end(); ++i)
        {
            assoc = i->second;
            key = i->first;

                sctpEV3<<"assocId: "<<assoc->assocId<<"  assoc: "<<assoc<<" src: "<<IPvXAddress(key.localAddr)<<" dst: "<<IPvXAddress(key.remoteAddr)<<" lPort: "<<key.localPort<<" rPort: "<<key.remotePort<<"\n";

        }

        sctpEV3<<"\n";
    }

}

void SCTP::printVTagMap()
{
    int32 assocId;
    VTagPair      key;
    sctpEV3<<"Number of Assocs: "<<sctpVTagMap.size()<<"\n";
    if (sctpVTagMap.size()>0)
    {
        for (SctpVTagMap::iterator i = sctpVTagMap.begin(); i!=sctpVTagMap.end(); ++i)
        {
            assocId = i->first;
            key = i->second;

                sctpEV3<<"assocId: "<<assocId<<" peerVTag: "<<key.peerVTag<<
                " localVTag: "<<key.localVTag<<
                " localPort: "<<key.localPort<<" rPort: "<<key.remotePort<<"\n";
        }

        sctpEV3<<"\n";
    }
}

void SCTP::bindPortForUDP()
{
    EV << "Binding to UDP port " << SCTP_UDP_PORT << endl;

    udpSocket.setOutputGate(gate("to_ip"));
    udpSocket.bind(SCTP_UDP_PORT);
}

void SCTP::initialize()
{
    nextEphemeralPort = (uint16)(intrand(10000) + 30000);

    //sctpEV3<<"SCTPMain initialize\n";

    cModule *netw = simulation.getSystemModule();

    testing = netw->hasPar("testing") && netw->par("testing").boolValue();
    if (testing) {
    }
    if (netw->hasPar("testTimeout"))
    {
        testTimeout = (simtime_t)netw->par("testTimeout");
    }
    numPacketsReceived = 0;
    numPacketsDropped = 0;
    sizeConnMap = 0;
    if ((bool)par("udpEncapsEnabled"))
        bindPortForUDP();
}


SCTP::~SCTP()
{
    sctpEV3<<"delete SCTPMain\n";
    if (!(sctpAppConnMap.empty()))
    {
        sctpEV3<<"clear appConnMap ptr="<<&sctpAppConnMap<<"\n";
        sctpAppConnMap.clear();
    }
    if (!(assocStatMap.empty()))
    {
        sctpEV3<<"clear assocStatMap ptr="<<&assocStatMap<<"\n";
        assocStatMap.clear();
    }
    if (!(sctpVTagMap.empty()))
    {
        sctpVTagMap.clear();
    }
    sctpEV3<<"after clearing maps\n";
}


void SCTP::handleMessage(cMessage *msg)
{
    IPvXAddress destAddr;
    IPvXAddress srcAddr;
    bool findListen = false;
    bool bitError = false;

    sctpEV3<<"\n\nSCTPMain handleMessage at "<<getFullPath()<<"\n";

    if (msg->isSelfMessage())
    {

        sctpEV3<<"selfMessage\n";

         SCTPAssociation *assoc = (SCTPAssociation *) msg->getContextPointer();
        bool ret = assoc->processTimer(msg);

        if (!ret)
            removeAssociation(assoc);
    }
    else if (msg->arrivedOn("from_ip") || msg->arrivedOn("from_ipv6"))
    {
        sctpEV3<<"Message from IP\n";
        printInfoConnMap();
        if (!dynamic_cast<SCTPMessage *>(msg))
        {
            sctpEV3<<"no sctp message, delete it\n";
            delete msg;
            return;
        }
        SCTPMessage *sctpmsg = check_and_cast<SCTPMessage *>(msg);

        numPacketsReceived++;

        if ((sctpmsg->hasBitError() || !(sctpmsg->getChecksumOk()))) {
            sctpEV3<<"Packet has bit-error. delete it\n";

            bitError = true;
            numPacketsDropped++;
            delete msg;
            return;
        }
        if (msg->arrivedOn("from_ip"))
        {
            if (par("udpEncapsEnabled"))
            {
                std::cout<<"Size of SCTPMSG="<<sctpmsg->getByteLength()<<"\n";
                UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(msg->removeControlInfo());
                srcAddr = ctrl->getSrcAddr();
                destAddr = ctrl->getDestAddr();
                std::cout<<"controlInfo srcAddr="<<srcAddr<<"  destAddr="<<destAddr<<"\n";
                std::cout<<"VTag="<<sctpmsg->getTag()<<"\n";
            }
            else
            {
                IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo *>(msg->removeControlInfo());
                IPv4Datagram *datagram = controlInfo->removeOrigDatagram();
                delete datagram;
                sctpEV3<<"controlInfo srcAddr="<<controlInfo->getSrcAddr()<<"   destAddr="<<controlInfo->getDestAddr()<<"\n";
                srcAddr = controlInfo->getSrcAddr();
                destAddr = controlInfo->getDestAddr();
            }
        }
        else
        {
            IPv6ControlInfo *controlInfoV6 = check_and_cast<IPv6ControlInfo *>(msg->removeControlInfo());
            srcAddr = controlInfoV6->getSrcAddr();
            destAddr = controlInfoV6->getDestAddr();
        }


        sctpEV3<<"srcAddr="<<srcAddr<<" destAddr="<<destAddr<<"\n";
        if (sctpmsg->getBitLength()>(SCTP_COMMON_HEADER*8))
        {
            if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT || ((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT_ACK )
                findListen = true;

            SCTPAssociation *assoc = findAssocForMessage(srcAddr, destAddr, sctpmsg->getSrcPort(), sctpmsg->getDestPort(), findListen);
            if (!assoc && sctpConnMap.size()>0)
                assoc = findAssocWithVTag(sctpmsg->getTag(), sctpmsg->getSrcPort(), sctpmsg->getDestPort());
            if (!assoc)
            {
                sctpEV3<<"no assoc found msg="<<sctpmsg->getName()<<"\n";
                if (bitError)
                {
                    delete sctpmsg;
                    return;
                }
                if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==SHUTDOWN_ACK)
                    sendShutdownCompleteFromMain(sctpmsg, destAddr, srcAddr);
                else if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()!=ABORT &&
                    ((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()!=SHUTDOWN_COMPLETE)
                {
                    sendAbortFromMain(sctpmsg, destAddr, srcAddr);
                }
                delete sctpmsg;
            }
            else
            {
                bool ret = assoc->processSCTPMessage(sctpmsg, srcAddr, destAddr);
                if (!ret)
                {
                    sctpEV3<<"SCTPMain:: removeAssociation \n";
                    removeAssociation(assoc);
                    delete sctpmsg;
                }
                else
                {
                    delete sctpmsg;
                }
            }
        }
        else
        {
            delete sctpmsg;
        }
    }
    else // must be from app
    {
        sctpEV3<<"must be from app\n";
        SCTPCommand *controlInfo = check_and_cast<SCTPCommand *>(msg->getControlInfo());

        int32 appGateIndex;
        if (controlInfo->getGate()!=-1)
            appGateIndex = controlInfo->getGate();
        else
            appGateIndex = msg->getArrivalGate()->getIndex();
        int32 assocId = controlInfo->getAssocId();
        sctpEV3<<"msg arrived from app for assoc "<<assocId<<"\n";
        SCTPAssociation *assoc = findAssocForApp(appGateIndex, assocId);

        if (!assoc)
        {
            sctpEV3 << "no assoc found. msg="<<msg->getName()<<" number of assocs = "<<assocList.size()<<"\n";

            if (strcmp(msg->getName(), "PassiveOPEN")==0 || strcmp(msg->getName(), "Associate")==0)
            {
                if (assocList.size()>0)
                {
                    assoc = NULL;
                    SCTPOpenCommand* open = check_and_cast<SCTPOpenCommand*>(controlInfo);
                    sctpEV3<<"Looking for assoc with remoteAddr="<<open->getRemoteAddr()<<", remotePort="<<open->getRemotePort()<<", localPort="<<open->getLocalPort()<<"\n";
                    for (std::list<SCTPAssociation*>::iterator iter=assocList.begin(); iter!=assocList.end(); iter++)
                    {
                        sctpEV3<<"remoteAddr="<<(*iter)->remoteAddr<<", remotePort="<<(*iter)->remotePort<<", localPort="<<(*iter)->localPort<<"\n";
                        if ((*iter)->remoteAddr == open->getRemoteAddr() && (*iter)->localPort==open->getLocalPort() && (*iter)->remotePort==open->getRemotePort())
                        {
                            assoc = (*iter);
                            break;
                        }
                    }
                }
                if (assoc==NULL)
                {
                    assoc = new SCTPAssociation(this, appGateIndex, assocId);

                    AppConnKey key;
                    key.appGateIndex = appGateIndex;
                    key.assocId = assocId;
                    sctpAppConnMap[key] = assoc;
                    sctpEV3 << "SCTP association created for appGateIndex " << appGateIndex << " and assoc "<<assocId<<"\n";
                    bool ret = assoc->processAppCommand(PK(msg));
                    if (!ret)
                    {
                        removeAssociation(assoc);
                    }
                }
            }
        }
        else
        {
            sctpEV3<<"assoc found\n";
            bool ret = assoc->processAppCommand(PK(msg));

            if (!ret)
                removeAssociation(assoc);
        }
        delete msg;
    }
    if (ev.isGUI())
        updateDisplayString();
}

void SCTP::sendAbortFromMain(SCTPMessage* sctpmsg, IPvXAddress srcAddr, IPvXAddress destAddr)
{
    SCTPMessage *msg = new SCTPMessage();

    sctpEV3<<"\n\nSCTPMain:sendABORT \n";

    msg->setSrcPort(sctpmsg->getDestPort());
    msg->setDestPort(sctpmsg->getSrcPort());
    msg->setBitLength(SCTP_COMMON_HEADER*8);
    msg->setChecksumOk(true);

    SCTPAbortChunk* abortChunk = new SCTPAbortChunk("ABORT");
    abortChunk->setChunkType(ABORT);
    if (sctpmsg->getChunksArraySize()>0 && ((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT)
    {

        SCTPInitChunk* initChunk = check_and_cast<SCTPInitChunk *>(sctpmsg->getChunks(0));
        abortChunk->setT_Bit(0);
        msg->setTag(initChunk->getInitTag());
    }
    else
    {
        abortChunk->setT_Bit(1);
        msg->setTag(sctpmsg->getTag());
    }
    abortChunk->setBitLength(SCTP_ABORT_CHUNK_LENGTH*8);
    msg->addChunk(abortChunk);
    if ((bool)par("udpEncapsEnabled"))
    {
        std::cout<<"VTag="<<msg->getTag()<<"\n";
        udpSocket.sendTo(msg, destAddr, SCTP_UDP_PORT);
    }
    else
    {
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
        controlInfo->setProtocol(IP_PROT_SCTP);
        controlInfo->setSrcAddr(srcAddr.get4());
        controlInfo->setDestAddr(destAddr.get4());
        msg->setControlInfo(controlInfo);
        send(msg, "to_ip");
    }
}

void SCTP::sendShutdownCompleteFromMain(SCTPMessage* sctpmsg, IPvXAddress srcAddr, IPvXAddress destAddr)
{
    SCTPMessage *msg = new SCTPMessage();

    sctpEV3<<"\n\nSCTP:sendABORT \n";

    msg->setSrcPort(sctpmsg->getDestPort());
    msg->setDestPort(sctpmsg->getSrcPort());
    msg->setBitLength(SCTP_COMMON_HEADER*8);
    msg->setChecksumOk(true);

    SCTPShutdownCompleteChunk* scChunk = new SCTPShutdownCompleteChunk("SHUTDOWN_COMPLETE");
    scChunk->setChunkType(SHUTDOWN_COMPLETE);
    scChunk->setTBit(1);
    msg->setTag(sctpmsg->getTag());

    scChunk->setBitLength(SCTP_SHUTDOWN_ACK_LENGTH*8);
    msg->addChunk(scChunk);
    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setProtocol(IP_PROT_SCTP);
    controlInfo->setSrcAddr(srcAddr.get4());
    controlInfo->setDestAddr(destAddr.get4());
    msg->setControlInfo(controlInfo);
    send(msg, "to_ip");
}


void SCTP::updateDisplayString()
{
#if 0
    if (ev.disable_tracing)
    {
        // in express mode, we don't bother to update the display
        // (std::map's iteration is not very fast if map is large)
        getDisplayString().setTagArg("t", 0, "");
        return;
    }

    //char buf[40];
    //sprintf(buf,"%d conns", sctpAppConnMap.size());
    //displayString().setTagArg("t",0,buf);

    int32 numCLOSED=0, numLISTEN=0, numSYN_SENT=0, numSYN_RCVD=0,
       numESTABLISHED=0, numCLOSE_WAIT=0, numLAST_ACK=0, numFIN_WAIT_1=0,
       numFIN_WAIT_2=0, numCLOSING=0, numTIME_WAIT=0;

    for (SctpAppConnMap::iterator i=sctpAppConnMap.begin(); i!=sctpAppConnMap.end(); ++i)
    {
       int32 state = (*i).second->getFsmState();
       switch(state)
       {
           // case SCTP_S_INIT:           numINIT++; break;
           case SCTP_S_CLOSED:            numCLOSED++; break;
           case SCTP_S_COOKIE_WAIT:       numLISTEN++; break;
           case SCTP_S_COOKIE_ECHOED:     numSYN_SENT++; break;
           case SCTP_S_ESTABLISHED:       numESTABLISHED++; break;
           case SCTP_S_SHUTDOWN_PENDING:  numCLOSE_WAIT++; break;
           case SCTP_S_SHUTDOWN_SENT:     numLAST_ACK++; break;
           case SCTP_S_SHUTDOWN_RECEIVED: numFIN_WAIT_1++; break;
           case SCTP_S_SHUTDOWN_ACK_SENT: numFIN_WAIT_2++; break;
       }
    }
    char buf2[300];
    buf2[0] = '\0';
    if (numCLOSED>0)     sprintf(buf2+strlen(buf2), "closed:%d ", numCLOSED);
    if (numLISTEN>0)     sprintf(buf2+strlen(buf2), "listen:%d ", numLISTEN);
    if (numSYN_SENT>0)   sprintf(buf2+strlen(buf2), "syn_sent:%d ", numSYN_SENT);
    if (numSYN_RCVD>0)   sprintf(buf2+strlen(buf2), "syn_rcvd:%d ", numSYN_RCVD);
    if (numESTABLISHED>0) sprintf(buf2+strlen(buf2),"estab:%d ", numESTABLISHED);
    if (numCLOSE_WAIT>0) sprintf(buf2+strlen(buf2), "close_wait:%d ", numCLOSE_WAIT);
    if (numLAST_ACK>0)   sprintf(buf2+strlen(buf2), "last_ack:%d ", numLAST_ACK);
    if (numFIN_WAIT_1>0) sprintf(buf2+strlen(buf2), "fin_wait_1:%d ", numFIN_WAIT_1);
    if (numFIN_WAIT_2>0) sprintf(buf2+strlen(buf2), "fin_wait_2:%d ", numFIN_WAIT_2);
    if (numCLOSING>0)    sprintf(buf2+strlen(buf2), "closing:%d ", numCLOSING);
    if (numTIME_WAIT>0)  sprintf(buf2+strlen(buf2), "time_wait:%d ", numTIME_WAIT);
    getDisplayString().setTagArg("t", 0, buf2);
#endif
}

SCTPAssociation *SCTP::findAssocWithVTag(uint32 peerVTag, uint32 remotePort, uint32 localPort)
{

    printVTagMap();
    sctpEV3<<"findAssocWithVTag: peerVTag="<<peerVTag<<" srcPort="<<remotePort<<"    destPort="<<localPort<<"\n";
    printInfoConnMap();

    // try with fully qualified SockPair
    for (SctpVTagMap::iterator i=sctpVTagMap.begin(); i!=sctpVTagMap.end(); i++)
    {
        if ((i->second.peerVTag==peerVTag && i->second.localPort==localPort
            && i->second.remotePort==remotePort)
            || (i->second.localVTag==peerVTag && i->second.localPort==localPort
            && i->second.remotePort==remotePort))
            return getAssoc(i->first);
    }
    return NULL;
}

SCTPAssociation *SCTP::findAssocForMessage(IPvXAddress srcAddr, IPvXAddress destAddr, uint32 srcPort, uint32 destPort, bool findListen)
{
    SockPair key;

    key.localAddr = destAddr;
    key.remoteAddr = srcAddr;
    key.localPort = destPort;
    key.remotePort = srcPort;
    SockPair save = key;
    sctpEV3<<"findAssocForMessage: srcAddr="<<destAddr<<" destAddr="<<srcAddr<<" srcPort="<<destPort<<"  destPort="<<srcPort<<"\n";
    printInfoConnMap();

    // try with fully qualified SockPair
    SctpConnMap::iterator i;
    i = sctpConnMap.find(key);
    if (i!=sctpConnMap.end())
        return i->second;


    // try with localAddr missing (only localPort specified in passive/active open)
    key.localAddr.set("0.0.0.0");

    i = sctpConnMap.find(key);
    if (i!=sctpConnMap.end())
    {

        //sctpEV3<<"try with localAddr missing (only localPort specified in passive/active open)\n";

        return i->second;
    }

    if (findListen==true)
    {
        /*key = save;
        key.localPort = 0;
        key.localAddr.set("0.0.0.0");
        i = sctpConnMap.find(key);
        if (i!=sctpConnMap.end())
        {
            return i->second;
        }*/


        // try fully qualified local socket + blank remote socket (for incoming SYN)
        key = save;
        key.remoteAddr.set("0.0.0.0");
        key.remotePort = 0;
        i = sctpConnMap.find(key);
        if (i!=sctpConnMap.end())
        {

            //sctpEV3<<"try fully qualified local socket + blank remote socket \n";

            return i->second;
        }


        // try with blank remote socket, and localAddr missing (for incoming SYN)
        key.localAddr.set("0.0.0.0");
        i = sctpConnMap.find(key);
        if (i!=sctpConnMap.end())
        {

            //sctpEV3<<"try with blank remote socket, and localAddr missing \n";

            return i->second;
        }
    }
    // given up

    sctpEV3<<"giving up on trying to find assoc for localAddr="<<srcAddr<<" remoteAddr="<<destAddr<<" localPort="<<srcPort<<" remotePort="<<destPort<<"\n";
    return NULL;
}

SCTPAssociation *SCTP::findAssocForApp(int32 appGateIndex, int32 assocId)
{
    AppConnKey key;
    key.appGateIndex = appGateIndex;
    key.assocId = assocId;
    sctpEV3<<"findAssoc for appGateIndex "<<appGateIndex<<" and assoc "<<assocId<<"\n";
    SctpAppConnMap::iterator i = sctpAppConnMap.find(key);
    return i==sctpAppConnMap.end() ? NULL : i->second;
}

uint16 SCTP::getEphemeralPort()
{
    if (nextEphemeralPort==5000)
        error("Ephemeral port range 1024..4999 exhausted (email SCTP model "
                "author that he should implement reuse of ephemeral ports!!!)");
    return nextEphemeralPort++;
}

void SCTP::updateSockPair(SCTPAssociation *conn, IPvXAddress localAddr, IPvXAddress remoteAddr, int32 localPort, int32 remotePort)
{
    SockPair key;
    sctpEV3<<"updateSockPair:   localAddr: "<<localAddr<<"   remoteAddr="<<remoteAddr<<"    localPort="<<localPort<<" remotePort="<<remotePort<<"\n";

    key.localAddr = (conn->localAddr = localAddr);
    key.remoteAddr = (conn->remoteAddr = remoteAddr);
    key.localPort = conn->localPort = localPort;
    key.remotePort = conn->remotePort = remotePort;

    for (SctpConnMap::iterator i=sctpConnMap.begin(); i!=sctpConnMap.end(); i++)
    {
        if (i->second == conn)
        {
            sctpConnMap.erase(i);
            break;
        }
    }

    sctpEV3<<"updateSockPair conn="<<conn<<"    localAddr="<<key.localAddr<<"            remoteAddr="<<key.remoteAddr<<"     localPort="<<key.localPort<<"  remotePort="<<remotePort<<"\n";

    sctpConnMap[key] = conn;
    sizeConnMap = sctpConnMap.size();
    //sctpEV3<<"number of connections="<<sctpConnMap.size()<<"\n";
    sctpEV3<<"assoc inserted in sctpConnMap\n";
    printInfoConnMap();
}

void SCTP::addLocalAddress(SCTPAssociation *conn, IPvXAddress address)
{

        //sctpEV3<<"Add local address: "<<address<<"\n";

        SockPair key;

        key.localAddr = conn->localAddr;
        key.remoteAddr = conn->remoteAddr;
        key.localPort = conn->localPort;
        key.remotePort = conn->remotePort;

        SctpConnMap::iterator i = sctpConnMap.find(key);
        if (i!=sctpConnMap.end())
        {
            ASSERT(i->second==conn);
            if (key.localAddr.isUnspecified())
            {
                sctpConnMap.erase(i);
                sizeConnMap--;
            }
        }
        else
            sctpEV3<<"no actual sockPair found\n";
        key.localAddr = address;
        //key.localAddr = address.get4().getInt();
        // //sctpEV3<<"laddr="<<key.localAddr<<"    lp="<<key.localPort<<"  raddr="<<key.remoteAddr<<" rPort="<<key.remotePort<<"\n";
        sctpConnMap[key] = conn;
        sizeConnMap = sctpConnMap.size();
        sctpEV3<<"number of connections="<<sizeConnMap<<"\n";

        printInfoConnMap();
}

void SCTP::addLocalAddressToAllRemoteAddresses(SCTPAssociation *conn, IPvXAddress address, std::vector<IPvXAddress> remAddresses)
{

        //sctpEV3<<"Add local address: "<<address<<"\n";

        SockPair key;

        for (AddressVector::iterator i=remAddresses.begin(); i!=remAddresses.end(); ++i)
        {
            //sctpEV3<<"remote address="<<(*i)<<"\n";
            key.localAddr = conn->localAddr;
            key.remoteAddr = (*i);
            key.localPort = conn->localPort;
            key.remotePort = conn->remotePort;

            SctpConnMap::iterator j = sctpConnMap.find(key);
            if (j!=sctpConnMap.end())
            {
            ASSERT(j->second==conn);
            if (key.localAddr.isUnspecified())
                    {
                    sctpConnMap.erase(j);
                    sizeConnMap--;
                }

            }
            else
                sctpEV3<<"no actual sockPair found\n";
            key.localAddr = address;
            sctpConnMap[key] = conn;

            sizeConnMap++;
            sctpEV3<<"number of connections="<<sctpConnMap.size()<<"\n";

            printInfoConnMap();
        }
}

void SCTP::removeLocalAddressFromAllRemoteAddresses(SCTPAssociation *conn, IPvXAddress address, std::vector<IPvXAddress> remAddresses)
{

        //sctpEV3<<"Remove local address: "<<address<<"\n";

        SockPair key;

        for (AddressVector::iterator i=remAddresses.begin(); i!=remAddresses.end(); ++i)
        {
            //sctpEV3<<"remote address="<<(*i)<<"\n";
            key.localAddr = address;
            key.remoteAddr = (*i);
            key.localPort = conn->localPort;
            key.remotePort = conn->remotePort;

            SctpConnMap::iterator j = sctpConnMap.find(key);
            if (j!=sctpConnMap.end())
            {
                ASSERT(j->second==conn);
                sctpConnMap.erase(j);
                sizeConnMap--;
            }
            else
                sctpEV3<<"no actual sockPair found\n";

            //sctpEV3<<"number of connections="<<sctpConnMap.size()<<"\n";

            printInfoConnMap();
        }
}

void SCTP::removeRemoteAddressFromAllConnections(SCTPAssociation *conn, IPvXAddress address, std::vector<IPvXAddress> locAddresses)
{

        //sctpEV3<<"Remove remote address: "<<address<<"\n";

        SockPair key;

        for (AddressVector::iterator i=locAddresses.begin(); i!=locAddresses.end(); i++)
        {
            //sctpEV3<<"local address="<<(*i)<<"\n";
            key.localAddr = (*i);
            key.remoteAddr = address;
            key.localPort = conn->localPort;
            key.remotePort = conn->remotePort;

            SctpConnMap::iterator j = sctpConnMap.find(key);
            if (j!=sctpConnMap.end())
            {
                ASSERT(j->second==conn);
                sctpConnMap.erase(j);
                sizeConnMap--;
            }
            else
                sctpEV3<<"no actual sockPair found\n";

            //sctpEV3<<"number of connections="<<sctpConnMap.size()<<"\n";

            printInfoConnMap();
        }
}

void SCTP::addRemoteAddress(SCTPAssociation *conn, IPvXAddress localAddress, IPvXAddress remoteAddress)
{

    sctpEV3<<"Add remote Address: "<<remoteAddress<<" to local Address "<<localAddress<<"\n";

    SockPair key;
    key.localAddr = localAddress;
    key.remoteAddr = remoteAddress;
    key.localPort = conn->localPort;
    key.remotePort = conn->remotePort;

    SctpConnMap::iterator i = sctpConnMap.find(key);
    if (i!=sctpConnMap.end())
    {
        ASSERT(i->second==conn);
    }
    else
    {

        //sctpEV3<<"no actual sockPair found\n";

        sctpConnMap[key] = conn;
        sizeConnMap++;
    }

    //sctpEV3<<"number of connections="<<sctpConnMap.size()<<"\n";
    printInfoConnMap();
}

void SCTP::addForkedAssociation(SCTPAssociation *assoc, SCTPAssociation *newAssoc, IPvXAddress localAddr, IPvXAddress remoteAddr, int32 localPort, int32 remotePort)
{
    SockPair keyAssoc;

    ev<<"addForkedConnection assocId="<<assoc->assocId<<"    newId="<<newAssoc->assocId<<"\n";

    for (SctpConnMap::iterator j=sctpConnMap.begin(); j!=sctpConnMap.end(); ++j)
        if (assoc->assocId==j->second->assocId)
            keyAssoc = j->first;
    // update conn's socket pair, and register newConn (which'll keep LISTENing)
    updateSockPair(assoc, localAddr, remoteAddr, localPort, remotePort);
    updateSockPair(newAssoc, keyAssoc.localAddr, keyAssoc.remoteAddr, keyAssoc.localPort, keyAssoc.remotePort);
    // conn will get a new assocId...
    AppConnKey key;
    key.appGateIndex = assoc->appGateIndex;
    key.assocId = assoc->assocId;
    sctpAppConnMap.erase(key);
    key.assocId = assoc->assocId = getNewConnId();
    sctpAppConnMap[key] = assoc;

    // ...and newConn will live on with the old assocId
    key.appGateIndex = newAssoc->appGateIndex;
    key.assocId = newAssoc->assocId;
    sctpAppConnMap[key] = newAssoc;
    /*ev<<"assocId="<<assoc->assocId<<" remoteAddr="<<assoc->remoteAddr<<"\n";
    assoc->removeOldPath();*/
    printInfoConnMap();
}



void SCTP::removeAssociation(SCTPAssociation *conn)
{
    bool            ok = false;
    bool            find = false;
    const int32 id = conn->assocId;

    sctpEV3 << "Deleting SCTP connection " << conn << " id= "<< id << endl;

    printInfoConnMap();
    if (sizeConnMap > 0) {
        AssocStatMap::iterator assocStatMapIterator = assocStatMap.find(conn->assocId);
        if (assocStatMapIterator != assocStatMap.end()) {
            assocStatMapIterator->second.stop = simulation.getSimTime();
            assocStatMapIterator->second.lifeTime = assocStatMapIterator->second.stop - assocStatMapIterator->second.start;
            assocStatMapIterator->second.throughput = assocStatMapIterator->second.ackedBytes*8 / assocStatMapIterator->second.lifeTime.dbl();
        }
        while (!ok) {
            if (sizeConnMap == 0) {
                ok = true;
            }
            else {
                for (SctpConnMap::iterator sctpConnMapIterator = sctpConnMap.begin();
                      sctpConnMapIterator != sctpConnMap.end(); sctpConnMapIterator++) {
                    if (sctpConnMapIterator->second != NULL) {
                        SCTPAssociation* assoc = sctpConnMapIterator->second;
                        if (assoc->assocId == conn->assocId) {
                            if (assoc->T1_InitTimer) {
                                assoc->stopTimer(assoc->T1_InitTimer);
                            }
                            if (assoc->T2_ShutdownTimer) {
                                assoc->stopTimer(assoc->T2_ShutdownTimer);
                            }
                            if (assoc->T5_ShutdownGuardTimer) {
                                assoc->stopTimer(assoc->T5_ShutdownGuardTimer);
                            }
                            if (assoc->SackTimer) {
                                assoc->stopTimer(assoc->SackTimer);
                            }
                            sctpConnMap.erase(sctpConnMapIterator);
                            sizeConnMap--;
                            find = true;
                            break;
                        }
                    }
                }
            }

            if (!find) {
                ok = true;
            }
            else {
                find = false;
            }
        }
    }

    // T.D. 26.11.09: Write statistics
    char str[128];
    for (SCTPAssociation::SCTPPathMap::iterator pathMapIterator = conn->sctpPathMap.begin();
          pathMapIterator != conn->sctpPathMap.end(); pathMapIterator++) {
        const SCTPPathVariables* path = pathMapIterator->second;
        snprintf((char*)&str, sizeof(str), "Number of Fast Retransmissions %d:%s",
                 conn->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfFastRetransmissions);
        snprintf((char*)&str, sizeof(str), "Number of Timer-Based Retransmissions %d:%s",
                 conn->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfTimerBasedRetransmissions);
        snprintf((char*)&str, sizeof(str), "Number of Heartbeats Sent %d:%s",
                 conn->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatsSent);
        snprintf((char*)&str, sizeof(str), "Number of Heartbeats Received %d:%s",
                 conn->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatsRcvd);
        snprintf((char*)&str, sizeof(str), "Number of Heartbeat ACKs Sent %d:%s",
                 conn->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatAcksSent);
        snprintf((char*)&str, sizeof(str), "Number of Heartbeat ACKs Received %d:%s",
                 conn->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatAcksRcvd);
    }


    conn->removePath();
    conn->deleteStreams();

    // TD 20.11.09: Chunks may be in the transmission and retransmission queues simultaneously.
    //                   Remove entry from transmission queue if it is already in the retransmission queue.
    for (SCTPQueue::PayloadQueue::iterator i = conn->getRetransmissionQueue()->payloadQueue.begin();
          i != conn->getRetransmissionQueue()->payloadQueue.end(); i++) {
        SCTPQueue::PayloadQueue::iterator j = conn->getTransmissionQueue()->payloadQueue.find(i->second->tsn);
        if (j != conn->getTransmissionQueue()->payloadQueue.end()) {
            conn->getTransmissionQueue()->payloadQueue.erase(j);
        }
    }
     // TD 20.11.09: Now, both queues can be safely deleted.
    delete conn->getRetransmissionQueue();
    delete conn->getTransmissionQueue();

    AppConnKey key;
    key.appGateIndex = conn->appGateIndex;
    key.assocId = conn->assocId;
    sctpAppConnMap.erase(key);
    assocList.remove(conn);
    delete conn;
}

SCTPAssociation* SCTP::getAssoc(int32 assocId)
{
    for (SctpAppConnMap::iterator i = sctpAppConnMap.begin(); i!=sctpAppConnMap.end(); i++)
    {
        if (i->first.assocId==assocId)
            return i->second;
    }
    return NULL;
}

void SCTP::finish()
{
    SctpConnMap::iterator connMapIterator = sctpConnMap.begin();
    while (connMapIterator != sctpConnMap.end()) {
        removeAssociation(connMapIterator->second);
        connMapIterator = sctpConnMap.begin();
    }
    ev << getFullPath() << ": finishing SCTP with "
        << sctpConnMap.size() << " connections open." << endl;

    for (AssocStatMap::const_iterator iterator = assocStatMap.begin();
          iterator != assocStatMap.end(); iterator++) {
        const SCTP::AssocStat& assoc = iterator->second;

        ev << "Association " << assoc.assocId << ": started at " << assoc.start
            << " and finished at " << assoc.stop << " --> lifetime: " << assoc.lifeTime << endl;
        ev << "Association " << assoc.assocId << ": sent bytes=" << assoc.sentBytes
            << ", acked bytes=" << assoc.ackedBytes<< ", throughput=" << assoc.throughput<< " bit/s" << endl;
        ev << "Association " << assoc.assocId << ": transmitted Bytes="
            << assoc.transmittedBytes<< ", retransmitted Bytes=" << assoc.transmittedBytes-assoc.ackedBytes<< endl;
        ev << "Association " << assoc.assocId << ": number of Fast RTX="
            << assoc.numFastRtx << ", number of Timer-Based RTX=" << assoc.numT3Rtx
            << ", path failures=" << assoc.numPathFailures<< ", ForwardTsns=" << assoc.numForwardTsn<< endl;
        ev << "AllMessages=" <<numPacketsReceived<< " BadMessages=" <<numPacketsDropped<< endl;

        recordScalar("Association Lifetime", assoc.lifeTime);
        recordScalar("Acked Bytes",          assoc.ackedBytes);
        recordScalar("Throughput [bit/s]",   assoc.throughput);
        recordScalar("Transmitted Bytes",    assoc.transmittedBytes);
        recordScalar("Fast RTX",                 assoc.numFastRtx);
        recordScalar("Timer-Based RTX",      assoc.numT3Rtx);
        recordScalar("Duplicate Acks",       assoc.numDups);
        recordScalar("Packets Received",         numPacketsReceived);
        recordScalar("Packets Dropped",      numPacketsDropped);

    }
}
