[General]
network = ${targetTypeName}

<#if parametric>
*.numOfHosts = ${numOfHosts}
    <#assign host_0 = "host[0]">
    <#assign host_all = "host[*]">
<#else>
    <#assign host_0 = "host0">
    <#assign host_all = "host*">
</#if>

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
**.${host_all}.**.mgmt.accessPointAddress = "10:00:00:00:00:00"
**.mgmt.frameCapacity = 100

# mobility
**.${host_all}.mobility.x = -1
**.${host_all}.mobility.y = -1

**.${host_all}.mobilityType = "MassMobility"
**.${host_all}.mobility.changeInterval = truncnormal(2s, 0.5s)
**.${host_all}.mobility.changeAngleBy = normal(0deg, 30deg)
**.${host_all}.mobility.speed = truncnormal(20mps, 8mps)
**.${host_all}.mobility.updateInterval = 100ms

<#if hasUdpApp>
# udp app
**.numUdpApps = 1
**.${host_0}.udpAppType = "UDPVideoStreamSvr"
**.${host_0}.udpApp[*].videoSize = 10MB
**.${host_0}.udpApp[*].serverPort = 3088
**.${host_0}.udpApp[*].waitInterval = 10ms
**.${host_0}.udpApp[*].packetLen = 1000B

**.${host_all}.udpAppType = "UDPVideoStreamCli"
**.${host_all}.udpApp[*].serverAddress = "${host_0}"
**.${host_all}.udpApp[*].localPort = 9999
**.${host_all}.udpApp[*].serverPort = 3088
**.${host_all}.udpApp[*].startTime = 0
</#if>

<#if hasPingApp>
# ping app (${host_0} pinged by others)
*.${host_0}.pingApp.destAddr = ""
*.${host_all}.pingApp.destAddr = "${host_0}"
**.pingApp.interval = 10ms
**.pingApp.startTime = uniform(0s,0.1s)
</#if>

<#if hasTcpApp>
**.${host_0}.tcpType = "${serverTcpLayer}" # TCP/TCP_old/TCP_NSC
**.${host_all}.tcpType = "${clientTcpLayer}" # TCP/TCP_old/TCP_NSC

# tcp apps
**.${host_0}.numTcpApps = 1
**.${host_0}.tcpAppType = "TCPSinkApp"
**.${host_0}.tcpApp[0].port = 1000

**.${host_all}.numTcpApps = 1
**.${host_all}.tcpAppType = "TCPSessionApp"  # ftp
**.${host_all}.tcpApp[0].active = true
**.${host_all}.tcpApp[0].connectAddress = "${host_0}"
**.${host_all}.tcpApp[0].connectPort = 1000
**.${host_all}.tcpApp[0].tOpen = 0
**.${host_all}.tcpApp[0].tSend = 0
**.${host_all}.tcpApp[0].sendBytes = 100MB
**.${host_all}.tcpApp[0].tClose = 0
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
