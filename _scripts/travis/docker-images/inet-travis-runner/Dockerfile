FROM omnetpp/omnetpp-core-triplatform

# ccache to speed up compilation for fingerprint (not build-only) test jobs,
# and parts of ffmpeg for the VoIPStream modules
RUN apt-get install -y --no-install-recommends ccache libavcodec-dev libavformat-dev libavutil-dev libavresample-dev
