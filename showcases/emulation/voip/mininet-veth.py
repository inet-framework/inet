from mininet.net import Mininet
from mininet.node import Controller
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.node import OVSController
from mininet.link import TCIntf

setLogLevel( 'info' )

net = Mininet( controller = OVSController, waitConnected=True, intf=TCIntf )

info( '*** Adding controller\n' )
net.addController( 'c0' )

info( '*** Adding hosts\n' )
h1 = net.addHost( 'h1', ip='192.168.2.1/24' )
h2 = net.addHost( 'h2', ip='192.168.2.2/24' )

info( '*** Adding switch\n' )
s3 = net.addSwitch( 's3' )

info( '*** Creating links\n' )
net.addLink( h1, s3 )
net.addLink( h2, s3 )

info( '*** Configuring link bandwidth and packet loss\n')
h1.intf().config(bw=10, loss=10, delay='10ms', jitter='1ms')     # bandwidth in Mpbs, packet loss in %

info( '*** Starting network\n')
net.start()

info( '*** Starting INET simulations\n')
# Note: Mininet runs as root, but we want to run the simulation as the normal user. We use 'sudo' to specify the user.
# We also need to restore the original PATH which is overwritten by 'sudo'.
h1.cmd('sudo -E -u $SAVED_USER bash -c "export PATH=$SAVED_PATH && inet -s -c VoipSenderMininet &"')
h2.cmd('sudo -E -u $SAVED_USER bash -c "export PATH=$SAVED_PATH && inet -s -c VoipReceiverMininet &"')

info( '*** Running CLI\n' )
CLI( net )

info( '*** Stopping network' )
net.stop()