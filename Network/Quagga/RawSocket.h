#ifndef RAWSOCKET_H_
#define RAWSOCKET_H_

#include "IPAddress.h"
#include "IPDatagram.h"

#include "zebra_env.h"

class RawSocket
{
    public:
        RawSocket(int userId, int protocol);

        void setOutputGate(cGate *toIP);

        // pure unix interface
        int send(const struct msghdr *message, int flags);

        int getProtocol();

        void setHdrincl(bool b);
        bool getHdrincl();

        void setPktinfo(bool b);
        void setMulticastLoop(bool b);
        void setMulticastTtl(bool b);

        void setMulticastIf(IPAddress addr);
        IPAddress getMulticastIf();

    private:

        void sendToIP(cMessage *msg);

        int userId;
        int protocol;

        bool hdrincl;
        bool pktinfo;
        bool multicastLoop;
        bool multicastTtl;
        IPAddress multicastIf;
        int multicastOutputPort; // determined from multicastIf address

        cGate *gateToIP;

};

#endif
