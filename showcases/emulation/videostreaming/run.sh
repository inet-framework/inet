# start streaming server
cvlc RickAstley.mkv --loop --sout '#transcode{vcodec=h264,acodec=mpga,vb=125k,ab=64k,deinterlace,scale=0.25,threads=2}:rtp{mux=ts,dst=192.168.2.99,port=4004}' &

# start streaming client
vlc rtp://192.168.3.20:4004 &

# start simulation
inet -u Cmdenv -f omnetpp.ini

# kill child processes
trap 'kill $(jobs -pr)' SIGINT SIGTERM EXIT
