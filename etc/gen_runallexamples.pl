#!/usr/bin/perl


@the_skiplist = <<END_OF_SKIPLIST =~ m/(\S.*\S)/g;
examples/adhoc/ieee80211/omnetpp.ini  Ping2         # interactive
examples/adhoc/mf80211/omnetpp.ini  Ping2         # interactive
examples/ethernet/lans/defaults.ini  General
examples/mpls/testte_failure/omnetpp.ini  General        # <!> Error in module (PPP) RSVPTE4.LSR2.ppp[1].ppp (id=70) at event #13304, t=2.009399999999: Cannot schedule message (cMessage)pppEndTxEvent to the past, t=0.
examples/mpls/testte_failure2/omnetpp.ini  General        # <!> Error in module (RSVP) RSVPTE4.LSR1.rsvp (id=22) at event #2, t=0: Model error: not a local peer: 10.1.1.1.
examples/rtp/multicast1/omnetpp.ini  General         # interactive parameter `RTPMulticast1.host1.IPForward'
examples/rtp/multicast2/omnetpp.ini  General         # <!> Error: Network `rtpNetwork' or `inet.examples.rtp.multicast2.rtpNetwork' not found, check .ini and .ned files.
examples/rtp/unicast_check/omnetpp.ini  General         # interactive parameter `RTPMulticast1.host1.IPForward'
examples/wireless/lan80211/omnetpp.ini  Ping2
examples/wireless/lan80211/omnetpp-ftp.ini  NHosts         # interactive
examples/wireless/lan80211/omnetpp-streaming.ini  Streaming2
examples/wireless/test2/omnetpp.ini  General        # interactive parameter `Throughput.cliHost[0].wlan.agent.probeDelay'
END_OF_SKIPLIST

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

chdir "$DN/..";

$INETROOT=`pwd`;
chomp $INETROOT;
#print "INETROOT='$INETROOT'\n";

print '#!/bin/sh
cd `dirname $0`/..
INETROOT=`pwd`

echo "[General]
cpu-time-limit = 3s" >tmp.ini

run() {
  cd $INETROOT/`dirname $1`
  echo "
========================================================
Running: $1  $2
--------------------------------------------------------
"
  $INETROOT/src/run_inet -u Cmdenv -n $INETROOT/src:$INETROOT/examples -c $2 `basename $1` $INETROOT/tmp.ini

  echo "
--------------------------------------------------------
finished: $1  $2
========================================================
"
}

';

@inifiles = sort `find examples -name '*.ini'`;

die("Not found ini files\n") if ($#inifiles lt 0);

#print "=====================================\n",@inifiles,"=========================================7\n";

@runs = ();

foreach $fname (@inifiles)
{
    chomp $fname;

    open(INFILE, $fname) || die "cannot open $fname";
    read(INFILE, $txt, 1000000) || die "cannot read $fname";
    close INFILE;

    next if ($txt =~ /\bfingerprint\s*=/); #skip fingerprint tests

    @configs = ($txt =~ /\[Config (\w+)\]/g);

#    print "-1- ",$#configs," ---------------->file=$fname, configs={",join(',',@configs),"}\n";

    @configs = ( "General" ) if ($#configs lt 0);

#    print "-2- ",$#configs," --------------->file=$fname, configs={",join(',',@configs),"}\n";

    foreach $conf (@configs)
    {
        $run = "$fname  $conf";
        $x = "run $run";
        if (length($skiplist{$run}))
        {
            $x = "# run $run   ".$skiplist{$run};
        }
        print "$x\n";
    }
}
