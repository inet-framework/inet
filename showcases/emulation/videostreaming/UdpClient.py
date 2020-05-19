#!/usr/bin/python

import socket
import sys
import time

# config
server_address = ('10.0.0.1', 4004)

# Create an UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


try:
    for i in range(1,10):
        message = 'This is the message %d.  It will be repeated.' % i
        # Send data
        print >>sys.stderr, 'sending "%s"' % message
        sent = sock.sendto(message, server_address)

        # Receive response
        print >>sys.stderr, 'waiting to receive'
        data, server = sock.recvfrom(4096)
        print >>sys.stderr, 'received "%s"' % data
        time.sleep(1)

finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
