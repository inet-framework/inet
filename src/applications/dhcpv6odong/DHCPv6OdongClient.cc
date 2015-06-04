#include "DHCPv6OdongClient.h"

#include "InterfaceTableAccess.h"
#include "IPv6InterfaceData.h"
#include "NodeStatus.h"
#include "NotifierConsts.h"
#include "NodeOperations.h"
#include "RoutingTableAccess.h"

Define_Module(DHCPv6OdongClient);

DHCPv6OdongClient::DHCPv6OdongClient()
{
	leaseTimer = NULL;
	startTimer = NULL;
    nb = NULL;
	ie = NULL;
	lease = NULL;
}

DHCPv6OdongClient::~DHCPv6OdongClient()
{
    cancelAndDelete(leaseTimer);
    cancelAndDelete(startTimer);
}

void DHCPv6OdongClient::initialize(int stage)
{
    if (stage ==0)
    {
        leaseTimer = new cMessage("Lease Timeout", LEASE_TIMEOUT);
        startTimer = new cMessage("Starting DHCPv6", START_DHCP);
    }
    else if (stage == 3)
    {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        clientPort = 546;
        serverPort = 547;

        cModule *host = findContainingNode(this);
        hostName = host->getFullName();
        nb = check_and_cast<NotificationBoard *>(getModuleByPath(par("notificationBoardPath")));

        // for a wireless interface subscribe the association event to start the DHCP protocol
        nb->subscribe(this, NF_L2_ASSOCIATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);

        // get the routing table to update and subscribe it to the blackboard
        irt = check_and_cast<IRoutingTable*>(getModuleByPath(par("routingTablePath")));
        // set client to idle state
        clientState = IDLE;

        if (isOperational)
        {
            ie = chooseInterface(); // wlan0 aja / yg terasosiasi dgn accesspoint.
            //kalau di nemo, yg terhubung dgn AP adalah wlan card... trus nyambung ke eth MR :/

            clientMacAddress = ie->getMacAddress();
            startApp();
        }
    }
}

InterfaceEntry *DHCPv6OdongClient::chooseInterface()
{
    IInterfaceTable* ift = check_and_cast<IInterfaceTable*>(getModuleByPath(par("interfaceTablePath")));
    InterfaceEntry *ie = ift->getInterfaceByName("eth0"); //karena MR yg terhubung dgn parent adalah eth0
    return ie;
}

void DHCPv6OdongClient::finish()
{
    cancelEvent(leaseTimer);
    cancelEvent(startTimer);
}

// belum ngeh bener ini buat apaan
static bool routeMatches(const IPv6Route *entry, const IPv6Address& gw)
{
    IPv6Address entryGateway = entry->getNextHop();
    if (!gw.isUnspecified() && !(entryGateway == gw))
        return false;

    return true;
}

const char *DHCPv6OdongClient::getStateName(ClientState state)
{
    switch (state)
    {
#define CASE(X) case X: return #X;
    CASE(INIT);
    default: return "???";
#undef CASE
    }
}

const char *DHCPv6OdongClient::getAndCheckMessageTypeName(DHCPv6MessageType type)
{
    switch (type)
    {
#define CASE(X) case X: return #X;
    CASE(DHCPREQUEST);
    CASE(DHCPACK);
    default: throw cRuntimeError("Unknown or invalid DHCPv6 message type %d",type);
#undef CASE
    }
}

void DHCPv6OdongClient::updateDisplayString()
{
    getDisplayString().setTagArg("t", 0, getStateName(clientState));
}

void DHCPv6OdongClient::handleMessage(cMessage *msg)
{
    if (!isOperational)
    {
        EV_ERROR << "Message '" << msg << "' arrived when module status is down, dropping." << endl;
        delete msg;
        return;
    }
    if (msg->isSelfMessage())
    {
        handleTimer(msg);
    }
    else if (msg->arrivedOn("udpIn"))
    {
        DHCPv6OdongMessage *dhcpPacket = dynamic_cast<DHCPv6OdongMessage*>(msg);
        if (!dhcpPacket)
            throw cRuntimeError(dhcpPacket, "Unexpected packet received (not a DHCPv6 Odong Message)");

        handleDHCPv6OdongMessage(dhcpPacket);
        delete msg;
    }

    if(ev.isGUI())
        updateDisplayString();
}

void DHCPv6OdongClient::handleTimer(cMessage *msg)
{
    int category = msg->getKind();

    if(category == START_DHCP)
    {
        openSocket();
        if(lease)
        {
            clientState = INIT_REBOOT;
            initRebootedClient();
        }
        else // we have no lease from the previous DHCP process
        {
            clientState = INIT;
            initClient();
        }
    }
    else if (category == LEASE_TIMEOUT)
    {
        EV_INFO << "Lease has expired. Starting DHCP process in INIT state." << endl;
        unboundLease();
        clientState = INIT;
        initClient();
    }
    else
        throw cRuntimeError("Unknown self message '%s'", msg->getName());
}

void DHCPv6OdongClient::recordLease(DHCPv6OdongMessage *dhcpAck)
{
    if (!dhcpAck->getPrefix().isUnspecified()) // prefix nya engga enol
    {
        IPv6Address prefix = dhcpAck->getPrefix();
        EV_DETAIL << "DHCPACK arrived with prefix: " << prefix << endl;

        lease->prefixLength = dhcpAck->getOptions().getPrefixLength();

        lease->leaseTime = dhcpAck->getOptions().getLeaseTime();
    }
    else
        EV_ERROR << "DHCPACK arrived, but no prefix confirmed." << endl;
}

void DHCPv6OdongClient::bindLease()
{

}

void DHCPv6OdongClient::unboundLease()
{
    return;
}

void DHCPv6OdongClient::initClient()
{
    return;
}

void DHCPv6OdongClient::initRebootedClient()
{
    return;
}

void DHCPv6OdongClient::handleDHCPv6OdongMessage(DHCPv6OdongMessage *msg)
{
    return;
}

void DHCPv6OdongClient::openSocket()
{
    return;
}
