Topology based on OSPF Networking Laboratory from Virtual Network User Mode 
Linux (VNUML) project.

To verify the validity of routing tables produced by Quagga/INET, results can
be compared with those produced by Quagga running in UML based scenario.

Regarding UML based simulation run:

Although the topology presented should be identical to that from VNUML website
(as of version 1.7), VNUML itself wasn't used for the simulation run. Instead
customized version of ttylinux distribution and blaisorblade kernel was used.
Also since I wasn't able to simulate the topology as a whole on my machine, all
non-router nodes were removed from the topology to save some resources during
the simulation run. However this should have no effect on the routers
themselves. 

References:

VNUML project: http://www.dit.upm.es/vnumlwiki/index.php/Main_Page
blaisorblade: http://www.user-mode-linux.org/~blaisorblade/
ttylinux: http://www.minimalinux.org/ttylinux/

Feel free to send questions and comments to vojta.janota@gmail.com
