
//
// Pair scenario file for the sockets pair example. 
//


import
    "Router",
    "StandardHost",
    "FlatNetworkConfigurator",
    "httptController";

channel gigabitline
    delay 0.1us;
    datarate 1000*1000000;
endchannel

module httptSockPair
    submodules:
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "StandardHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "i=block/cogwheel";
        controller: httptController;
            display: "p=62,60;i=block/cogwheel";
        srv: StandardHost;
            display: "p=170,188;i=device/server_l";
        cli: StandardHost;
            display: "p=316,188;i=device/laptop_l";
    connections:
		cli.out++ --> gigabitline --> srv.in++;
		cli.in++ <-- gigabitline <-- srv.out++;
    display: "b=474,374";
endmodule

network httptSockPair : httptSockPair
endnetwork

