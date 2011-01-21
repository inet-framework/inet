#!/usr/bin/perl


@the_skiplist = <<END_OF_SKIPLIST =~ m/(\S.*\S)/g;
examples/adhoc/ieee80211/omnetpp.ini  Ping2                             # <!> Error in module (cCompoundModule) Net80211 (id=1) during network setup: The simulation wanted to ask a question, set cmdenv-interactive=true to allow it: "Enter parameter `Net80211.numHosts' (unassigned):".
examples/adhoc/mf80211/omnetpp.ini  Ping2                               # <!> Error in module (cCompoundModule) Net80211 (id=1) during network setup: The simulation wanted to ask a question, set cmdenv-interactive=true to allow it: "Enter parameter `Net80211.numHosts' (unassigned):".
examples/emulation/extclient/omnetpp.ini  General                       # <!> Error in module (ExtInterface) ExtClient.peer.ext[0] (id=14) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/emulation/extserver/omnetpp.ini  Uplink_Traffic                # <!> Error in module (ExtInterface) extserver.router.ext[0] (id=12) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/emulation/extserver/omnetpp.ini  Downlink_Traffic              # <!> Error in module (ExtInterface) extserver.router.ext[0] (id=12) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/emulation/extserver/omnetpp.ini  Uplink_and_Downlink_Traffic   # <!> Error in module (ExtInterface) extserver.router.ext[0] (id=12) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/emulation/traceroute/omnetpp.ini  General                      # <!> Error in module (ExtInterface) Traceroute.extRouter.ext[0] (id=310) during network initialization: cSocketRTScheduler::setInterfaceModule(): pcap devices not supported.
examples/ethernet/lans/defaults.ini  General                            # <!> Error: Network `' or `inet.examples.ethernet.lans.' not found, check .ini and .ned files.
examples/inet/tcpclientserver/omnetpp.ini  NSCfreebsd__NSCfreebsd       # return value: CRASH (139)
examples/inet/tcpclientserver/omnetpp.ini  NSClwip__inet                # return value: CRASH (134)
examples/mpls/ldp/omnetpp.ini  General :                                # <!> Error in module (LDP) LDPTEST.LSR5.ldp (id=133) at event #893, t=0.023541800236: Model error: ASSERT: condition uit == fecUp.end() false in function processLABEL_REQUEST, networklayer/ldp/LDP.cc line 952.
examples/mpls/testte_failure2/omnetpp.ini  General                      # <!> Error in module (RSVP) RSVPTE4.LSR1.rsvp (id=22) at event #2, t=0: Model error: not a local peer: 10.1.1.1.
examples/wireless/lan80211/omnetpp-ftp.ini  NHosts                      # <!> Error in module (cCompoundModule) Lan80211 (id=1) during network setup: The simulation wanted to ask a question, set cmdenv-interactive=true to allow it: "Enter parameter `Lan80211.numHosts' (unassigned):".
examples/wireless/lan80211/omnetpp-streaming.ini  Streaming2            # <!> Error in module (cCompoundModule) Lan80211 (id=1) during network setup: The simulation wanted to ask a question, set cmdenv-interactive=true to allow it: "Enter parameter `Lan80211.numHosts' (unassigned):".
examples/wireless/lan80211/omnetpp.ini  Ping2                           # <!> Error in module (cCompoundModule) Lan80211 (id=1) during network setup: The simulation wanted to ask a question, set cmdenv-interactive=true to allow it: "Enter parameter `Lan80211.numHosts' (unassigned):".
END_OF_SKIPLIST


# next line DISABLE previous skiplist:
@the_skiplist = ();


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
cmdenv-express-mode = true
record-eventlog = false
cpu-time-limit = 3s" >tmp.ini

run() {
  cd $INETROOT/`dirname $1`
  echo "
========================================================
Running: $1  $2
--------------------------------------------------------
"
  $INETROOT/src/run_inet -u Cmdenv -n $INETROOT/src:$INETROOT/examples -c $2 `basename $1` $INETROOT/tmp.ini

  ret="$?"
  retstr="OK"

  if [ $ret \> 127 ] ; then
    echo "<!!> Error - Simulation crashed, return value is $ret"
    retstr="CRASH"
  elif [ $ret != 0 ] ; then
    echo "<!!> Error - Simulation has an error, return value is $ret"
    retstr="ERROR"
  fi

  echo "
--------------------------------------------------------
<!!> Finished: $1  $2 : return value: $retstr ($ret)
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
