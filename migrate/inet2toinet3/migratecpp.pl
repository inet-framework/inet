
%renamedParamsAndGates = (
    # gates
#    "from_ip"   => "ipIn",
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

    # custom renamings
#    $txt =~ s/\bmtu\(\)/getMTU()/mg;
    $txt =~ s/\bIPvXAddressResolver\b/L3AddressResolver/mg;
    $txt =~ s/\bIPvXAddress\b/L3Address/mg;
    $txt =~ s/\bopp_error\b/throw cRuntimeError/mg;
    $txt =~ s/\bRoutingTable6\b/IPv6RoutingTable/mg;

    $txt =~ s/\buint64\b/uint64_t/mg;
    $txt =~ s/\bint64\b/int64_t/mg;
    $txt =~ s/\buint32\b/uint32_t/mg;
    $txt =~ s/\bint32\b/int32_t/mg;
    $txt =~ s/\buint16\b/uint16_t/mg;
    $txt =~ s/\bint16\b/int16_t/mg;
    $txt =~ s/\buint8\b/uint8_t/mg;
    $txt =~ s/\bint8\b/int8_t/mg;

    $txt =~ s/\bisIPv6\(\)/getType() == L3Address::AddressType::IPv6/mg;
    $txt =~ s/\bisIPv4\(\)/getType() == L3Address::AddressType::IPv4/mg;
    $txt =~ s/\bget6\(\)/toIPv6()/mg;
    $txt =~ s/\bget4\(\)/toIPv4()/mg;

    $txt =~ s/\bsimulation\b/(*getSimulation())/mg;

    # notification
    $txt =~ s/\breceiveChangeNotification\(int\b/receiveSignal(cComponent *source, simsignal_t/mg;
    $txt =~ s/^\#include "INotifiable.h"\n//mg;
    $txt =~ s/^\#include <INotifiable.h>\n//mg;
    $txt =~ s/^\#include "cListener.h"\n//mg;
    $txt =~ s/^\#include <cListener.h>\n//mg;
    $txt =~ s/\bINotifiable\b/cListener/mg;
    $txt =~ s/^\#include "NotificationBoard.h"\n//mg;
    $txt =~ s/^\#include <NotificationBoard.h>\n//mg;
    $txt =~ s/^\#include "cModule.h"\n//mg;
    $txt =~ s/^\#include <cModule.h>\n//mg;
    $txt =~ s/\bNotificationBoard\b/cModule/mg;
    $txt =~ s/\bNotificationBoardAccess\(\)\.getIfExists\(\)/findContainingNode(this)/mg;
    $txt =~ s/\bNotificationBoardAccess\(\)\.get\(\)/getContainingNode(this)/mg;
    $txt =~ s/\bsubscribe\(this, *(NF_[A-Za-z0-9_])\)/subscribe($1, this)/mg;
    $txt =~ s/\bequals\(/operator==(/mg;
    $txt =~ s/\bev( +)/EV$1/mg;

    # override
    $txt =~ s/\b(int +numInitStages\(\) +const);/$1 override;/mg;
    $txt =~ s/\b(int +numInitStages\(\) +const)(\s*\{)/$1 override$2/mg;
    $txt =~ s/\b(void +initialize\(\));/$1 override;/mg;
    $txt =~ s/\b(void +initialize\(int +stage\));/$1 override;/mg;
    $txt =~ s/\b(void +handleMessage\(cMessage *\* *msg\));/$1 override;/mg;
    $txt =~ s/\b(void +handleMessage\(cMessage *\* *msg\))(\s*\{);/$1 override$2/mg;
    $txt =~ s/\b(void +finish\(\));/$1 override;/mg;
    $txt =~ s/\b(void +updateDisplayString\(\));/$1 override;/mg;


#BasicModule ???

    # InterfaceTableAccess
    $txt =~ s/#include <InterfaceTableAccess\.h>/#include <inet\/common\/ModuleAccess.h>/mg;
    $txt =~ s/#include "InterfaceTableAccess\.h"/#include "inet\/common\/ModuleAccess.h"/mg;
    $txt =~ s/\bInterfaceTableAccess\(\)\.get\(this\)/getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)/mg;
    $txt =~ s/\bInterfaceTableAccess\(\)\.get\(\)/getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)/mg;

    # do parameter and gate renamings
    foreach my $from (keys(%renamedParamsAndGates)) {
        my $to = $renamedParamsAndGates{$from};
        $txt =~ s/\b$from\b/$to/sg;
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

print "\nConversion done. You may safely re-run this script as many times as you want.\n";


