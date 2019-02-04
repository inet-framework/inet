FEATURES_H = src/base/inet_features.h

.PHONY: all clean cleanall makefiles makefiles-so makefiles-lib makefiles-exe checkmakefiles doxy doc

all: checkmakefiles $(FEATURES_H)
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	@cd src && $(MAKE) MODE=release clean
	@cd src && $(MAKE) MODE=debug clean
	@rm -f src/Makefile $(FEATURES_H)

INCLUDE_PATH := -I. \
		-Iapplications \
		-Iapplications/common \
		-Iapplications/dhcp \
		-Iapplications/ethernet \
		-Iapplications/generic \
		-Iapplications/httptools \
		-Iapplications/netperfmeter \
		-Iapplications/pingapp \
		-Iapplications/rtpapp \
		-Iapplications/sctpapp \
		-Iapplications/tcpapp \
		-Iapplications/traci \
		-Iapplications/udpapp \
		-Iapplications/voip \
		-Iapplications/voipstream \
		-Ibase \
		-Ibattery \
		-Ibattery/models \
		-Iinet \
		-Ilinklayer \
		-Ilinklayer/common \
		-Ilinklayer/configurator \
		-Ilinklayer/contract \
		-Ilinklayer/ethernet \
		-Ilinklayer/ethernet/switch \
		-Ilinklayer/ext \
		-Ilinklayer/idealwireless \
		-Ilinklayer/ieee80211 \
		-Ilinklayer/ieee80211/mac \
		-Ilinklayer/ieee80211/mgmt \
		-Ilinklayer/ieee80211/radio \
		-Ilinklayer/ieee80211/radio/errormodel \
		-Ilinklayer/ieee8021d \
		-Ilinklayer/ieee8021d/common \
		-Ilinklayer/ieee8021d/relay \
		-Ilinklayer/ieee8021d/rstp \
		-Ilinklayer/ieee8021d/stp \
		-Ilinklayer/ieee8021d/tester \
		-Ilinklayer/loopback \
		-Ilinklayer/mf80211 \
		-Ilinklayer/mf80211/macLayer \
		-Ilinklayer/mfcore \
		-Ilinklayer/ppp \
		-Ilinklayer/queue \
		-Ilinklayer/radio \
		-Ilinklayer/radio/propagation \
		-Imobility \
		-Imobility/common \
		-Imobility/contract \
		-Imobility/group \
		-Imobility/single \
		-Imobility/static \
		-Inetworklayer \
		-Inetworklayer/arp \
		-Inetworklayer/autorouting \
		-Inetworklayer/autorouting/ipv4 \
		-Inetworklayer/autorouting/ipv6 \
		-Inetworklayer/bgpv4 \
		-Inetworklayer/bgpv4/BGPMessage \
		-Inetworklayer/common \
		-Inetworklayer/contract \
		-Inetworklayer/diffserv \
		-Inetworklayer/icmpv6 \
		-Inetworklayer/internetcloud \
		-Inetworklayer/ipv4 \
		-Inetworklayer/ipv6 \
		-Inetworklayer/ipv6tunneling \
		-Inetworklayer/ldp \
		-Inetworklayer/manetrouting \
		-Inetworklayer/manetrouting/aodv-uu \
		-Inetworklayer/manetrouting/aodv-uu/aodv-uu \
		-Inetworklayer/manetrouting/base \
		-Inetworklayer/manetrouting/batman \
		-Inetworklayer/manetrouting/batman/batmand \
		-Inetworklayer/manetrouting/batman/batmand/orig \
		-Inetworklayer/manetrouting/dsdv \
		-Inetworklayer/manetrouting/dsr \
		-Inetworklayer/manetrouting/dsr/dsr-uu \
		-Inetworklayer/manetrouting/dymo \
		-Inetworklayer/manetrouting/dymo/dymoum \
		-Inetworklayer/manetrouting/dymo_fau \
		-Inetworklayer/manetrouting/olsr \
		-Inetworklayer/mpls \
		-Inetworklayer/ospfv2 \
		-Inetworklayer/ospfv2/interface \
		-Inetworklayer/ospfv2/messagehandler \
		-Inetworklayer/ospfv2/neighbor \
		-Inetworklayer/ospfv2/router \
		-Inetworklayer/routing \
		-Inetworklayer/routing/aodv \
		-Inetworklayer/routing/dymo \
		-Inetworklayer/routing/gpsr \
		-Inetworklayer/routing/rip \
		-Inetworklayer/rsvp_te \
		-Inetworklayer/ted \
		-Inetworklayer/xmipv6 \
		-Inodes \
		-Inodes/aodv \
		-Inodes/bgp \
		-Inodes/dymo \
		-Inodes/ethernet \
		-Inodes/gpsr \
		-Inodes/httptools \
		-Inodes/inet \
		-Inodes/internetcloud \
		-Inodes/ipv6 \
		-Inodes/mpls \
		-Inodes/ospfv2 \
		-Inodes/rip \
		-Inodes/rtp \
		-Inodes/wireless \
		-Inodes/xmipv6 \
		-Istatus \
		-Itransport \
		-Itransport/contract \
		-Itransport/rtp \
		-Itransport/rtp/profiles \
		-Itransport/rtp/profiles/avprofile \
		-Itransport/sctp \
		-Itransport/tcp \
		-Itransport/tcp_common \
		-Itransport/tcp/flavours \
		-Itransport/tcp_lwip \
		-Itransport/tcp_lwip/lwip \
		-Itransport/tcp_lwip/lwip/core \
		-Itransport/tcp_lwip/lwip/include \
		-Itransport/tcp_lwip/lwip/include/arch \
		-Itransport/tcp_lwip/lwip/include/ipv4 \
		-Itransport/tcp_lwip/lwip/include/ipv4/lwip \
		-Itransport/tcp_lwip/lwip/include/ipv6 \
		-Itransport/tcp_lwip/lwip/include/ipv6/lwip \
		-Itransport/tcp_lwip/lwip/include/lwip \
		-Itransport/tcp_lwip/lwip/include/netif \
		-Itransport/tcp_lwip/queues \
		-Itransport/tcp_nsc \
		-Itransport/tcp_nsc/queues \
		-Itransport/tcp/queues \
		-Itransport/udp \
		-Iutil \
		-Iutil/headerserializers \
		-Iutil/headerserializers/headers \
		-Iutil/headerserializers/ipv4 \
		-Iutil/headerserializers/ipv4/headers \
		-Iutil/headerserializers/ipv6 \
		-Iutil/headerserializers/ipv6/headers \
		-Iutil/headerserializers/sctp \
		-Iutil/headerserializers/sctp/headers \
		-Iutil/headerserializers/tcp \
		-Iutil/headerserializers/tcp/headers \
		-Iutil/headerserializers/udp \
		-Iutil/headerserializers/udp/headers \
		-Iutil/messageprinters \
		-Iworld \
		-Iworld/annotations \
		-Iworld/httptools \
		-Iworld/obstacles \
		-Iworld/radio \
		-Iworld/scenario \
		-Iworld/traci

MAKEMAKE_OPTIONS := -f --deep -o INET -O out -pINET $(INCLUDE_PATH)

makefiles: makefiles-so

makefiles-so: $(FEATURES_H)
	@FEATURE_OPTIONS=$$(opp_featuretool options -f -l) && cd src && opp_makemake --make-so $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-lib: $(FEATURES_H)
	@FEATURE_OPTIONS=$$(opp_featuretool options -f -l) && cd src && opp_makemake --make-lib $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-exe: $(FEATURES_H)
	@FEATURE_OPTIONS=$$(opp_featuretool options -f -l) && cd src && opp_makemake $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

# generate an include file that contains all the WITH_FEATURE macros according to the current enablement of features
$(FEATURES_H): $(wildcard .oppfeaturestate) .oppfeatures
	@opp_featuretool defines >$(FEATURES_H)

doxy:
	doxygen doxy.cfg

tcptut:
	cd doc/src/tcp && $(MAKE)

