#!/usr/bin/perl


@the_skiplist = <<END_OF_SKIPLIST =~ m/(\S.*\S)/g;
examples/emulation/extclient/omnetpp.ini  General                       # <!> Error in module (ExtInterface) ExtClient.peer.ext[0] (id=14) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/emulation/extserver/omnetpp.ini  Uplink_Traffic                # <!> Error in module (ExtInterface) extserver.router.ext[0] (id=12) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/emulation/extserver/omnetpp.ini  Downlink_Traffic              # <!> Error in module (ExtInterface) extserver.router.ext[0] (id=12) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/emulation/extserver/omnetpp.ini  Uplink_and_Downlink_Traffic   # <!> Error in module (ExtInterface) extserver.router.ext[0] (id=12) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/emulation/traceroute/omnetpp.ini  General                      # <!> Error in module (ExtInterface) Traceroute.extRouter.ext[0] (id=310) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/ethernet/lans/defaults.ini  General                            # <!> Error: Network `' or `inet.examples.ethernet.lans.' not found, check .ini and .ned files. # The defaults.ini file included from other ini files
#examples/inet/tcpclientserver/omnetpp.ini  NSCfreebsd__NSCfreebsd       # return value: CRASH (139)
#examples/inet/tcpclientserver/omnetpp.ini  NSClwip__inet                # return value: CRASH (134)
#examples/mpls/ldp/omnetpp.ini  General :                                # <!> Error in module (LDP) LDPTEST.LSR5.ldp (id=133) at event #893, t=0.023541800236: Model error: ASSERT: condition uit == fecUp.end() false in function processLABEL_REQUEST, networklayer/ldp/LDP.cc line 952.
#examples/mpls/testte_failure2/omnetpp.ini  General                      # <!> Error in module (RSVP) RSVPTE4.LSR1.rsvp (id=22) at event #2, t=0: Model error: not a local peer: 10.1.1.1.
END_OF_SKIPLIST


# next line DISABLE previous skiplist:
#@the_skiplist = ();


foreach $i ( @the_skiplist )
{
    ($x) = $i =~ m/\#\s*(.*)$/;
    $i =~ s/\s*\#.*//;
    $skiplist{$i} = '# '.$x;
}
#print "-",join("-\n-",keys(%skiplist)),"-\n";
#die();

$DN = `dirname $0`;
chomp $DN;
#print "DN='$DN'\n";

chdir "$DN/../..";

$INETROOT=`pwd`;
chomp $INETROOT;
#print "INETROOT='$INETROOT'\n";


@inifiles = sort `find examples -name '*.ini'`;

die("Not found ini files\n") if ($#inifiles lt 0);

#print "=====================================\n",@inifiles,"=========================================\n";

@runs = ();

print '# workingdir,                        args,                                          simtimelimit,    fingerprint'."\n";

foreach $fname (@inifiles)
{
    chomp $fname;

    open(INFILE, $fname) || die "cannot open $fname";
    read(INFILE, $txt, 1000000) || die "cannot read $fname";
    close INFILE;

###    next if ($txt =~ /\bfingerprint\s*=/); #skip fingerprint tests

    @configs = ($txt =~ /^\s*(\[Config \w+\].*)$/mg);

#    print "-1- ",$#configs," ---------------->file=$fname, configs={",join(',',@configs),"}\n";

    @configs = ( "[Config General]" ) if ($#configs lt 0);

#    print "-2- ",$#configs," --------------->file=$fname, configs={",join(',',@configs),"}\n";

    foreach $conf (@configs)
    {
        ($cfg,$comm) = ($conf =~ /^\[Config (\w+)\]\s*(\#.*)?$/g);
        ($dir,$fnameonly) = ($fname =~ /(.*)[\/\\](.*)/);

        $run = "/".$dir.'/'.",";
        $run .= (' 'x(36-length $run)).' ';
        $run .= "-f $fnameonly -c $cfg -r 0".",";
        $run .= (' 'x(83-length $run)).' ';
        $run .= '---100s'.",";
        $run .= (' 'x(100-length $run)).' ';
        $run .= '0'; # intentionally no "-r 0" -- test should do all runs if fits into the time limit...

        $x = "$run";

        if ($comm =~ /\b__interactive__\b/i)
        {
            $skiplist{$run} = '# '.$conf;
        }

        if (length($skiplist{$run}))
        {
            $x = "# $run   ".$skiplist{$run};
        }

        print "$x\n";
    }
}
