//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
//


package inet.applications.udpapp;


//
// Connector which directly connects UDP application server and client by filtering UDP-related messages
//
simple UDPAppConnector
{
    parameters:
        @display("i=block/layer");
    gates:
        input in;
        output out;
}
