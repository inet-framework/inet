sudo apt install icecc
sudo apt install icecc-monitor

sudo systemctl enable iceccd
sudo systemctl enable icecc-scheduler

export CCACHE_PREFIX=icecc

make -j30
idemon&
