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

def setupHost(host, netns, tap_intf, ip):
    "Creates TAP interface in the host OS, assigns IP address to it, creates Mininet host and attaches TAP interface to it."
    info( '*** Adding host', host, 'to Mininet network', '\n' )
    python_host = net.addHost(host)
    info( '*** Adding tap interface', tap_intf, 'in host OS', '\n' )
    subprocess.run(['bash', '-c', f'sudo tunctl -t {tap_intf} -u $SAVED_USER -g $SAVED_GROUP && sudo ip link set dev {tap_intf} up'])
    info( '*** Checking', tap_intf, '\n' )
    checkIntf( tap_intf )
    info( '*** Adding hardware interface', tap_intf, 'to Mininet host', host, '\n' )
    _intf = Intf( tap_intf, node=python_host, ip=ip )
    info( '*** Attaching mininet tap interface to host OS tap interface', tap_intf, '\n' )
    host_pid = python_host.pid
    subprocess.run(['bash', '-c', f'sudo ip netns attach {netns} {host_pid}'])
    return python_host


setLogLevel( 'info' )

subprocess.run(['sudo', 'ip', '-all', 'netns', 'delete'])

info( '*** Creating network\n' )
net = Mininet( topo=LinearTopo( k=0, n=2 ), controller=None)

host1 = setupHost(host='h1', netns='host1', tap_intf='tapa', ip='192.168.2.20/24')
host2 = setupHost(host='h2', netns='host2', tap_intf='tapb', ip='192.168.3.20/24')

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
