#include <omnetpp.h>
#include <string.h>
#include "MPLSModule.h"


using namespace std;

Define_Module(MPLSModule);


void MPLSModule::initialize()
{
    ipdataQueue.setName("ipdataQueue");
    ldpQueue.setName("ldpQueue");
    maxFecId = 0;

    // Is this LSR an Ingress Router
    isIR = par("isIR");

    // Is this ER an Egress Router
    isER = par("isER");

    // Which FEC classification scheme is used
    classifierType = par("classifier").longValue();

    // Signalling component is ready or not
    isSignallingReady = false;
}


void MPLSModule::dumpFECTable()
{
    ev << "Current FEC table:\n";
    for (std::vector<FECElem>::iterator it = fecList.begin(); it != fecList.end(); it++)
    {
        ev << "FECid=" << it->fecId
           << "  dest=" << it->destAddr
           << "  src =" << it->srcAddr << "\n";
    }
}


void MPLSModule::handleMessage(cMessage * msg)
{
    if (!strcmp(msg->arrivalGate()->name(), "fromL3"))
    {
        ev << "Processing message from L3: " << msg << endl;
        processPacketFromL3(msg);
    }
    else if (!strcmp(msg->arrivalGate()->name(), "fromSignalModule"))
    {
        ev << "Processing message from signalling module: " << msg << endl;
        processPacketFromSignalling(msg);
    }
    else
    {
        ev << "Processing message from L2: " << msg << endl;
        processPacketFromL2(msg);
    }
}


void MPLSModule::processPacketFromL3(cMessage * msg)
{
    IPDatagram *ipdatagram = check_and_cast<IPDatagram *>(msg);
    int gateIndex = msg->arrivalGate()->index();

    // If the MPLS processing is not on, then simply passing the packet
    if (ipdatagram->hasPar("trans"))  // FIXME put this field into IPDatagram!
    {
        send(ipdatagram, "toL2", gateIndex);
        return;
    }

    // IP data from L3 and requires MPLS processing
    MPLSPacket *outPacket = new MPLSPacket(ipdatagram->name());
    outPacket->encapsulate(ipdatagram);

    // This is native IP
    outPacket->pushLabel(-1);
    send(outPacket, "toL2", gateIndex);
}


void MPLSModule::processPacketFromSignalling(cMessage * msg)
{
    RoutingTable *rt = routingTableAccess.get();
    LIBTable *lt = libTableAccess.get();

    if (!isSignallingReady)
    {
        // This is message from LDP saying that it is ready.
        // FIXME some assert() to make sure it's really that?
        ev << "LDP says it is ready, sending out buffered LDP queries to it\n";

        isSignallingReady = true;

        // Start to send out all the pending queries to LDP
        cModule *ldpMod = parentModule()->submodule("signal_module");  // FIXME maybe use connections instead of direct sending....
        if (!ldpMod)
            error("Cannot find signal_module");
        while (!ldpQueue.empty())
        {
            cMessage *ldpMsg = (cMessage *) ldpQueue.pop();
            sendDirect(ldpMsg, 0.0, ldpMod, "from_mpls_switch");
        }
        delete msg;
        return;
    }

    // Get the mapping from the message: "label", "fec" parameters
    int label = msg->par("label").longValue();
    int returnedFEC = (int) (msg->par("fec").longValue());
    bool isLDP = msg->hasPar("my_name");
    delete msg;

    ev << "Message from signalling: label=" << label << ", FEC=" << returnedFEC << "\n";

    std::vector<FECElem>::iterator iterF;
    FECElem iter;
    if (!isLDP)
    {
        for (int i = 0; i < fecList.size(); i++)
        {
            // FIXME!!!! pending FEC's seem to get an id (2*MAX_LSP_NO-returnedFEC)
            // until they are resolved!!! Ughhh!!! should use a bool flag "pending"
            if (fecList[i].fecId == (2 * MAX_LSP_NO - returnedFEC))
            {
                fecList[i].fecId = returnedFEC;
                break;
            }
        }
    }

    dumpFECTable();

    // try sending out buffered IP datagrams and MPLS packets which are waiting for this FEC
    trySendBufferedPackets(returnedFEC, label);
}


