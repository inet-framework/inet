
import  
    "TcpTestClient.ned",
    "TCPMain.ned"; 
                         
 
module TcpTest 
    submodules: 
        cli: TcpTestClient; 
            display: "p=95,95;i=app";
        tcpcli: TCPMain; 
            display: "b=32,30;p=95,178;i=prot2";
        tcpsrv: TCPMain; 
            display: "b=32,30;p=302,178;i=prot2";
        srv: TcpTestClient; 
            display: "b=35,26;p=303,96;i=app";
    connections: 
        cli.tcpOut --> tcpcli.from_appl; 
        cli.tcpIn <-- tcpcli.to_appl; 
        srv.tcpOut --> tcpsrv.from_appl; 
        srv.tcpIn <-- tcpsrv.to_appl; 
        tcpcli.to_ip --> delay 0.1 --> tcpsrv.from_ip; 
        tcpcli.from_ip <-- delay 0.1 <-- tcpsrv.to_ip; 
endmodule 


network tcpTest : TcpTest
endnetwork
