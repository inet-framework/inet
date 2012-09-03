// (C) 2012 OpenSim Ltd

// OLSR packet plan for a new implementation

class noncobject IPvXAddress;


cplusplus {{
#include "IPvXAddress.h"

    class OlsrMsg;

}}

enum OlsrMessageType
{
    OLSR_HELLO_MSG = 1;
    OLSR_TC_MSG    = 2;
    OLSR_MID_MSG   = 3;
    OLSR_HNA_MSG   = 4;
};

enum OlsrWillingness
{
    WILL_NEVER    = 0;
    WILL_LOW      = 1;
    WILL_DEFAULT  = 3;
    WILL_HIGH     = 6;
    WILL_ALWAYS   = 7;
}


enum OlsrLinkType
{
    OLSR_UNSPEC_LINK = 0; // indicating that no specific information about the links is given.
    OLSR_ASYM_LINK   = 1; // indicating that the links are asymmetric (i.e., the neighbor interface is "heard").
    OLSR_SYM_LINK    = 2; // indicating that the links are symmetric with the interface.
    OLSR_LOST_LINK   = 3; // indicating that the links have been lost.
};

enum OlsrNeighborType
{
    OLSR_NOT_NEIGH = 0; // indicating that the nodes are either no longer or have not yet become symmetric neighbors.
    OLSR_SYM_NEIGH = 1; // indicating that the neighbors have at least one symmetrical link with this node.
    OLSR_MPR_NEIGH = 2; // indicating that the neighbors have at least one symmetrical link AND have been selected as MPR by the sender.
};


class HnaPair
{
    // byteLength = 2 * addressSize;
    IPvXAddress address;
    IPvXAddress netmask;
}

class OlsrMsg extends cOwnedObject
{
    // byteLength = 1 + 1 + 2 + addressSize + 1 + 1;
    short msgType @enum(OlsrMessageType);
    simtime_t vtime;
    int msgSize;
    IPvXAddress originatorAddress;
    int ttl;
    int hopCount;
    short msgSeqNum;
};

class OlsrHelloMsgItem
{
    // byteLength = 1 + 1 + 2 + size of address[];
    short linkType @enum(OlsrLinkType);
    short neighborType @enum(OlsrNeighborType);
    IPvXAddress neighborIfAddress[];
};

class OlsrHelloMsg extends OlsrMsg
{
    // byteLength = OlsrMsg::byteLength + 2 + 1 + 1 + size of item[];
    msgType = OLSR_HELLO_MSG;
    simtime_t htime;
    short willingness @enum(OlsrWillingness);
    OlsrHelloMsgItem item[];
};

class OlsrTcMsg extends OlsrMsg
{
    // byteLength = OlsrMsg::byteLength + 2 + 2 + size of address[];
    msgType = OLSR_TC_MSG;
    short ansn;
    IPvXAddress AdvertisedNeighborMainAddres[];
};

class OlsrMidMsg extends OlsrMsg
{
    // byteLength = OlsrMsg::byteLength + size of address[];
    msgType = OLSR_MID_MSG;
    IPvXAddress interfaceAddress[];
};

class OlsrHnaMsg extends OlsrMsg
{
    // byteLength = OlsrMsg::byteLength + size of hnaPair[];
    msgType = OLSR_HNA_MSG;
    HnaPair hnaPair[];
};

packet OlsrPacket
{
    @customize;
    // byteLength = 2 + 2 + size of msg[];
    short pktSeqNum;
    abstract OlsrMsg msg[];
    //TODO: function for calculate packet size
};