void MPLSModule::trySendBufferedPackets(int returnedFEC, int label)
{
    RoutingTable *rt = routingTableAccess.get();
    LIBTable *lt = libTableAccess.get();
    for (int i = 0; i < ipdataQueue.items(); i++)
    {
        cMessage *queuedmsg = (cMessage *) ipdataQueue[i];
        if (!queuedmsg)
            continue;

        // Release packets in queue
        IPDatagram *data = dynamic_cast<IPDatagram *>(queuedmsg);
        MPLSPacket *mplsPck = dynamic_cast<MPLSPacket *>(queuedmsg);
        ASSERT(data || mplsPck);

        // Incoming interface
        int gateIndex;
        if (data)
        {
            gateIndex = data->par("gateIndex");
        }
        else
        {
            gateIndex = mplsPck->par("gateIndex");
            data = check_and_cast < IPDatagram * >(mplsPck->decapsulate());
        }

        InterfaceEntry *ientry = rt->interfaceByPortNo(gateIndex);
        string senderInterface = string(ientry->name.c_str());
        int fecID = classifyPacket(data, classifierType);

        // FIXME khmmm --- we already decapsulated here, can't prentend nothing happened!@!!!@!!!! Andras
        if (fecID!=returnedFEC)
            continue;

        // Remove the message
        ipdataQueue.remove(i);

        // Construct a new MPLS packet
        MPLSPacket *newPacket = NULL;
        if (mplsPck != NULL)
        {
            newPacket = mplsPck;
        }
        else
        {
            newPacket = new MPLSPacket(data->name());
            ev << "FIXME debug: " << data->fullPath();
            ev << " / " << data->owner()->fullPath() << endl;
            newPacket->encapsulate(data);
        }
        newPacket->pushLabel(label);

        newPacket->setKind(fecID);

        // Find the outgoing interface
        string outgoingInterface = lt->findOutgoingInterface(senderInterface, label);

        // A check routine will be added later to make sure outgoingInterface !="X"
        int outgoingPort = rt->interfaceByName(outgoingInterface.c_str())->outputPort;

        send(newPacket, "toL2", outgoingPort);
    }
}

void MPLSModule::processPacketFromL2(cMessage *msg)
{
    IPDatagram *ipdatagram = dynamic_cast<IPDatagram *>(msg);
    MPLSPacket *mplsPacket = dynamic_cast<MPLSPacket *>(msg);

    if (ipdatagram)
    {
        if (isIR)
        {
            // IP datagram, from outside for IR host. We'll try to classify it
            // and add an MPLS header
            processIPDatagramFromL2(ipdatagram);
        }
        else
        {
            // if we are not an Ingress Router and still get an IP packet,
            // then just pass it through to L3
            ipdatagram->addPar("trans") = 0;
            int gateIndex = msg->arrivalGate()->index();
            send(ipdatagram, "toL3", gateIndex);
        }
    }
    else if (mplsPacket)
    {
        processMPLSPacketFromL2(mplsPacket);
    }
    else
    {
        error("Unknown message received");
    }
}


void MPLSModule::processMPLSPacketFromL2(MPLSPacket *mplsPacket)
{
    RoutingTable *rt = routingTableAccess.get();
    LIBTable *lt = libTableAccess.get();

    int gateIndex = mplsPacket->arrivalGate()->index();

    // Here we process MPLS packets
    InterfaceEntry *ientry = rt->interfaceByPortNo(gateIndex);
    string senderInterface = string(ientry->name.c_str());
    int oldLabel = mplsPacket->topLabel();

    if (oldLabel==-1)
    {
        // This is not IP native packet
        // Decapsulate the message and pass up to L3 since this is LDP packet
        //
        // FIXME this smells like hacking. Or is this an "IPv4 Explicit NULL Label"
        // (rfc 3032) or something like this? (Andras)
        IPDatagram *ipdatagram = check_and_cast<IPDatagram *>(mplsPacket->decapsulate());
        send(ipdatagram, "toL3", gateIndex);
        return;
    }

    int newLabel = lt->findNewLabel(senderInterface, oldLabel);
    int optCode = lt->getOptCode(senderInterface, oldLabel);

    if (newLabel >= 0)  // New label can be found
    {
        ev << "incoming label=" << oldLabel << ": ";
        switch (optCode)
        {
            case PUSH_OPER:
                ev << "PUSH " << newLabel;
                mplsPacket->pushLabel(newLabel);
                break;
            case SWAP_OPER:
                ev << "SWAP with " << newLabel;
                mplsPacket->swapLabel(newLabel);
                break;
            case POP_OPER:
                ev << "POP";
                mplsPacket->popLabel();
                break;
            default:
                error("Unknown MPLS OptCode %d", optCode);
        }

        string outgoingInterface = lt->findOutgoingInterface(senderInterface, newLabel);
        int outgoingPort = rt->interfaceByName(outgoingInterface.c_str())->outputPort;

        ev << ", outgoing interface: " << outgoingInterface << "\n";

        send(mplsPacket, "toL2", outgoingPort);

    }
    else if (newLabel==-1 && isER)  // ER router and the new label must be native IP
    {
        string outgoingInterface = lt->findOutgoingInterface(senderInterface, newLabel, oldLabel);
        int outgoingPort = rt->interfaceByName(outgoingInterface.c_str())->outputPort;

        mplsPacket->popLabel();

        // Test if this is a tunnel ER
        if (!mplsPacket->hasLabel())
        {
            // last label popped: decapsulate and send out IP datagram
            IPDatagram *nativeIP = check_and_cast<IPDatagram *>(mplsPacket->decapsulate());
            delete mplsPacket;

            ev << "No new label & last label popped: decapsulating\n";
            ev << "Sending the packet to interface " << outgoingInterface.c_str() << "\n";

            send(nativeIP, "toL2", outgoingPort);
        }
        else  // Message is out of the tunnel
        {
            ev << "Label popped\n";
            ev << "Sending the packet to interface " << outgoingInterface.c_str() << "\n";
            send(mplsPacket, "toL2", outgoingPort);
        }
    }
    else  // Some sort of error here
    {
        error("LIB table inconsistent");
    }
}

