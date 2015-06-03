cplusplus {{
    #include "MACAddress.h"
    #include "IPv6Address.h"
}}

class noncobject MACAddress;
class noncobject IPv6Address;

enum DHCPv6MessageType
{
    DHCPREQUEST = 1;
    DHCPACK = 2;
}

class DHCPv6Options {
    int messageType @enum(DHCPv6MessageType);
    string hostName;
    MACAddress clientIdentifier;
    int prefixLength;
    IPv6Address serverIdentifier;
    simtime_t leaseTime;
}

packet DHCPv6OdongMessage
{
    IPv6Address prefix;
    MACAddress chaddr;
    DHCPv6Options options;
}