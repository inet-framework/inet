#!/usr/bin/python

secondaryRouters = 4
hostsPerRouter = 4



###
#   creation of routing file for main router
###

mainRouterFile = open("mainRouter.mrt", "w")

mainRouterFile.write("ifconfig:\n")

for i in range(0,secondaryRouters):
  mainRouterFile.write("name: eth%i    encap: Ethernet    inet_addr: 192.%i.0.1\n" % (i, i))
  mainRouterFile.write("MTU: 1500    Metric: 1    BROADCAST    MULTICAST\n")

mainRouterFile.write("ifconfigend.\n")
mainRouterFile.write("route:\n")

for i in range(0, secondaryRouters):
  mainRouterFile.write("192.%i.0.0    192.%i.0.2    255.255.0.0    G    0    eth%i\n" % (i, i, i))

for i in range(0, secondaryRouters):
  mainRouterFile.write("225.0.0.1    192.%i.%i.2    255.255.255.255    G    0    eth%i\n" % (i, i, i))

mainRouterFile.write("routeend.\n")
mainRouterFile.close()


###
#   creation of routing files for secondary routers
#     creation of routing files for hosts
###

for i in range(0, secondaryRouters):

  secondaryRouterFile = open("secondaryRouter%i.mrt" % i, "w")
  secondaryRouterFile.write("ifconfig:\n")
  secondaryRouterFile.write("name: eth0    encap: Ethernet    inet_addr: 192.%i.0.2\n" % (i))
  secondaryRouterFile.write("MTU: 1500    Metric: 1    BROADCAST    MULTICAST\n")

  for j in range(0, hostsPerRouter):
    secondaryRouterFile.write("name: eth%i    encap: Ethernet    inet_addr: 192.%i.%i.1\n" % (j + 1, i, j + 1))
    secondaryRouterFile.write("MTU: 1500    Metric: 1    BROADCAST    MULTICAST\n")

  secondaryRouterFile.write("ifconfigend.\n")
  secondaryRouterFile.write("route:\n")

  for j in range(0, hostsPerRouter):
    secondaryRouterFile.write("192.%i.%i.2     *    255.255.255.255    H    0    eth%i\n" % (i, j + 1, j + 1))

  for j in range(0, hostsPerRouter):
    secondaryRouterFile.write("225.0.0.1    *    255.255.255.255    H    0    eth%i\n" % (j + 1))

  secondaryRouterFile.write("225.0.0.1    192.%i.0.1    255.255.255.255    G    0    eth0\n" % (i))

  secondaryRouterFile.write("default:    192.%i.0.1    255.255.255.255    G    0    eth0\n" % (i))
  secondaryRouterFile.write("routeend.\n")
  secondaryRouterFile.close()

  for j in range(0, hostsPerRouter):
    hostRoutingFile = open("host%i-%i.mrt" % (i, j), "w")
    hostRoutingFile.write("ifconfig:\n")
    hostRoutingFile.write("name: eth0    encap: Ethernet    inet_addr: 192.%i.%i.2\n" % (i, j + 1))
    hostRoutingFile.write("MTU: 1500    Metric: 1    BROADCAST    MULTICAST\n")
    hostRoutingFile.write("Groups: 225.0.0.1\n")
    hostRoutingFile.write("ifconfigend.\n")
    hostRoutingFile.write("route:\n")
    hostRoutingFile.write("225.0.0.1    192.%i.%i.1    255.255.255.255    G    0    eth0\n" % (i, j + 1))
    hostRoutingFile.write("default:    192.%i.%i.1    255.255.255.255    G    0    eth0\n" % (i, j + 1))
    hostRoutingFile.write("routeend.\n")
    hostRoutingFile.close()


###
#   creation of network description
###

networkFile = open("multicast2.ned", "w")
networkFile.write("""import "RTPHost", "Router";

channel ethernet
  delay normal(0.00015,0.00005);
  datarate 10*10^6;
endchannel

module RTPMulticast2
  parameters:
    debug: bool;
  submodules:
    mainRouter: Router
      parameters:
        nodename = "mainRouter",
        numOfPorts = %i,
        routingFile = "mainRouter.mrt";
      gatesizes:
        in[%i],
        out[%i];
""" % (secondaryRouters, secondaryRouters, secondaryRouters))

for i in range(0, secondaryRouters):
  networkFile.write("""
    secondaryRouter%i: Router
      parameters:
        nodename = "secondaryRouter%i",
        numOfPorts = %i,
        routingFile = "secondaryRouter%i.mrt";
      gatesizes:
        in[%i],
        out[%i];
""" % (i, i, hostsPerRouter + 1, i, hostsPerRouter + 1, hostsPerRouter + 1))

  for j in range(0, hostsPerRouter):
    networkFile.write("""
    host%i_%i: RTPHost
      parameters:
""" % (i, j))
    networkFile.write("""
        debug = debug,
        numOfPorts = 1,
        nodename = "host%i-%i",
        routingFile = "host%i-%i.mrt";
""" % (i, j, i, j))

networkFile.write("""
  connections:
""")

for i in range(0, secondaryRouters):
  networkFile.write("""    mainRouter.out[%i] --> ethernet --> secondaryRouter%i.in[0],
    mainRouter.in[%i] <-- ethernet <-- secondaryRouter%i.out[0],
""" % (i, i, i, i))

  for j in range(0, hostsPerRouter):

     if ((i == secondaryRouters - 1) & (j == hostsPerRouter - 1)):
       networkFile.write("""    secondaryRouter%i.out[%i] --> ethernet --> host%i_%i.in[0],
    secondaryRouter%i.in[%i] <-- ethernet <-- host%i_%i.out[0];
""" % (i, j + 1, i, j, i, j + 1, i, j))

     else:
       networkFile.write("""    secondaryRouter%i.out[%i] --> ethernet --> host%i_%i.in[0],
    secondaryRouter%i.in[%i] <-- ethernet <-- host%i_%i.out[0],
""" % (i, j + 1, i, j, i, j + 1, i, j))

networkFile.write("""
endmodule

network
  rtpNetwork: RTPMulticast2
    parameters:
      debug = true;
endnetwork""")

networkFile.close()


###
#   creation of omnetpp.ini
###

omnetppiniFile = open("omnetpp.ini", "w")
omnetppiniFile.write("""
[General]
network = rtpNetwork
ini-warnings = false
total-stack-kb=27535

[Tkenv]
default-run=1
module-messages = yes
Verbose-simulation = yes

[Parameters]
*.numOfProcessors = 1
*.profileName = "RTPAVProfile"
*.destinationAddress = "225.0.0.1"
*.portNumber = 5004
*.bandwidth = 8000
*.fileName = ""
*.payloadType = 32
*.autoOutputFileNames = true
*.sessionEnterDelay = 0s
*.transmissionStartDelay = 10s
*.transmissionStopDelay = 3m
*.sessionLeaveDelay = 10m
""")
omnetppiniFile.close()
