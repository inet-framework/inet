import re
import sys
import subprocess
import os

from sys import exit  # pylint: disable=redefined-builtin

from mininet.cli import CLI
from mininet.log import setLogLevel, info, error
from mininet.net import Mininet
from mininet.link import Intf
from mininet.topo import LinearTopo
from mininet.util import quietRun


def checkIntf( intf ):
    "Make sure intf exists and is not configured."
    config = quietRun( 'ifconfig %s 2>/dev/null' % intf, shell=True )
    if not config:
        error( 'Error:', intf, 'does not exist!\n' )
        exit( 1 )
    ips = re.findall( r'\d+\.\d+\.\d+\.\d+', config )
    if ips:
        error( 'Error:', intf, 'has an IP address,'
               'and is probably in use!\n' )
        exit( 1 )


setLogLevel( 'info' )

subprocess.run(['sudo', 'ip', '-all', 'netns', 'delete'])

info( '*** Creating network\n' )
net = Mininet( topo=LinearTopo( k=0, n=2 ), controller=None)

host1 = net.addHost('h1')
host2 = net.addHost('h2')

# create tap interfaces
subprocess.run(['bash', '-c', 'sudo tunctl -t tapa -u $SAVED_USER -g $SAVED_GROUP && sudo ip link set dev tapa up'])
subprocess.run(['bash', '-c', 'sudo tunctl -t tapb -u $SAVED_USER -g $SAVED_GROUP && sudo ip link set dev tapb up'])

info( '*** Checking', 'tapa', '\n' )
checkIntf( 'tapa' )

info( '*** Checking', 'tapb', '\n' )
checkIntf( 'tapb' )

info( '*** Adding hardware interface', 'tapa', 'to host',
      host1.name, '\n' )
_intf = Intf( 'tapa', node=host1, ip='192.168.2.20/24' )

info( '*** Adding hardware interface', 'tapb', 'to host',
      host2.name, '\n' )
_intf = Intf( 'tapb', node=host2, ip='192.168.3.20/24' )

info( '*** Note: you may need to reconfigure the interfaces for '
      'the Mininet hosts:\n', net.hosts, '\n' )

pid1 = net.hosts[0].pid
pid2 = net.hosts[1].pid

cmd1 = """sudo ip netns attach host1 """ + str(pid1)
cmd2 = """sudo ip netns attach host2 """ + str(pid2)

subprocess.run(['bash', '-c', str(cmd1)])
subprocess.run(['bash', '-c', str(cmd2)])

net.start()

# uncomment to run wireshark on host
#host1.cmd("wireshark -i tapa -k &")
#host2.cmd("wireshark -i tapb -k &")

user = os.getenv('SAVED_USER')
group = os.getenv('SAVED_GROUP')
saved_path = os.getenv('SAVED_PATH')
saved_env = os.environ.copy()
saved_env["PATH"] = saved_path

# start inet

# Note: Mininet runs as root, but we want to run the simulation as the normal user. We use 'sudo' to specify the user.
# We also need to restore the original PATH which is overwritten by 'sudo'.
subprocess.run(['inet -f omnetpp.ini -c Mininet -u Cmdenv &'], user=user, group=group, env=saved_env, shell=True)

# Note: Similarly, we want to use VLC as the normal user.
host1.cmd("sudo -E -u $SAVED_USER xterm -e cvlc RickAstley.mkv --loop --sout '#transcode{vcodec=h264,acodec=mpga,vb=125k,ab=64k,deinterlace,scale=0.25,threads=2}:rtp{mux=ts,dst=192.168.2.99,port=4004}' &")
host2.cmd("""sudo -E -u $SAVED_USER xterm -e 'export PATH=$SAVED_PATH;bash -c "vlc rtp://192.168.3.20:4004"' """)

CLI( net )

subprocess.run(['sudo', 'ip', '-all', 'netns', 'delete'])

net.stop()
