//!!!!!!!!!!!
// FIXME another version of tcp4ip.ned -- TO BE ELIMINATED (one of them)
//!!!!!!!!!!!

import  
    "TcpIpInterface.ned",
    "MyTCPApp",
    "tcpmodule";

module MyTCPUpperLayers
    parameters: 
        mytcpApplType : string,
        local_addr : string,
        start_time : numeric,
        server_addr : string;
    gates: 
        in: in; 
        out: out; 
    submodules: 
        application: mytcpApplType like MyTCPApp;
            parameters:
                local_addr = local_addr, 
                server_addr = server_addr,
                timeout = 30000s,
                data_interarrival_time = 10s,
                start_time = start_time;

            display: "p=76,52;b=40,24,rect";
        tcp: TcpModule; 
        parameters:
         TCPDelayedAck = true,
         TCPFastRetrans = false,
             TCPFastRecovery = false,
             TCPNewReno = false;
            display: "p=79,120;i=prot1";
        tcpipinterface: TcpIpInterface; 
            display: "p=81,202;i=prot3";
    connections: 
        application.out --> tcp.from_appl; 
        application.in <-- tcp.to_appl; 
        tcp.to_ip --> tcpipinterface.from_tcp; 
        tcp.from_ip <-- tcpipinterface.to_tcp; 
        tcpipinterface.to_ip --> out;
        tcpipinterface.from_ip <-- in;
    display: "p=10,10;b=150,236,rect";
endmodule 

