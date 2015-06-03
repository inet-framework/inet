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
}

