import RTPModule;
import RTCPModule;

module RTPLayer
{
    parameters:
        string rtpModuleType;
        string rtcpModuleType;
        @display("i=block/layer;bgb=178,105");

    gates:
        input fromApp;
        input fromUDPtoRTP;
        input fromUDPtoRTCP;
        output toApp;
        output toUDPfromRTP;
        output toUDPfromRTCP;

    submodules:
        rtpModule: <rtpModuleType> like RTPModule {
            parameters:
                @display("i=block/buffer;p=63,64");
        }
        rtcpModule: <rtcpModuleType> like RTCPModule {

            parameters:
                @display("i=block/control;p=135,64");
        }
    connections allowunconnected:
        fromApp --> rtpModule.fromApp;
        toApp <-- rtpModule.toApp;
        rtpModule.toRTCP --> rtcpModule.fromRTP;
        rtpModule.fromRTCP <-- rtcpModule.toRTP;
        rtpModule.toUDPLayer --> toUDPfromRTP;
        rtpModule.fromUDPLayer <-- fromUDPtoRTP;
        rtcpModule.toUDP --> toUDPfromRTCP;
        rtcpModule.fromUDP <-- fromUDPtoRTCP;

}


