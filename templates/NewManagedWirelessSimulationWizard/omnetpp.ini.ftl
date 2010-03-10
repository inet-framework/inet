[General]
network = ${targetTypeName}

*.numOfHosts = ${numOfHosts}

#debug-on-errors = true
tkenv-plugin-path = ../../../etc/plugins

*.playgroundSizeX = 600
*.playgroundSizeY = 400
**.debug = true
**.coreDebug = false
**.channelNumber = 0
**.mobility.x = -1
**.mobility.y = -1

# channel physical parameters
*.channelcontrol.carrierFrequency = 2.4GHz
*.channelcontrol.pMax = 20.0mW
*.channelcontrol.sat = -110dBm
*.channelcontrol.alpha = 2

# access point
**.ap.wlan.mac.address = "10:00:00:00:00:00"
**.host[*].**.mgmt.accessPointAddress = "10:00:00:00:00:00"
**.mgmt.frameCapacity = 100

# mobility
**.host[*].mobility.x = -1
**.host[*].mobility.y = -1

**.host[*].mobilityType = "MassMobility"
**.host[*].mobility.changeInterval = truncnormal(2s, 0.5s)
**.host[*].mobility.changeAngleBy = normal(0deg, 30deg)
**.host[*].mobility.speed = truncnormal(20mps, 8mps)
**.host[*].mobility.updateInterval = 100ms

<#if hasUdpApp>
# udp app
**.numUdpApps = 1
**.host[0].udpAppType = "UDPVideoStreamSvr"
**.host[0].udpApp[*].videoSize = 10MB
**.host[0].udpApp[*].serverPort = 3088
**.host[0].udpApp[*].waitInterval = 10ms
**.host[0].udpApp[*].packetLen = 1000B

**.host[*].udpAppType = "UDPVideoStreamCli"
**.host[*].udpApp[*].serverAddress = "host[0]"
**.host[*].udpApp[*].localPort = 9999
**.host[*].udpApp[*].serverPort = 3088
**.host[*].udpApp[*].startTime = 0
</#if>

<#if hasPingApp>
# ping app (host[0] pinged by others)
*.host[0].pingApp.destAddr = ""
*.host[*].pingApp.destAddr = "host[0]"
**.pingApp.interval = 10ms
**.pingApp.startTime = uniform(0s,0.1s)
</#if>

<#if hasTcpApp>
**.host[0].tcpType = "${serverTcpLayer}" # TCP/TCP_old/TCP_NSC
**.host[*].tcpType = "${clientTcpLayer}" # TCP/TCP_old/TCP_NSC

# tcp apps
**.host[0].numTcpApps = 1
**.host[0].tcpAppType = "TCPSinkApp"
**.host[0].tcpApp[0].port = 1000

**.host[*].numTcpApps = 1
**.host[*].tcpAppType = "TCPSessionApp"  # ftp
**.host[*].tcpApp[0].active = true
**.host[*].tcpApp[0].connectAddress = "host[0]"
**.host[*].tcpApp[0].connectPort = 1000
**.host[*].tcpApp[0].tOpen = 0
**.host[*].tcpApp[0].tSend = 0
**.host[*].tcpApp[0].sendBytes = 100MB
**.host[*].tcpApp[0].tClose = 0
**.tcpApp[*].address = ""
**.tcpApp[*].port = -1
**.tcpApp[*].sendScript = ""
</#if>


# nic settings
**.mac.address = "auto"
**.mac.maxQueueSize = 14
**.mac.rtsThresholdBytes = 3000B
**.mac.bitrate = 2Mbps
**.wlan.mac.retryLimit = 7
**.wlan.mac.cwMinData = 7
**.wlan.mac.cwMinBroadcast = 31

**.radio.bitrate = 2Mbps
**.radio.transmitterPower = 20.0mW
**.radio.carrierFrequency = 2.4GHz
**.radio.thermalNoise = -110dBm
**.radio.sensitivity = -85mW
**.radio.pathLossAlpha = 2
**.radio.snirThreshold = 4dB

# relay unit configuration
**.relayUnitType = "MACRelayUnitNP"
**.relayUnit.addressTableSize = 100
**.relayUnit.agingTime = 120s
**.relayUnit.bufferSize = 1MB
**.relayUnit.highWatermark = 512KB
**.relayUnit.pauseUnits = 300  # pause for 300*512 bit (19200 byte) time
**.relayUnit.addressTableFile = ""
**.relayUnit.numCPUs = 2
**.relayUnit.processingTime = 2us
