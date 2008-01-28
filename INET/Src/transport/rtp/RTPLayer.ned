
package org.omnetpp.inet.Transport.RTP;

module RTPLayer

{
    parameters:
        string rtpModuleType;
        string rtcpModuleType;

    gates:
        input fromApp;
        input fromSocketLayerRTP;
        input fromSocketLayerRTCP;
        output toApp;
        output toSocketLayerRTP;
        output toSocketLayerRTCP;

    submodules:
        rtpModule: <rtpModuleType> like RTPModule;
        rtcpModule: <rtcpModuleType> like RTCPModule;
    connections allowunconnected:
        fromApp --> rtpModule.fromApp;
        toApp <-- rtpModule.toApp;
        rtpModule.toRTCP --> rtcpModule.fromRTP;
        rtpModule.fromRTCP <-- rtcpModule.toRTP;
        rtpModule.toSocketLayer --> toSocketLayerRTP;
        rtpModule.fromSocketLayer <-- fromSocketLayerRTP;
        rtcpModule.toSocketLayer --> toSocketLayerRTCP;
        rtcpModule.fromSocketLayer <-- fromSocketLayerRTCP;

}


