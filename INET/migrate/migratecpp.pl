#
#
#

$verbose = 1;

$listfile = $ARGV[0];
die "no listfile specified" if ($listfile eq '');

# parse listfile
print "reading $listfile...\n" if ($verbose);
open(INFILE, $listfile) || die "cannot open $listfile";
@fnames = ();
while (<INFILE>) {
    chomp;
    s/\r$//; # cygwin/mingw perl does not do CR/LF translation
    push(@fnames,$_);
}
#print join(' ', @fnames);

%replacements = (
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

foreach $fname (@fnames)
{
    print "reading $fname...\n" if ($verbose);
    $txt = readfile($fname);

    # process $txt:
    foreach my $from (keys(%replacements)) {
        my $to = $replacements{$from};
        $txt =~ s/\b$from\b/$to/sg;
    }

    writefile($fname, $txt);
    #writefile("$fname.new", $txt);
}


sub readfile ()
{
    my $fname = shift;
    my $content;
    open FILE, "$fname" || die "cannot open $fname";
    read(FILE, $content, 1000000);
    close FILE;
    $content;
}

sub writefile ()
{
    my $fname = shift;
    my $content = shift;
    open FILE, ">$fname" || die "cannot open $fname for write";
    print FILE $content;
    close FILE;
}




