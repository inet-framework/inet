<#assign startTime = 0.1> 

[General]
network = ${targetTypeName}
total-stack = 7MiB
tkenv-plugin-path = ../../../etc/plugins
#debug-on-errors = true
#record-eventlog = true


<#assign tcpTraffic = tcpDataTraffic || telnetTraffic > 

<#if tcpTraffic>
# TCP Traffic Configuration:
**.server*.tcpType = "${serverTcpLayer}" # "TCP", "TCP_lwIP", "TCP_NSC"
**.client*.tcpType = "${clientTcpLayer}" # "TCP", "TCP_lwIP", "TCP_NSC"
<#assign appIdx = 0 >

<#if tcpDataTraffic>
**.server*.tcpApp[${appIdx}].typename = "TCPEchoApp"            #//TODO must set only tcpApp[${appIdx}]
**.server*.tcpApp[${appIdx}].address = ""
**.server*.tcpApp[${appIdx}].localPort = 1000
**.server*.tcpApp[${appIdx}].echoFactor = 2.0
**.server*.tcpApp[${appIdx}].echoDelay = 0

**.client*.tcpApp[${appIdx}].typename = "TCPSessionApp"            #//TODO must set only tcpApp[${appIdx}]
**.client*.tcpApp[${appIdx}].active = true
**.client*.tcpApp[${appIdx}].address = ""
**.client*.tcpApp[${appIdx}].localPort = -1
**.client*.tcpApp[${appIdx}].connectAddress = "server"
**.client*.tcpApp[${appIdx}].connectPort = 1000
**.client*.tcpApp[${appIdx}].tOpen = ${startTime}s
**.client*.tcpApp[${appIdx}].tSend = ${startTime+0.1}s
**.client*.tcpApp[${appIdx}].sendBytes = 100MiB
**.client*.tcpApp[${appIdx}].sendScript = ""
**.client*.tcpApp[${appIdx}].tClose = 0
<#assign appIdx = appIdx + 1 >
</#if>

<#if telnetTraffic>
# tcp apps
**.client*.tcpApp[${appIdx}].typename = "TelnetApp"            #//TODO must set only tcpApp[${appIdx}]
**.client*.tcpApp[${appIdx}].address = ""
**.client*.tcpApp[${appIdx}].localPort = 23
#IP address intentionally set incorrectly
**.client*.tcpApp[${appIdx}].connectAddress = "server"
**.client*.tcpApp[${appIdx}].connectPort = 1000
**.client*.tcpApp[${appIdx}].startTime = uniform(${startTime}s,${startTime+5}s)
**.client*.tcpApp[${appIdx}].numCommands = exponential(10)
**.client*.tcpApp[${appIdx}].commandLength = exponential(10B)
**.client*.tcpApp[${appIdx}].keyPressDelay = exponential(0.1s)
**.client*.tcpApp[${appIdx}].commandOutputLength = exponential(40B)
**.client*.tcpApp[${appIdx}].thinkTime = truncnormal(2s,3s)
**.client*.tcpApp[${appIdx}].idleInterval = truncnormal(3600s,1200s)
**.client*.tcpApp[${appIdx}].reconnectInterval = 30s

**.server*.tcpApp[${appIdx}].typename = "TCPGenericSrvApp"     #//TODO must set only tcpApp[${appIdx}]
**.server*.tcpApp[${appIdx}].address = ""
**.server*.tcpApp[${appIdx}].localPort = 1000
**.server*.tcpApp[${appIdx}].replyDelay = 0
<#assign appIdx = appIdx + 1 >
</#if>

**.server*.numTcpApps = ${appIdx}
**.client*.numTcpApps = ${appIdx}
# end of TCP Traffic Configuration
</#if>

<#if videoStreamUdpTraffic && ipv4Layer>
# Video Stream UDP Traffic Configuration:

**.server*.numUdpApps = 1
**.server*.udpApp[*].typename = "UDPVideoStreamSvr"
**.server*.udpApp[*].videoSize = 100MiB
**.server*.udpApp[*].localPort = 3088
**.server*.udpApp[*].sendInterval = 10ms
**.server*.udpApp[*].packetLen = 1000B

**.client*.numUdpApps = 1
**.client*.udpApp[*].typename = "UDPVideoStreamCli"
**.client*.udpApp[*].serverAddress = "server"
**.client*.udpApp[*].localPort = 9999
**.client*.udpApp[*].serverPort = 3088
**.client*.udpApp[*].startTime = ${startTime}s

# end of UDP Traffic Configuration
</#if>

<#if pingTraffic>
# ping app (server pinged by others)
**.client*.numPingApps = 1
**.client*.pingApp[*].destAddr = "server"
**.client*.pingApp[*].sendInterval = 50ms
**.client*.pingApp[*].startTime = ${startTime}s
**.client*.pingApp[*].printPing = true
</#if>

<#if sctpTraffic && ipv4Layer>
# SCTP Traffic Configuration:

**.server*.numSctpApps = 1
**.server*.sctpApp[*].typename = "SCTPServer"
**.server*.sctpApp[0].address = ""
**.server*.sctpApp[0].localPort = 1000
**.server*.sctpApp[0].numPacketsToReceivePerClient = 30
**.server*.sctpApp[0].echoFactor = 0

**.client*.numSctpApps = 1
**.client*.sctpApp[*].typename = "SCTPClient"
**.client*.sctpApp[0].address = ""
**.client*.sctpApp[0].connectAddress = "server"
**.client*.sctpApp[0].primaryPath = "server"
**.client*.sctpApp[0].localPort = -1
**.client*.sctpApp[0].connectPort = 1000
**.client*.sctpApp[0].numRequestsPerSession = 30
**.client*.sctpApp[0].requestLength = 1000
**.client*.sctpApp[0].thinkTime = exponential(0.1s)
**.client*.sctpApp[0].queueSize = 0
**.client*.sctpApp[0].startTime = ${startTime}s

# end of SCTP Traffic Configuration
</#if>

# ip settings

# Ethernet NIC configuration
**.eth[*].queueType = "DropTailQueue" # in routers
