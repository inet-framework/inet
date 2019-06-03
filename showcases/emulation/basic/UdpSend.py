import socket
import time

# script running for 2 seconds, just like the simulation

timeout = time.time() + 2

# implementing "UdpBasicApp"

destAddr = "192.168.2.2"
destPort = 5001
message = "udp message"

# send packets each 100ms

while True:
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.sendto(message, (destAddr, destPort))
	print("packet sent: destAddr: " + destAddr + " destPort: " + str(destPort))
	time.sleep(.100)
	if time.time() > timeout:
		break
