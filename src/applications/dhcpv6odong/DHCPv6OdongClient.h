#ifndef INET_DHCPV6ODONGClIENT_H__
#define INET_DHCPV6ODONGCLIENT_H__

#include <vector>
#include "NotificationBoard.h"
#include "MACAddress.h"
#include "DHCPv6OdongMessage_m.h"
#include "DHCPv6OdongLease.h"
#include "InterfaceTable.h"
#include "RoutingTable6.h"
#include "UDPSocket.h"
#include "INotifiable.h"
#include "ILifecycle.h"

#include "PrefixTable.h"

class INET_API DHCPv6OdongClient : public cSimpleModule, public INotifiable, public ILifecycle
{
    protected:
        int serverPort;
        int clientPort;
        UDPSocket socket; // UDP socket for client-server communication
        bool isOperational; //lifecycle (?)
        cMessage* leaseTimer; // length of time the lease is valid
        cMessage* startTimer; // self message to start DHCP initialization

        enum TimerType
        {
            LEASE_TIMEOUT, START_DHCP
        };

        enum ClientState
        {
             IDLE, INIT, INIT_REBOOT
        };

        std::string hostName;
        simtime_t startTime;
        ClientState clientState;

        MACAddress clientMacAddress;
        NotificationBoard *nb;
        InterfaceEntry *ie;
        IRoutingTable *irt;
        PrefixTable *prefixTable;
        DHCPv6OdongLease *lease;
        IPv6Route *route;

    protected:
        virtual int numInitStages() const { return 4; }
        virtual void initialize(int stage);
        virtual void finish();
        virtual void handleMessage(cMessage *msg);

        virtual void openSocket();

        virtual void handleDHCPv6OdongMessage (DHCPv6OdongMessage *msg);

        virtual void handleTimer(cMessage *msg);

        virtual void receiveChangeNotification(int category, const cPolymorphic *details);

        virtual void sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort);

        virtual void sendRequest();

        virtual void recordLease(DHCPv6OdongMessage *dhcpAck);

        // Assign prefix to the router
        virtual void bindLease();

        virtual void unboundLease();

        virtual void initClient();

        virtual void initRebootedClient();

        virtual InterfaceEntry *chooseInterface();

        static const char *getStateName(ClientState state);

        const char *getAndCheckMessageTypeName(DHCPv6MessageType type);

        virtual void updateDisplayString();

        virtual void startApp();

    public:
        DHCPv6OdongClient();
        virtual ~DHCPv6OdongClient();

        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

};

#endif
