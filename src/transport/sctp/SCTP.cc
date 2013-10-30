//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
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
#include "IPSocket.h"
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

int32 SCTP::nextAssocId = 0;


void SCTP::printInfoAssocMap()
{
    SCTPAssociation* assoc;
    SockPair      key;
    sctpEV3<<"Number of Assocs: "<<sizeAssocMap<<"\n";
    if (sizeAssocMap>0)
    {
        for (SctpAssocMap::iterator i = sctpAssocMap.begin(); i!=sctpAssocMap.end(); ++i)
        {
            assoc = i->second;
            key = i->first;

                sctpEV3<<"assocId: "<<assoc->assocId<<"  assoc: "<<assoc<<" src: "<<Address(key.localAddr)<<" dst: "<<Address(key.remoteAddr)<<" lPort: "<<key.localPort<<" rPort: "<<key.remotePort<<"\n";

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

void SCTP::initialize(int stage)
{
    InetSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        this->auth = (bool)par("auth");
        this->pktdrop = (bool)par("packetDrop");
        this->sackNow = (bool)par("sackNow");
        numPktDropReports = 0;
        numPacketsReceived = 0;
        numPacketsDropped = 0;
        sizeAssocMap = 0;
        nextEphemeralPort = (uint16)(intrand(10000) + 30000);

        cModule *netw = simulation.getSystemModule();
        testing = netw->hasPar("testing") && netw->par("testing").boolValue();
        if (testing) {
        }
        if (netw->hasPar("testTimeout"))
        {
            testTimeout = (simtime_t)netw->par("testTimeout");
        }
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER)
    {
        IPSocket socket(gate("to_ip"));
        socket.registerProtocol(IP_PROT_SCTP);
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER_2)
    {
        if (par("udpEncapsEnabled").boolValue())
        {
            bindPortForUDP();
        }
    }
}


SCTP::~SCTP()
{
    sctpEV3<<"delete SCTPMain\n";
    if (!(sctpAppAssocMap.empty()))
    {
        sctpEV3<<"clear appConnMap ptr="<<&sctpAppAssocMap<<"\n";
        sctpAppAssocMap.clear();
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
    Address destAddr;
    Address srcAddr;
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
        printInfoAssocMap();
        if (!dynamic_cast<SCTPMessage *>(msg))
        {
            sctpEV3<<"no sctp message, delete it\n";
            delete msg;
            return;
        }
        SCTPMessage *sctpmsg = check_and_cast<SCTPMessage *>(msg);

        numPacketsReceived++;

        if (!pktdrop && (sctpmsg->hasBitError() || !(sctpmsg->getChecksumOk()))) {
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
                sctpEV3<<"Size of SCTPMSG="<<sctpmsg->getByteLength()<<"\n";
                UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(msg->removeControlInfo());
                srcAddr = ctrl->getSrcAddr();
                destAddr = ctrl->getDestAddr();
                sctpEV3<<"controlInfo srcAddr="<<srcAddr<<"  destAddr="<<destAddr<<"\n";
                sctpEV3<<"VTag="<<sctpmsg->getTag()<<"\n";
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
        if (sctpmsg->getByteLength()>(SCTP_COMMON_HEADER))
        {
            if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT || ((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT_ACK )
                findListen = true;

            SCTPAssociation *assoc = findAssocForMessage(srcAddr, destAddr, sctpmsg->getSrcPort(), sctpmsg->getDestPort(), findListen);
            if (!assoc && sctpAssocMap.size()>0 && (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==ERRORTYPE 
                || (sctpmsg->getChunksArraySize() > 1 && 
                (((SCTPChunk*)(sctpmsg->getChunks(1)))->getChunkType()==ASCONF || ((SCTPChunk*)(sctpmsg->getChunks(1)))->getChunkType()==ASCONF_ACK)))) {
                assoc = findAssocWithVTag(sctpmsg->getTag(), sctpmsg->getSrcPort(), sctpmsg->getDestPort());
            }
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
                sctpEV3 << "assoc " << assoc->assocId << "found\n";
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
                if (assocList.size() == 0 || assoc==NULL)
                {
                    assoc = new SCTPAssociation(this, appGateIndex, assocId);

                    AppAssocKey key;
                    key.appGateIndex = appGateIndex;
                    key.assocId = assocId;
                    sctpAppAssocMap[key] = assoc;
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

void SCTP::sendAbortFromMain(SCTPMessage* sctpmsg, Address srcAddr, Address destAddr)
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
        sctpEV3<<"VTag="<<msg->getTag()<<"\n";
        udpSocket.sendTo(msg, destAddr, SCTP_UDP_PORT);
    }
    else
    {
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
        controlInfo->setProtocol(IP_PROT_SCTP);
        controlInfo->setSrcAddr(srcAddr.toIPv4());
        controlInfo->setDestAddr(destAddr.toIPv4());
        msg->setControlInfo(controlInfo);
        sendSync(msg, "to_ip");
    }
}

void SCTP::sendShutdownCompleteFromMain(SCTPMessage* sctpmsg, Address srcAddr, Address destAddr)
{
    SCTPMessage *msg = new SCTPMessage();

    sctpEV3<<"\n\nSCTP:sendShutdownCompleteFromMain \n";

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
    controlInfo->setSrcAddr(srcAddr.toIPv4());
    controlInfo->setDestAddr(destAddr.toIPv4());
    msg->setControlInfo(controlInfo);
    sendSync(msg, "to_ip");
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
    printInfoAssocMap();

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

SCTPAssociation *SCTP::findAssocForMessage(Address srcAddr, Address destAddr, uint32 srcPort, uint32 destPort, bool findListen)
{
    SockPair key;

    key.localAddr = destAddr;
    key.remoteAddr = srcAddr;
    key.localPort = destPort;
    key.remotePort = srcPort;
    SockPair save = key;
    sctpEV3<<"findAssocForMessage: srcAddr="<<destAddr<<" destAddr="<<srcAddr<<" srcPort="<<destPort<<"  destPort="<<srcPort<<"\n";
    printInfoAssocMap();

    // try with fully qualified SockPair
    SctpAssocMap::iterator i;
    i = sctpAssocMap.find(key);
    if (i!=sctpAssocMap.end())
        return i->second;


    // try with localAddr missing (only localPort specified in passive/active open)
    key.localAddr = Address();

    i = sctpAssocMap.find(key);
    if (i!=sctpAssocMap.end())
    {
        // try with localAddr missing (only localPort specified in passive/active open)
        return i->second;
    }

    if (findListen==true)
    {
        // try fully qualified local socket + blank remote socket (for incoming SYN)
        key = save;
        key.remoteAddr = Address();
        key.remotePort = 0;
        i = sctpAssocMap.find(key);
        if (i!=sctpAssocMap.end())
        {
            // try fully qualified local socket + blank remote socket
            return i->second;
        }

        // try with blank remote socket, and localAddr missing (for incoming SYN)
        key.localAddr = Address();
        i = sctpAssocMap.find(key);
        if (i!=sctpAssocMap.end())
        {
            // try with blank remote socket, and localAddr missing
            return i->second;
        }
    }
    // given up

    sctpEV3<<"giving up on trying to find assoc for localAddr="<<srcAddr<<" remoteAddr="<<destAddr<<" localPort="<<srcPort<<" remotePort="<<destPort<<"\n";
    return NULL;
}

SCTPAssociation *SCTP::findAssocForApp(int32 appGateIndex, int32 assocId)
{
    AppAssocKey key;
    key.appGateIndex = appGateIndex;
    key.assocId = assocId;
    sctpEV3<<"findAssoc for appGateIndex "<<appGateIndex<<" and assoc "<<assocId<<"\n";
    SctpAppAssocMap::iterator i = sctpAppAssocMap.find(key);
    return ((i == sctpAppAssocMap.end()) ? NULL : i->second);
}

uint16 SCTP::getEphemeralPort()
{
    if (nextEphemeralPort==5000)
        error("Ephemeral port range 1024..4999 exhausted (email SCTP model "
                "author that he should implement reuse of ephemeral ports!!!)");
    return nextEphemeralPort++;
}

void SCTP::updateSockPair(SCTPAssociation *assoc, Address localAddr, Address remoteAddr, int32 localPort, int32 remotePort)
{
    SockPair key;
    sctpEV3<<"updateSockPair:   localAddr: "<<localAddr<<"   remoteAddr="<<remoteAddr<<"    localPort="<<localPort<<" remotePort="<<remotePort<<"\n";

    key.localAddr = (assoc->localAddr = localAddr);
    key.remoteAddr = (assoc->remoteAddr = remoteAddr);
    key.localPort = assoc->localPort = localPort;
    key.remotePort = assoc->remotePort = remotePort;

    for (SctpAssocMap::iterator i=sctpAssocMap.begin(); i!=sctpAssocMap.end(); i++)
    {
        if (i->second == assoc)
        {
            sctpAssocMap.erase(i);
            break;
        }
    }

    sctpEV3<<"updateSockPair assoc="<<assoc<<"    localAddr="<<key.localAddr<<"            remoteAddr="<<key.remoteAddr<<"     localPort="<<key.localPort<<"  remotePort="<<remotePort<<"\n";

    sctpAssocMap[key] = assoc;
    sizeAssocMap = sctpAssocMap.size();
    sctpEV3<<"assoc inserted in sctpAssocMap\n";
    printInfoAssocMap();
}

void SCTP::addLocalAddress(SCTPAssociation *assoc, Address address)
{

        SockPair key;

        key.localAddr = assoc->localAddr;
        key.remoteAddr = assoc->remoteAddr;
        key.localPort = assoc->localPort;
        key.remotePort = assoc->remotePort;

        SctpAssocMap::iterator i = sctpAssocMap.find(key);
        if (i!=sctpAssocMap.end())
        {
            ASSERT(i->second==assoc);
            if (key.localAddr.isUnspecified())
            {
                sctpAssocMap.erase(i);
                sizeAssocMap--;
            }
        }
        else
            sctpEV3<<"no actual sockPair found\n";
        key.localAddr = address;
        sctpAssocMap[key] = assoc;
        sizeAssocMap = sctpAssocMap.size();
        sctpEV3<<"addLocalAddress " << address << " number of connections now="<<sizeAssocMap<<"\n";

        printInfoAssocMap();
}

void SCTP::addLocalAddressToAllRemoteAddresses(SCTPAssociation *assoc, Address address, std::vector<Address> remAddresses)
{

        SockPair key;

        for (AddressVector::iterator i=remAddresses.begin(); i!=remAddresses.end(); ++i)
        {
            //sctpEV3<<"remote address="<<(*i)<<"\n";
            key.localAddr = assoc->localAddr;
            key.remoteAddr = (*i);
            key.localPort = assoc->localPort;
            key.remotePort = assoc->remotePort;

            SctpAssocMap::iterator j = sctpAssocMap.find(key);
            if (j!=sctpAssocMap.end())
            {
            ASSERT(j->second==assoc);
            if (key.localAddr.isUnspecified())
                    {
                    sctpAssocMap.erase(j);
                    sizeAssocMap--;
                }

            }
            else
                sctpEV3<<"no actual sockPair found\n";
            key.localAddr = address;
            sctpAssocMap[key] = assoc;

            sizeAssocMap++;
            sctpEV3<<"number of connections="<<sctpAssocMap.size()<<"\n";

            printInfoAssocMap();
        }
}

void SCTP::removeLocalAddressFromAllRemoteAddresses(SCTPAssociation *assoc, Address address, std::vector<Address> remAddresses)
{

        SockPair key;

        for (AddressVector::iterator i=remAddresses.begin(); i!=remAddresses.end(); ++i)
        {
            key.localAddr = address;
            key.remoteAddr = (*i);
            key.localPort = assoc->localPort;
            key.remotePort = assoc->remotePort;

            SctpAssocMap::iterator j = sctpAssocMap.find(key);
            if (j!=sctpAssocMap.end())
            {
                ASSERT(j->second==assoc);
                sctpAssocMap.erase(j);
                sizeAssocMap--;
            }
            else
                sctpEV3<<"no actual sockPair found\n";

            printInfoAssocMap();
        }
}

void SCTP::removeRemoteAddressFromAllAssociations(SCTPAssociation *assoc, Address address, std::vector<Address> locAddresses)
{

        SockPair key;

        for (AddressVector::iterator i=locAddresses.begin(); i!=locAddresses.end(); i++)
        {
            key.localAddr = (*i);
            key.remoteAddr = address;
            key.localPort = assoc->localPort;
            key.remotePort = assoc->remotePort;

            SctpAssocMap::iterator j = sctpAssocMap.find(key);
            if (j!=sctpAssocMap.end())
            {
                ASSERT(j->second==assoc);
                sctpAssocMap.erase(j);
                sizeAssocMap--;
            }
            else
                sctpEV3<<"no actual sockPair found\n";

            printInfoAssocMap();
        }
}

bool SCTP::addRemoteAddress(SCTPAssociation *assoc, Address localAddress, Address remoteAddress)
{

    sctpEV3<<"Add remote Address: "<<remoteAddress<<" to local Address "<<localAddress<<"\n";

    SockPair key;
    key.localAddr = localAddress;
    key.remoteAddr = remoteAddress;
    key.localPort = assoc->localPort;
    key.remotePort = assoc->remotePort;

    SctpAssocMap::iterator i = sctpAssocMap.find(key);
    if (i!=sctpAssocMap.end())
    {
        ASSERT(i->second==assoc);
        return false;
    }
    else
    {
        sctpAssocMap[key] = assoc;
        sizeAssocMap++;
    }

    printInfoAssocMap();
    return true;
}

void SCTP::addForkedAssociation(SCTPAssociation *assoc, SCTPAssociation *newAssoc, Address localAddr, Address remoteAddr, int32 localPort, int32 remotePort)
{
    SockPair keyAssoc;

    EV<<"addForkedConnection assocId="<<assoc->assocId<<"    newId="<<newAssoc->assocId<<"\n";

    for (SctpAssocMap::iterator j=sctpAssocMap.begin(); j!=sctpAssocMap.end(); ++j)
        if (assoc->assocId==j->second->assocId)
            keyAssoc = j->first;
    // update assoc's socket pair, and register newAssoc (which'll keep LISTENing)
    updateSockPair(assoc, localAddr, remoteAddr, localPort, remotePort);
    updateSockPair(newAssoc, keyAssoc.localAddr, keyAssoc.remoteAddr, keyAssoc.localPort, keyAssoc.remotePort);

    // assoc will get a new assocId...
    AppAssocKey key;
    key.appGateIndex = assoc->appGateIndex;
    key.assocId = assoc->assocId;
    sctpAppAssocMap.erase(key);
    key.assocId = assoc->assocId = getNewAssocId();
    sctpAppAssocMap[key] = assoc;

    // ...and newAssoc will live on with the old assocId
    key.appGateIndex = newAssoc->appGateIndex;
    key.assocId = newAssoc->assocId;
    sctpAppAssocMap[key] = newAssoc;
    sizeAssocMap = sctpAssocMap.size();
    printInfoAssocMap();
}



void SCTP::removeAssociation(SCTPAssociation *assoc)
{
    bool            ok = false;
    bool            find = false;
    const int32 id = assoc->assocId;

    sctpEV3 << "Deleting SCTP connection " << assoc << " id= "<< id << endl;

    printInfoAssocMap();
    if (sizeAssocMap > 0) {
        AssocStatMap::iterator assocStatMapIterator = assocStatMap.find(assoc->assocId);
        if (assocStatMapIterator != assocStatMap.end()) {
            assocStatMapIterator->second.stop = simulation.getSimTime();
            assocStatMapIterator->second.lifeTime = assocStatMapIterator->second.stop - assocStatMapIterator->second.start;
            assocStatMapIterator->second.throughput = assocStatMapIterator->second.ackedBytes*8 / assocStatMapIterator->second.lifeTime.dbl();
        }
        while (!ok) {
            if (sizeAssocMap == 0) {
                ok = true;
            }
            else {
                for (SctpAssocMap::iterator sctpAssocMapIterator = sctpAssocMap.begin();
                      sctpAssocMapIterator != sctpAssocMap.end(); sctpAssocMapIterator++) {
                    if (sctpAssocMapIterator->second != NULL) {
                        SCTPAssociation* myAssoc = sctpAssocMapIterator->second;
                        if (myAssoc->assocId == assoc->assocId) {
                            if (myAssoc->T1_InitTimer) {
                                myAssoc->stopTimer(myAssoc->T1_InitTimer);
                            }
                            if (myAssoc->T2_ShutdownTimer) {
                                myAssoc->stopTimer(myAssoc->T2_ShutdownTimer);
                            }
                            if (myAssoc->T5_ShutdownGuardTimer) {
                                myAssoc->stopTimer(myAssoc->T5_ShutdownGuardTimer);
                            }
                            if (myAssoc->SackTimer) {
                                myAssoc->stopTimer(myAssoc->SackTimer);
                            }
                            if (myAssoc->StartAddIP) {
                                myAssoc->stopTimer(myAssoc->StartAddIP);
                            }
                            sctpAssocMap.erase(sctpAssocMapIterator);
                            sizeAssocMap--;
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

    // Write statistics
    char str[128];
    for (SCTPAssociation::SCTPPathMap::iterator pathMapIterator = assoc->sctpPathMap.begin();
          pathMapIterator != assoc->sctpPathMap.end(); pathMapIterator++) {
        const SCTPPathVariables* path = pathMapIterator->second;
        snprintf((char*)&str, sizeof(str), "Number of Fast Retransmissions %d:%s",
                 assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfFastRetransmissions);
        snprintf((char*)&str, sizeof(str), "Number of Timer-Based Retransmissions %d:%s",
                 assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfTimerBasedRetransmissions);
        snprintf((char*)&str, sizeof(str), "Number of Heartbeats Sent %d:%s",
                 assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatsSent);
        snprintf((char*)&str, sizeof(str), "Number of Heartbeats Received %d:%s",
                 assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatsRcvd);
        snprintf((char*)&str, sizeof(str), "Number of Heartbeat ACKs Sent %d:%s",
                 assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatAcksSent);
        snprintf((char*)&str, sizeof(str), "Number of Heartbeat ACKs Received %d:%s",
                 assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatAcksRcvd);
        snprintf((char*)&str, sizeof(str), "Number of Duplicates %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfDuplicates);
        snprintf((char*)&str, sizeof(str), "Number of Bytes received from %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfBytesReceived);
    }
    for (uint16 i = 0; i < assoc->inboundStreams; i++) {
        snprintf((char*)&str, sizeof(str), "Bytes received on stream %d of assoc %d",
                i, assoc->assocId);
                recordScalar(str, assoc->getState()->streamThroughput[i]); 
    } 
    assoc->removePath();
    assoc->deleteStreams();

    // TD 20.11.09: Chunks may be in the transmission and retransmission queues simultaneously.
    //                   Remove entry from transmission queue if it is already in the retransmission queue.
    for (SCTPQueue::PayloadQueue::iterator i = assoc->getRetransmissionQueue()->payloadQueue.begin();
          i != assoc->getRetransmissionQueue()->payloadQueue.end(); i++) {
        SCTPQueue::PayloadQueue::iterator j = assoc->getTransmissionQueue()->payloadQueue.find(i->second->tsn);
        if (j != assoc->getTransmissionQueue()->payloadQueue.end()) {
            assoc->getTransmissionQueue()->payloadQueue.erase(j);
        }
    }
     // TD 20.11.09: Now, both queues can be safely deleted.
    delete assoc->getRetransmissionQueue();
    delete assoc->getTransmissionQueue();

    AppAssocKey key;
    key.appGateIndex = assoc->appGateIndex;
    key.assocId = assoc->assocId;
    sctpAppAssocMap.erase(key);
    assocList.remove(assoc);
    delete assoc;
}

SCTPAssociation* SCTP::getAssoc(int32 assocId)
{
    for (SctpAppAssocMap::iterator i = sctpAppAssocMap.begin(); i!=sctpAppAssocMap.end(); i++)
    {
        if (i->first.assocId==assocId)
            return i->second;
    }
    return NULL;
}

void SCTP::finish()
{
    SctpAssocMap::iterator assocMapIterator = sctpAssocMap.begin();
    while (assocMapIterator != sctpAssocMap.end()) {
        removeAssociation(assocMapIterator->second);
        assocMapIterator = sctpAssocMap.begin();
    }
    EV << getFullPath() << ": finishing SCTP with "
        << sctpAssocMap.size() << " connections open." << endl;

    for (AssocStatMap::const_iterator iterator = assocStatMap.begin();
          iterator != assocStatMap.end(); iterator++) {
        const SCTP::AssocStat& assoc = iterator->second;

        EV << "Association " << assoc.assocId << ": started at " << assoc.start
            << " and finished at " << assoc.stop << " --> lifetime: " << assoc.lifeTime << endl;
        EV << "Association " << assoc.assocId << ": sent bytes=" << assoc.sentBytes
            << ", acked bytes=" << assoc.ackedBytes<< ", throughput=" << assoc.throughput<< " bit/s" << endl;
        EV << "Association " << assoc.assocId << ": transmitted Bytes="
            << assoc.transmittedBytes<< ", retransmitted Bytes=" << assoc.transmittedBytes-assoc.ackedBytes<< endl;
        EV << "Association " << assoc.assocId << ": number of Fast RTX="
            << assoc.numFastRtx << ", number of Timer-Based RTX=" << assoc.numT3Rtx
            << ", path failures=" << assoc.numPathFailures<< ", ForwardTsns=" << assoc.numForwardTsn<< endl;
        EV << "AllMessages=" <<numPacketsReceived<< " BadMessages=" <<numPacketsDropped<< endl;

        recordScalar("Association Lifetime", assoc.lifeTime);
        recordScalar("Acked Bytes", assoc.ackedBytes);
        recordScalar("Throughput [bit/s]", assoc.throughput);
        recordScalar("Transmitted Bytes", assoc.transmittedBytes);
        recordScalar("Fast RTX", assoc.numFastRtx);
        recordScalar("Timer-Based RTX", assoc.numT3Rtx);
        recordScalar("Duplicate Acks", assoc.numDups);
        recordScalar("Packets Received", numPacketsReceived);
        recordScalar("Packets Dropped", numPacketsDropped);
        recordScalar("Sum of R Gap Ranges", assoc.sumRGapRanges);
        recordScalar("Sum of NR Gap Ranges", assoc.sumNRGapRanges);
        recordScalar("Overfull SACKs", assoc.numOverfullSACKs);
        recordScalar("Drops Because New TSN Greater Than Highest TSN", assoc.numDropsBecauseNewTSNGreaterThanHighestTSN);
        recordScalar("Drops Because No Room In Buffer", assoc.numDropsBecauseNoRoomInBuffer);
        recordScalar("Chunks Reneged", assoc.numChunksReneged);
        recordScalar("sackPeriod", (simtime_t)par("sackPeriod"));
        recordScalar("Number of AUTH chunks sent", assoc.numAuthChunksSent);
        recordScalar("Number of AUTH chunks accepted", assoc.numAuthChunksAccepted);
        recordScalar("Number of AUTH chunks rejected", assoc.numAuthChunksRejected);
        recordScalar("Number of StreamReset requests sent", assoc.numResetRequestsSent);
        recordScalar("Number of StreamReset requests performed", assoc.numResetRequestsPerformed);
        if ((double)par("fairStart") > 0) {
            recordScalar("fair acked bytes", assoc.fairAckedBytes);
            recordScalar("fair start time", assoc.fairStart);
            recordScalar("fair stop time", assoc.fairStop);
            recordScalar("fair lifetime", assoc.fairLifeTime);
            recordScalar("fair throughput", assoc.fairThroughput);
        }
        recordScalar("Number of PacketDrop Reports", numPktDropReports);

        if (assoc.numEndToEndMessages > 0 && (assoc.cumEndToEndDelay / assoc.numEndToEndMessages) > 0) {
            uint32 msgnum = assoc.numEndToEndMessages - assoc.startEndToEndDelay;
            if (assoc.stopEndToEndDelay > 0)
                msgnum -= (assoc.numEndToEndMessages - assoc.stopEndToEndDelay);
            recordScalar("Average End to End Delay", assoc.cumEndToEndDelay / msgnum);
        }

        recordScalar("RTXMethod", (double)par("RTXMethod"));
    }
}
