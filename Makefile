all: checkmakefiles
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	@cd src && $(MAKE) MODE=release clean
	@cd src && $(MAKE) MODE=debug clean
	@rm -f src/Makefile

makefiles:
	cd src && opp_makemake -f --deep --make-so \
	        -I. \
	        -Iapplications \
	        -Iapplications/ethernet \
	        -Iapplications/generic \
	        -Iapplications/pingapp \
	        -Iapplications/rtpapp \
	        -Iapplications/sctpapp \
	        -Iapplications/tcpapp \
	        -Iapplications/udpapp \
	        -Ibase \
	        -Ilinklayer \
	        -Ilinklayer/contract \
	        -Ilinklayer/ethernet \
	        -Ilinklayer/etherswitch \
	        -Ilinklayer/ext \
	        -Ilinklayer/ieee80211 \
	        -Ilinklayer/ieee80211/mac \
	        -Ilinklayer/ieee80211/mgmt \
	        -Ilinklayer/mf80211 \
	        -Ilinklayer/mf80211/macLayer \
	        -Ilinklayer/mf80211/phyLayer \
	        -Ilinklayer/mf80211/phyLayer/decider \
	        -Ilinklayer/mf80211/phyLayer/snrEval \
	        -Ilinklayer/mfcore \
	        -Ilinklayer/ppp \
	        -Ilinklayer/radio \
	        -Imobility \
	        -Inetworklayer \
	        -Inetworklayer/arp \
	        -Inetworklayer/autorouting \
	        -Inetworklayer/common \
	        -Inetworklayer/contract \
	        -Inetworklayer/extras \
	        -Inetworklayer/icmpv6 \
	        -Inetworklayer/ipv4 \
	        -Inetworklayer/ipv6 \
	        -Inetworklayer/ldp \
	        -Inetworklayer/mpls \
	        -Inetworklayer/ospfv2 \
	        -Inetworklayer/ospfv2/interface \
	        -Inetworklayer/ospfv2/messagehandler \
	        -Inetworklayer/ospfv2/neighbor \
	        -Inetworklayer/ospfv2/router \
	        -Inetworklayer/queue \
	        -Inetworklayer/rsvp_te \
	        -Inetworklayer/ted \
	        -Inodes \
	        -Inodes/adhoc \
	        -Inodes/ethernet \
	        -Inodes/inet \
	        -Inodes/ipv6 \
	        -Inodes/mpls \
	        -Inodes/wireless \
	        -Itransport \
	        -Itransport/contract \
	        -Itransport/rtp \
	        -Itransport/rtp/profiles \
	        -Itransport/rtp/profiles/avprofile \
	        -Itransport/sctp \
	        -Itransport/tcp \
	        -Itransport/tcp/flavours \
	        -Itransport/tcp_nsc \
	        -Itransport/tcp_nsc/queues \
	        -Itransport/tcp_old \
	        -Itransport/tcp_old/flavours \
	        -Itransport/tcp_old/queues \
	        -Itransport/tcp/queues \
	        -Itransport/udp \
	        -Iutil \
	    -Iutil/headerserializers \
	    -Iutil/headerserializers/headers \
	    -Iworld \
	     -o INET -O out -pINET $$NSC_VERSION_DEF

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

doxy:
	doxygen doxy.cfg

tcptut:
	cd doc/src/tcp && $(MAKE)