void MPLSModule::processIPDatagramFromL2(IPDatagram *ipdatagram)
{
    RoutingTable *rt = routingTableAccess.get();
    LIBTable *lt = libTableAccess.get();
    cModule *ldpMod = parentModule()->submodule("signal_module");  // FIXME should use connections instead of direct sending....
    if (!ldpMod)
        error("Cannot find signal_module");

    int gateIndex = ipdatagram->arrivalGate()->index();

    if (ipdatagram->hasPar("trans"))
    {
        send(ipdatagram, "toL3", gateIndex);
    }
    else
    {
        // Incoming interface
        InterfaceEntry *ientry = rt->interfaceByPortNo(gateIndex);
        string senderInterface = string(ientry->name.c_str());
        ev << " Message from outside to Ingress node" << "\n";

        bool makeRequest = false;
        int fecID = classifyPacket(ipdatagram, classifierType);
        // int myColor =0;
        if (fecID == -1)
        {
            makeRequest = true;
            fecID = classifyPacket(ipdatagram, classifierType);
            ev << "My LSP id mapping is " << fecID << "\n";
        }
        ev << "Message(src, dest, fec)=(" << ipdatagram->srcAddress() << "," <<
            ipdatagram->destAddress() << "," << fecID << ")\n";

        int label = lt->findLabelforFec(fecID);
        ev << " Label found for this message is label(" << label << ")\n";

        if (label != -2)  // New Label found
        {
            // Construct a new MPLS packet

            MPLSPacket *newPacket = NULL;
            newPacket = new MPLSPacket(ipdatagram->name());
            newPacket->encapsulate(ipdatagram);

            // consistent in packet color
            if (fecID < MAX_LSP_NO)
                newPacket->setKind(fecID);
            else
                newPacket->setKind(2 * MAX_LSP_NO - fecID);

            newPacket->pushLabel(label);

            // Find outgoing interface

            // string outgoingInterface= lt->findOutgoingInterface(senderInterface, label);
            string outgoingInterface = lt->findOutgoingInterface(fecID);

            int outgoingPort = rt->interfaceByName(outgoingInterface.c_str())->outputPort;

            // Send out the packet
            ev << " Ingress Node push new label and sending this packet\n";
            send(newPacket, "toL2", outgoingPort);
            // delete msg;
        }
        else  // Need to make ldp query
        {
            ipdatagram->addPar("gateIndex") = gateIndex;
            ipdataQueue.add(ipdatagram);

            // Whether I made requests for this FEC
            if (!makeRequest)
                return;  // Do nothing since I have made a previous request pending

            // signal the LDP by sending some messages
            cMessage *signalMessage = new cMessage("toSignallingModule");
            // signalMessage->setKind(SIGNAL_KIND);
            signalMessage->addPar("FEC") = fecID;
            signalMessage->addPar("dest_addr") = IPAddress(ipdatagram->destAddress()).getInt();
            signalMessage->addPar("src_addr") = IPAddress(ipdatagram->srcAddress()).getInt();
            signalMessage->addPar("gateIndex") = gateIndex;

            if (isSignallingReady)
            {
                // Send to MPLSSwitch
                sendDirect(signalMessage, 0.0, ldpMod, "from_mpls_switch");
            }
            else  // Pending
            {
                ldpQueue.insert(signalMessage);
            }
        }  // End query making

    }  // End data is not transparency
}


int MPLSModule::classifyPacket(IPDatagram *ipdatagram, int type)
{
    IPAddress src = ipdatagram->destAddress();
    IPAddress dest = ipdatagram->srcAddress();

    // find existing FEC based on classifier type
    for (std::vector<FECElem>::iterator it=fecList.begin(); it!=fecList.end(); it++)
    {
        // FEC determined by Destination only
        if (type==DEST_CLASSIFIER && dest==it->destAddr)
            return it->fecId;

        // FEC determined by Destination and Source
        if (type==SRC_AND_DEST_CLASSIFIER && dest==it->destAddr && src==it->srcAddr)
            return it->fecId;
    }

    // if no existing FEC is found: add a new one, but return -1
    FECElem newEle;
    newEle.destAddr = dest;
    newEle.srcAddr = src;
    newEle.fecId = ++maxFecId;
    fecList.push_back(newEle);
    return -1;
}


