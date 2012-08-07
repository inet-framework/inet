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

**.mobility.constraintAreaMinZ = 0m
**.mobility.constraintAreaMaxZ = 0m
**.mobility.constraintAreaMinX = 0m
**.mobility.constraintAreaMinY = 0m
**.mobility.constraintAreaMaxX = 600m
**.mobility.constraintAreaMaxY = 400m
**.debug = false
**.coreDebug = false

**.channelNumber = 0

# channel physical parameters
*.channelControl.carrierFrequency = 2.4GHz
*.channelControl.pMax = 20.0mW
*.channelControl.sat = -110dBm
*.channelControl.alpha = 2

# mobility

**.${host_all}.mobilityType = "MassMobility"
**.${host_all}.mobility.changeInterval = truncnormal(2s, 0.5s)
**.${host_all}.mobility.changeAngleBy = normal(0deg, 30deg)
**.${host_all}.mobility.speed = truncnormal(20mps, 8mps)
**.${host_all}.mobility.updateInterval = 100ms

<#if hasUdpApp>
# udp app
**.numUdpApps = 1
**.${host_0}.udpApp[*].typename = "UDPVideoStreamSvr"
**.${host_0}.udpApp[*].videoSize = 10MiB
**.${host_0}.udpApp[*].serverPort = 3088
**.${host_0}.udpApp[*].sendInterval = 10ms
**.${host_0}.udpApp[*].packetLen = 1000B

**.${host_all}.udpApp[*].typename = "UDPVideoStreamCli"
**.${host_all}.udpApp[*].serverAddress = "${host_0}"
**.${host_all}.udpApp[*].localPort = 9999
**.${host_all}.udpApp[*].serverPort = 3088
**.${host_all}.udpApp[*].startTime = 0
</#if>

<#if hasPingApp>
# ping app (${host_0} pinged by others)
**.numPingApps = 1
*.${host_0}.pingApp[*].destAddr = ""
*.${host_all}.pingApp[*].destAddr = "${host_0}"
**.pingApp[*].sendInterval = 10ms
**.pingApp[*].startTime = uniform(0s,0.1s)
</#if>

<#if hasTcpApp>
**.${host_0}.tcpType = "${serverTcpLayer}" # TCP/TCP_lwIP/TCP_NSC
**.${host_all}.tcpType = "${clientTcpLayer}" # TCP/TCP_lwIP/TCP_NSC

# tcp apps
**.${host_0}.numTcpApps = 1
**.${host_0}.tcpApp[0].typename = "TCPSinkApp"

**.${host_all}.numTcpApps = 1
**.${host_all}.tcpApp[0].typename = "TCPSessionApp"  # ftp
**.${host_all}.tcpApp[0].connectAddress = "${host_0}"
</#if>

# nic settings
**.bitrate = 2Mbps

**.mac.address = "auto"
**.mac.maxQueueSize = 14
**.mac.rtsThresholdBytes = 3000B
**.wlan[*].mac.retryLimit = 7
**.wlan[*].mac.cwMinData = 7
**.wlan[*].mac.cwMinMulticast = 31

**.radio.transmitterPower = 20.0mW
**.radio.carrierFrequency = 2.4GHz
**.radio.thermalNoise = -110dBm
**.radio.sensitivity = -85dBm
**.radio.pathLossAlpha = 2
**.radio.snirThreshold = 4dB

# relay unit configuration
**.relayUnitType = "MACRelayUnitNP"
**.relayUnit.addressTableSize = 100
**.relayUnit.agingTime = 120s
**.relayUnit.bufferSize = 1MiB
**.relayUnit.highWatermark = 512KiB
**.relayUnit.pauseUnits = 300  # pause for 300*512 bit (19200 byte) time
**.relayUnit.addressTableFile = ""
**.relayUnit.numCPUs = 2
**.relayUnit.processingTime = 2us
