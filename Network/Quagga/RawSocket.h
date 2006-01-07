#ifndef RAWSOCKET_H_
#define RAWSOCKET_H_

#include "IPAddress.h"
#include "IPDatagram.h"

#include "zebra_env.h"

class RawSocket
{
    public:
        RawSocket(int userId, int protocol);

        void setOutputGate(cGate *toIP) { gateToIP = toIP; }

        // pure unix interface
        int send(const struct msghdr *message, int flags);

        int getProtocol() { return protocol; }

        void setHdrincl(bool b) { hdrincl = b; }
        bool getHdrincl() { return hdrincl; }

        void setPktinfo(bool b) { pktinfo = b; }
        void setMulticastLoop(bool b) { multicastLoop = b; }
        void setMulticastTtl(bool b) { multicastTtl = b; }
        void setMulticastInterface(int n) { multicastOutputInterfaceId = n; }

    private:

        void sendToIP(cMessage *msg);

        int userId;
        int protocol;

        bool hdrincl;
        bool pktinfo;
        bool multicastLoop;
        bool multicastTtl;
        int multicastOutputInterfaceId;

        cGate *gateToIP;
};

#endif
