
simple TcpTestClient
    parameters:
        active: bool,
        tOpen: numeric const,
        tSend: numeric const,
        sendBytes: numeric const,
        tClose: numeric const;
    gates:
        in: tcpIn;
        out: tcpOut;
endsimple

