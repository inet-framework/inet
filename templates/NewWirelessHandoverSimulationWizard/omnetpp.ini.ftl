[General]

network = ${targetTypeName}
tkenv-plugin-path = ../../../etc/plugins
#debug-on-errors = true

*.playgroundSizeX = ${ numAccessPoints?number * 300 - 100 }
*.playgroundSizeY = 400
**.debug = true
**.coreDebug = false

# channel physical parameters
*.channelcontrol.carrierFrequency = 2.4GHz
*.channelcontrol.pMax = 2.0mW
*.channelcontrol.sat = -110dBm
*.channelcontrol.alpha = 2
*.channelcontrol.numChannels = ${numAccessPoints?number + 2}

# access point
<#list 1..numAccessPoints?number as i>
**.ap${i}.wlan.mac.address = "1${i-1}:00:00:00:00:00"
**.ap${i}.wlan.mgmt.ssid = "AP${i}"
**.ap${i}.wlan.radio.channelNumber = ${i+1}
</#list>
**.ap*.wlan.mgmt.beaconInterval = 100ms
**.wlan.mgmt.numAuthSteps = 4

**.mgmt.frameCapacity = 10

# mobility
**.host*.mobilityType = "LinearMobility"
**.host*.mobility.speed = 20 mps
**.host*.mobility.angle = 0
**.host*.mobility.acceleration = 0
**.host*.mobility.updateInterval = 100ms

# wireless channels

**.host.wlan.radio.channelNumber = 0  # just initially -- it'll scan

# wireless configuration
**.wlan.agent.activeScan = true
**.wlan.agent.channelsToScan = ""  # "" means all
**.wlan.agent.probeDelay = 0.1s
**.wlan.agent.minChannelTime = 0.15s
**.wlan.agent.maxChannelTime = 0.3s
**.wlan.agent.authenticationTimeout = 5s
**.wlan.agent.associationTimeout = 5s

**.mac.address = "auto"
**.mac.maxQueueSize = 14
**.mac.rtsThresholdBytes = 4000B
**.mac.bitrate = 2Mbps
**.wlan.mac.retryLimit = 7
**.wlan.mac.cwMinData = 7
**.wlan.mac.cwMinBroadcast = 31

**.radio.bitrate = 2Mbps
**.radio.transmitterPower = 2.0mW
**.radio.thermalNoise = -110dBm
**.radio.sensitivity = -85mW
**.radio.pathLossAlpha = 2
**.radio.snirThreshold = 4dB
