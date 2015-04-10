all: checkmakefiles
	cd src && $(MAKE) all

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile


MAKEMAKE_OPTIONS := -f --deep -o INET -O out -pINET --no-deep-includes -Xinet/applications/voipstream -Xinet/linklayer/ext -Xinet/transportlayer/tcp_lwip -Xinet/transportlayer/tcp_nsc -I../src -DWITH_TCP_COMMON -DWITH_TCP_INET -DWITH_IPv4 -DWITH_IPv6 -DWITH_xMIPv6 -DWITH_UDP -DWITH_RTP -DWITH_SCTP -DWITH_DHCP -DWITH_ETHERNET -DWITH_PPP -DWITH_MPLS -DWITH_OSPFv2 -DWITH_BGPv4 -DWITH_MANET -DWITH_TRACI -DWITH_AODV -DWITH_RIP -DWITH_RADIO -DWITH_POWER -DWITH_IEEE80211 -DWITH_GENERIC -DWITH_IDEALWIRELESS -DWITH_FLOOD -DWITH_PIM -DWITH_IEEE802154 -DWITH_APSKRADIO -DWITH_TUN -DWITH_BMAC -DWITH_LMAC -DWITH_CSMA
	
makefiles:
	cd src && opp_makemake --make-so $(MAKEMAKE_OPTIONS) 

makefiles-static:
	cd src && opp_makemake $(MAKEMAKE_OPTIONS) 


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
