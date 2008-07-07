
# from msg files
my $arglessGetters = "fec addr destAddr destAddress nextHopAddr
    receiverAddress senderAddress srcAddr srcAddress recordRoute
    sourceRoutingOption timestampOption destAddr destAddress
    destinationAddress prefix srcAddr srcAddress targetAddress
    destAddr localAddr remoteAddr sourcePort destinationPort
    srcAddr sourceLinkLayerAddress targetLinkLayerAddress abit
    ackBit autoAddressConfFlag serverClose dbit dontFragment finBit
    fin_ack_rcvd fork isRequest isWithdraw managedAddrConfFlag
    moreFragments onlinkFlag otherStatefulConfFlag overrideFlag pshBit
    rbit routerFlag rstBit solicitedFlag synBit tbit urgBit msg
    diffServCodePoint routingType segmentsLeft bitrate holdTime
    keepAliveTime replyDelay MTU ackNo channelNumber code connId
    curHopLimit destPort endSequenceNo errorCode expectedReplyLength flag
    flowLabel fragmentOffset identification identifier interfaceId irs
    iss label lsaLength localPort optionCode payloadLength preferredLifetime
    prefixLength protocol pvLim rcv_nxt rcv_up rcv_wnd reachableTime
    remotePort retransTimer seqNumber sequenceNo snd_max snd_mss snd_nxt
    snd_una snd_up snd_wl1 snd_wl2 snd_wnd sockId srcPort state status
    trafficClass transportProtocol type userId validLifetime originatorId
    seqNo urgentPointer window destPort fragmentOffset headerLength
    hopLimit lastAddressPtr nextAddressPtr overflow protocol
    routerLifetime srcPort timeToLive version
    family messageText receiveQueueClass receiverLDPIdentifier
    sendQueueClass stateName tcpAlgorithmClass
    controlCode controlType id ipHops dst initiator src target isRequest
    isTopPeer last listenPort messageId msg myIP noPeers nslpId packetNumber
    replyLength replyPerRequest sigHops timeToRespond";

# from C++ files
$arglessGetters .= "blackboard connState port inetAddress
    netmask routerId extensionType localAddress remoteAddress
    currentTransmission frameReceivedBeforeSIFS firstLoopbackInterface
    interfaceEntry packetType src scope senderReport socket
    hostModule stateVariables advManagedFlag advOtherConfigFlag
    advSendAdvertisements receptionReports
    message protocol3 protocol4 myPosition multicastGroups
    destPrefix linkLocalAddress nextHop preferredAddress interfaceToken
    macAddress playgroundSizeX playgroundSizeY advLinkMTU
    advReachableTime advRetransTimer connectionId contentionWindow
    delaySinceLastSR fixedHeaderLength interfaceID metric multicastScope
    netmaskLength networkLayerGateIndex nodeInputGateId nodeOutputGateId
    numAddresses numAdvPrefixes numInterfaces numQueues numRoutes
    numRoutingEntries peerNamId socketId topLabel queueLength
    advCurHopLimit advDefaultLifetime expiryTime maxRtrAdvInterval
    minRtrAdvInterval baseReachableTime linkMTU bufferEndSeq
    totalLength";

# IPv6
my $underscoreArglessGetters =
    "maxRandomFactor minRandomFactor maxFinalRtrAdvertisements
    maxInitialRtrAdvertisements maxMulticastSolicit maxNeighbourAdvertisement
    maxRtrSolicitations maxUnicastSolicit delayFirstProbeTime
    maxAnycastDelayTime maxInitialRtrAdvertInterval maxRADelayTime
    maxRtrSolicitationDelay minDelayBetweenRAs reachableTime retransTimer
    rtrSolicitationInterval";

# array fields in msg files
my $gettersWithArg = "payload recordAddress address extensionHeader
    prefixInformation recordTimestamp addresses data";
foreach $i (split(/\s/, $gettersWithArg)) {$arglessGetters .= " ${i}ArraySize";}

# from C++ files
$gettersWithArg .= "gatewayForDestAddr interfaceAddrByPeerAddress
    peerByLocalAddress route interfaceByAddress interfaceByAddress
    interfaceByName interfaceByNetworkLayerGateIndex interfaceByNodeInputGateId
    interfaceByNodeOutputGateId interfaceForDestAddr sourceInterfaceFrom
    multicastRoutesFor routingEntry payloadOwner advPrefix
    numMatchingPrefixBits outputGateForProtocol bytesAvailable";

