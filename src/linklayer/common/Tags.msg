// L4

class TransportPortTag // L4PortTag
{
    int sourcePort;
    int destinationPort;
}

class TransportProtocolTag // L4ProtocolTag, L4Tag
{
    int transportProtocolId; // l4ProtocolId
}

// L3

class NetworkAddressTag // L3AddressTag
{
//    NetworkAddress sourceAddress; // L3Address source
//    NetworkAddress destinationAddress; // L3Address destination
}

class NetworkProtocolTag // L3ProtocolTag, L3Tag
{
    int networkProtocolId; // l3ProtocolId
}

class RoutingProtocolTag
{
    int hopLimit;
}

class RoutingDecisionTag
{
//    NetworkAddress nextHopAddress; // L3Address nextHop
    int outgoingInterfaceId; // interfaceId
}

class IPv4Tag
{
    bool multicastLoop;
    bool dontFragment;
}

class QOSTag
{
    unsigned char typeOfService;
    int diffServCodePoint;
    int explicitCongestionNotification;
}

// L2

// TODO: rename current MACAddress to IEEE802MACAddress or IEEE802Address?

class MACAddressTag // L2AddressTag
{
//    MACAddress sourceAddress; // L2Address source
//    MACAddress destinationAddress; // L2Address destination
}

class InterfaceTag
{
    int interfaceId;
}

class EthernetTag
{
    int etherType;
}

// L1

class SignalTag
{
    double ber;
    double rssi;
    double snr;
}
