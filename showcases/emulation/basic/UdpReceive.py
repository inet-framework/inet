import socket
import time

# script running for 2 seconds, just like the simluation

timeout = time.time() + 2

# implementing "UdpEchoApp"

localAddr = "receiver"
localPort = 5001

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((localAddr, localPort))

# receive packets and echo them back

while True:
	payload, client_address = sock.recfrom(1)
	print("packet received: payload: " + str(payload) + " sourceAddr: " + str(client_address))
	sent = sock.sendto(payload, client_address)
	if time.time() > timeout:
		break