$arglessGetters =~ s/\s+/|/g;
$underscoreArglessGetters =~ s/\s+/|/g;
$gettersWithArg =~ s/\s+/|/g;


%renamedParamsAndGates = (
    # gates
    "from_ip"   => "ipIn",
    "from_ipv6" => "ipv6In",
    "from_udp"  => "udpIn",
    "from_app"  => "appIn",
    "from_mpls_switch" => "mplsSwitchIn",
    "to_ip"     => "ipOut",
    "to_ipv6"   => "ipv6Out",
    "to_udp"    => "udpOut",
    "to_app"    => "appOut",
    "to_appl"   => "appOut",

    "TCPIn"     => "tcpIn",
    "UDPIn"     => "udpIn",
    "RSVPIn"    => "rsvpIn",
    "OSPFIn"    => "ospfIn",
    "UDPOut"    => "udpOut",
    "RSVPOut"   => "rsvpOut",
    "OSPFOut"   => "ospfOut",

    "fromIPv6"  => "ipv6In",
    "toIPv6"    => "ipv6Out",

    # from RTP -- TBD only when RTP code has been patched!
#    "fromApp" =>            "appIn",
#    "fromProfile" =>        "profileIn",
#    "fromRTP" =>            "rtpIn",
#    "fromRTCP" =>           "rtcpIn",
#    "fromSocketLayer" =>    "socketLayerIn",
#    "fromSocketLayerRTP" => "socketLayerRTPIn",
#    "fromSocketLayerRTCP" =>"socketLayerRTCPIn",
#    "toApp" =>              "appOut",
#    "toProfile" =>          "profileOut",
#    "toRTCP" =>             "rtcpOut",
#    "toRTP" =>              "rtpOut",
#    "toSocketLayer" =>      "socketLayerOut",
#    "toSocketLayerRTP" =>   "socketLayerRTPOut",
#    "toSocketLayerRTCP" =>  "socketLayerRTCPOut",

    # parameters
    "local_port" => "localPort",
    "dest_port" => "destPort",
    "message_length" => "messageLength",
    "message_freq" => "messageFreq",
    "dest_addresses" => "destAddresses",
);



