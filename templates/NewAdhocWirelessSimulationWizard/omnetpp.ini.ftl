[General]
network = ${targetTypeName}
#record-eventlog = true
#eventlog-message-detail-pattern = *:(not declaredOn(cMessage) and not declaredOn(cNamedObject) and not declaredOn(cObject))

<#if parametric>
*.numHosts = ${numOfHosts}
    <#assign host_0 = "host[0]">
    <#assign host_all = "host[*]">
<#else>
    <#assign host_0 = "host0">
    <#assign host_all = "host*">
</#if>

num-rngs = 3
**.mobility.rng-0 = 1
**.wlan[*].mac.rng-0 = 2
#debug-on-errors = true

tkenv-plugin-path = ../../../etc/plugins

**.channelNumber = 0

# channel physical parameters
*.channelControl.carrierFrequency = 2.4GHz
*.channelControl.pMax = 2.0mW
*.channelControl.sat = -110dBm
*.channelControl.alpha = 2
*.channelControl.numChannels = 1

# mobility
**.${host_all}.mobilityType = "MassMobility"
**.mobility.constraintAreaMinZ = 0m
**.mobility.constraintAreaMaxZ = 0m
**.mobility.constraintAreaMinX = 0m
**.mobility.constraintAreaMinY = 0m
**.mobility.constraintAreaMaxX = 600m
**.mobility.constraintAreaMaxY = 400m
**.mobility.changeInterval = truncnormal(2s, 0.5s)
**.mobility.changeAngleBy = normal(0deg, 30deg)
**.mobility.speed = truncnormal(20mps, 8mps)
**.mobility.updateInterval = 100ms

# ping app (${host_0} pinged by others)
*.${host_0}.pingApp[0].destAddr = ""
*.${host_all}.numPingApps = 1
*.${host_all}.pingApp[0].destAddr = "${host_0}"
*.${host_all}.pingApp[0].startTime = uniform(1s,5s)
*.${host_all}.pingApp[0].printPing = true

# nic settings
**.wlan[*].bitrate = 2Mbps

**.wlan[*].mgmt.frameCapacity = 10
**.wlan[*].mac.address = "auto"
**.wlan[*].mac.maxQueueSize = 14
**.wlan[*].mac.rtsThresholdBytes = 3000B
**.wlan[*].mac.retryLimit = 7
**.wlan[*].mac.cwMinData = 7
**.wlan[*].mac.cwMinMulticast = 31

**.wlan[*].radio.transmitterPower = 2mW
**.wlan[*].radio.thermalNoise = -110dBm
**.wlan[*].radio.sensitivity = -85dBm
**.wlan[*].radio.pathLossAlpha = 2
**.wlan[*].radio.snirThreshold = 4dB

