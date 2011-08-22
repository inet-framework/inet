all: checkmakefiles
	cd src && $(MAKE)
	cd examples && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean
	cd examples && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd examples && $(MAKE) MODE=debug clean
	rm -f src/Makefile
	rm -f examples/Makefile

makefiles:
	cd src && opp_makemake -f --deep --make-so -o inet -O out -Xapplications/voiptool -Xnetworklayer/ipv6tunneling -Xnetworklayer/xmipv6 -Xnodes/xmipv6 -Xtransport/tcp_nsc -DWITH_TCP_COMMON -DWITH_TCP_INET -DWITH_TCP_LWIP -DWITH_IPv4 -DWITH_IPv6 -DWITH_UDP -DWITH_RTP -DWITH_SCTP -DWITH_ETHERNET -DWITH_PPP -DWITH_EXT_IF -DWITH_MPLS -DWITH_OSPFv2 -DWITH_BGPv4 -DWITH_MANET
	cd examples && opp_makemake -f --deep --make-so -o inetexamples -O out -Xapplications/voiptool -Xnetworklayer/ipv6tunneling -Xnetworklayer/xmipv6 -Xnodes/xmipv6 -Xtransport/tcp_nsc -DWITH_TCP_COMMON -DWITH_TCP_INET -DWITH_TCP_LWIP -DWITH_IPv4 -DWITH_IPv6 -DWITH_UDP -DWITH_RTP -DWITH_SCTP -DWITH_ETHERNET -DWITH_PPP -DWITH_EXT_IF -DWITH_MPLS -DWITH_OSPFv2 -DWITH_BGPv4 -DWITH_MANET

checkmakefiles:
	@if [ ! -f src/Makefile -o ! -f examples/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile or examples/Makefile does not exist.' \
	echo 'Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

doxy:
	doxygen doxy.cfg

tcptut:
	cd doc/src/tcp && $(MAKE)