$listfname = $ARGV[0];
open(LISTFILE, $listfname) || die "cannot open $listfname";
while (<LISTFILE>)
{
    chomp;
    s/\r$//; # cygwin/mingw perl does not do CR/LF translation

    $fname = $_;

    if ($fname =~ /_m\./) {
        print "skipping $fname...\n";
        next;
    }

    print "processing $fname... ";

    open(INFILE, $fname) || die "cannot open $fname";
    read(INFILE, $txt, 1000000) || die "cannot read $fname";
    close INFILE;

    my $origtxt = $txt;

    # process $txt:

    # remove omitGetVerb from .msg files
    $txt =~ s/\n *\@omitGetVerb\(true\); *\n/\n/gs;

    # rename getters
    $txt =~ s/\b($arglessGetters)\( *\)/"get".ucfirst($1)."()"/mge;
    $txt =~ s/\b_($underscoreArglessGetters)\( *\)/"_get".ucfirst($1)."()"/mge;
    $txt =~ s/\b($gettersWithArg)\(/"get".ucfirst($1)."("/mge;

    # custom renamings
    $txt =~ s/\bmtu\(\)/getMTU()/mg;

    # 802.11
    $txt =~ s/\bDIFSPeriod\(\)/getDIFS()/mg;
    $txt =~ s/\bEIFSPeriod\(\)/getEIFS()/mg;
    $txt =~ s/\bPIFSPeriod\(\)/getPIFS()/mg;
    $txt =~ s/\bSIFSPeriod\(\)/getSIFS()/mg;
    $txt =~ s/\bSlotPeriod\(\)/getSlotTime()/mg;

    $txt =~ s/\bbackoff\(/computeBackoff(/mg;
    $txt =~ s/\bcontentionWindow\(/computeContentionWindow(/mg;
    $txt =~ s/\bframeDuration\(/computeFrameDuration(/mg;
    $txt =~ s/\bpacketDuration\(/computePacketDuration(/mg;
    $txt =~ s/\bBackoffPeriod\(/computeBackoffPeriod(/mg;
    $txt =~ s/\btimeOut\(/computeTimeout(/mg;

    # RTP
    $txt =~ s/\bvalid\(\)/isValid()/mg;
    $txt =~ s/\bactive\(\)/isActive()/mg;
    $txt =~ s/\brtcpPort\(\)/getRTCPPort()/mg;
    $txt =~ s/\brtpPort\(\)/getRTPPort()/mg;
    $txt =~ s/\bsdesChunk\(\)/getSDESChunk()/mg;
    $txt =~ s/\bsdesChunks\(\)/getSDESChunks()/mg;
    $txt =~ s/\brtcpPackets\(\)/getRTCPPackets()/mg;
    $txt =~ s/\breceptionReport\(\)/createReceptionReport()/mg;
    $txt =~ s/\bsenderReport\(\)/createSender()/mg;

    # UDPSocket
    $txt =~ s/\bsetMulticastInterface\(/setMulticastInterfaceId(/mg;
    $txt =~ s/\bmulticastInterface\(/getMulticastInterfaceId(/mg;

    # other
    $txt =~ s/\bpacketOk\(/isPacketOK(/mg;
    $txt =~ s/\bnodepos\(/find(/mg;
    $txt =~ s/\binterfaceAt\(/getInterface(/mg;
    $txt =~ s/\bbitErrorRate\(/calculateBER(/mg;
    $txt =~ s/\binitialSeqNum\(/chooseInitialSeqNum(/mg;

    $txt =~ s/\bgetInetAddress\(\)/getIPAddress()/mg;
    $txt =~ s/\bsetInetAddress\(/setIPAddress(/mg;

    $txt =~ s/\bipForward\(\)/isIPForwardingEnabled()/mg;
    $txt =~ s/\blocalDeliver\(/isLocalAddress(/mg;
    $txt =~ s/\bmulticastLocalDeliver\(/isLocalMulticastAddress(/mg;

    #$txt =~ s/\bgetInterfaceForDestAddr\(/findInterfaceForDestAddr(/mg;
    #$txt =~ s/\bgetGatewayForDestAddr\(/findGatewayForDestAddr(/mg;

    # do parameter and gate renamings
    foreach my $from (keys(%renamedParamsAndGates)) {
        my $to = $renamedParamsAndGates{$from};
        $txt =~ s/\b$from\b/$to/sg;
    }

    # RoutingTable methods
    $txt =~ s/\bgetNumRoutingEntries\(\)/getNumRoutes()/mg;
    $txt =~ s/\bgetRoutingEntry\(/getRoute(/mg;
    $txt =~ s/\bfindRoutingEntry\(/findRoute(/mg;
    $txt =~ s/\baddRoutingEntry\(/addRoute(/mg;
    $txt =~ s/\bdeleteRoutingEntry\(/deleteRoute(/mg;
    $txt =~ s/\bRoutingEntry\b/IPRoute/mg;  # the class

    # NotificationBoard
    # add "const" to 'detail' argument in receiveChangeNotification()
    $txt =~ s/(\breceiveChangeNotification *\( *int +[a-zA-Z]+ *), *(cPolymorphic|cObject) *\*/$1, const $2 */mg;

    # InterfaceEntry
    $txt =~ s/\bipv4\(\)/ipv4Data()/mg;
    $txt =~ s/\bipv6\(\)/ipv6Data()/mg;

    # IInterfaceTable, IRoutingTable
    $txt =~ s/\bInterfaceTable\b/IInterfaceTable/mg;
    $txt =~ s/\bRoutingTable\b/IRoutingTable/mg;

    # print warnings
    $lineno = 0;
    foreach $linewithcomment (split ("\n", $txt)) {
       $lineno++;
       my $line = $linewithcomment;
       $line =~ s|//.*||; # avoid warning for stuff in comments

       # getInterfaceById()
       if ($line =~ /\bgetInterface\([^)]/) {
          print "*** warning at $fname:$lineno: maybe you need getInterfaceById(int interfaceId) here instead of getInterface(int index). As a rule of thumb, inside a 0..getNumInterfaces() 'for' loop it should be getInterface(i), all other occurrences are likely supposed to be getInterfaceById(interfaceId).\n";
          print "$linewithcomment\n";
       }
    }

    if ($txt eq $origtxt) {
        print "unchanged\n";
    } else {
        open(OUTFILE, ">$fname") || die "cannot open $fname for write";
        print OUTFILE $txt || die "cannot write $fname";
        close OUTFILE;
        print "DONE\n";
    }
}

# BEWARE OF BOGUS REPLACEMENTS INSIDE COMMENTS!!
# getAddress(), getData(), getPayload() !!!!
print "\nConversion done. You may safely re-run this script as many times as you want.\n";


