.. _ug:cha:History:

History
=======

.. _ug:sec:history:ipsuite-to-inet:

IPSuite to INET Framework (2000-2006)
-------------------------------------

The predecessor of the INET framework was written by Klaus Wehrle,
Jochen Reber, Dirk Holzhausen, Volker Boehm, Verena Kahmann, Ulrich
Kaage and others at the University of Karlsruhe during 2000-2001, under
the name IPSuite.

The MPLS, LDP and RSVP-TE models were built as an add-on to IPSuite
during 2003 by Xuan Thang Nguyen (Xuan.T.Nguyen@uts.edu.au) and other
students at the University of Technology, Sydney under supervision of Dr
Robin Brown. The package consisted of around 10,000 LOCs, and was
published at http://charlie.it.uts.edu.au/ tkaphan/xtn/capstone (now
unavailable).

After a period of IPSuite being unmaintained, Andras Varga took over the
development in July 2003. Through a series of snapshot releases in
2003-2004, modules got completely reorganized, documented, and many of
them rewritten from scratch. The MPLS models (including RSVP-TE, LDP,
etc) also got refactored and merged into the codebase.

During 2004, Andras added a new, modular and extensible TCP
implementation, application models, Ethernet implementation and an
all-in-one IP model to replace the earlier, modularized one.

The package was renamed INET Framework in October 2004.

Support for wireless and mobile networks got added during summer 2005 by
using code from the Mobility Framework.

The MPLS models (including LDP and RSVP-TE) got revised and mostly
rewritten from scratch by Vojta Janota in the first half of 2005 for his
diploma thesis. After further refinements by Vojta, the new code got
merged into the INET CVS in fall 2005, and got eventually released in
the March 2006 INET snapshot.

The OSPFv2 model was created by Andras Babos during 2004 for his diploma
thesis which was submitted early 2005. This work was sponsored by Andras
Varga, using revenues from commercial OMNEST licenses. After several
refinements and fixes, the code got merged into the INET Framework in
2005, and became part of the March 2006 INET snapshot.

The Quagga routing daemon was ported into the INET Framework also by
Vojta Janota. This work was also sponsored by Andras Varga. During fall
2005 and the months after, ripd and ospfd were ported, and the
methodology of porting was refined. Further Quagga daemons still remain
to be ported.

Based on experience from the IPv6Suite (from Ahmet Sekercioglu’s group
at CTIE, Monash University, Melbourne) and IPv6SuiteWithINET (Andras’s
effort to refactor IPv6Suite and merge it with INET early 2005), Wei
Yang Ng (Monash Uni) implemented a new IPv6 model from scratch for the
INET Framework in 2005 for his diploma thesis, under guidance from
Andras who was visiting Monash between February and June 2005. This IPv6
model got first included in the July 2005 INET snapshot, and gradually
refined afterwards.

The SCTP implementation was contributed by Michael Tuexen, Irene
Ruengeler and Thomas Dreibholz

Support for Sam Jensen’s Network Simulation Cradle, which makes
real-world TCP stacks available in simulations was added by Zoltan
Bojthe in 2010.

TCP SACK and New Reno implementation was contributed by Thomas Reschka.

Several other people have contributed to the INET Framework by providing
feedback, reporting bugs, suggesting features and contributing patches;
I’d like to acknowledge their help here as well.
