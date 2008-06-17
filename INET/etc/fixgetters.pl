
# from msg files
my $arglessGetters = "fec addr destAddr destAddress nextHopAddr
    receiverAddress senderAddress srcAddr srcAddress recordRoute
    sourceRoutingOption timestampOption destAddr destAddress
    destinationAddress prefix srcAddr srcAddress targetAddress
    destAddr localAddr remoteAddr
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
    sendQueueClass stateName tcpAlgorithmClass";

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
$gettersWithArg =~ s/\s+/|/g;


$arglessGetters="----";  #FIXME remove just temp!!!
$gettersWithArg="----";  #FIXME remove just temp!!!


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
    $txt =~ s/\b($arglessGetters) ?\( *\)/"get".ucfirst($1)."()"/mge;
    $txt =~ s/\b($gettersWithArg) ?\(/"get".ucfirst($1)."("/mge;

    # custom renamings
    $txt =~ s/\bmtu\(\)/getMTU()/mg;

    # 802.11
    $txt =~ s/\bDIFSPeriod\(\)/getDIFS()/mg;
    $txt =~ s/\bEIFSPeriod\(\)/getEIFS()/mg;
    $txt =~ s/\bPIFSPeriod\(\)/getPIFS()/mg;
    $txt =~ s/\bSIFSPeriod\(\)/getSIFS()/mg;
    $txt =~ s/\bSlotPeriod\(\)/getSlotTime()/mg;

    # RTP
    $txt =~ s/\bvalid\(\)/isValid()/mg;
    $txt =~ s/\bactive\(\)/isActive()/mg;
    $txt =~ s/\brtcpPort\(\)/getRTCPPort()/mg;
    $txt =~ s/\brtpPort\(\)/getRTPPort()/mg;
    $txt =~ s/\bsdesChunk\(\)/getSDESChunk()/mg;
    $txt =~ s/\bsdesChunks\(\)/getSDESChunks()/mg;
    $txt =~ s/\brtcpPackets\(\)/getRTCPPackets()/mg;

    $txt =~ s/\bpacketOk\(/isPacketOK(/mg;


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


