
simple TcpTestClient
    parameters:
        address: string,
        port: numeric const,
        active: bool,
        connectAddress: string,
        connectPort: numeric const,
        tOpen: numeric const,
        tSend: numeric const,
        sendBytes: numeric const,
        sendScript: string,
        tClose: numeric const;
    gates:
        in: tcpIn;
        out: tcpOut;
endsimple

