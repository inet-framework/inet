import "RTPModule", "RTCPModule";

module RTPLayer

	parameters:
		rtpModuleType: string,
		rtcpModuleType: string;

	gates:
		in: fromApp, fromSocketLayerRTP, fromSocketLayerRTCP;
		out: toApp, toSocketLayerRTP, toSocketLayerRTCP;

	submodules:
		rtpModule: rtpModuleType like RTPModule;
		rtcpModule: rtcpModuleType like RTCPModule;

	connections nocheck:
		fromApp --> rtpModule.fromApp;
		toApp <-- rtpModule.toApp;
		rtpModule.toRTCP --> rtcpModule.fromRTP;
		rtpModule.fromRTCP <-- rtcpModule.toRTP;
		rtpModule.toSocketLayer --> toSocketLayerRTP;
		rtpModule.fromSocketLayer <-- fromSocketLayerRTP;
		rtcpModule.toSocketLayer --> toSocketLayerRTCP;
		rtcpModule.fromSocketLayer <-- fromSocketLayerRTCP;

endmodule


