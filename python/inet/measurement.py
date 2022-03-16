# Measurement
# ===========
#
# what/where/how/when/why/who/which/...
#
#  - what do we measure?
#    number of existing things
#    number of passing things
#    rate of things
#    state of things
#    life time????
#    length of things?
#  - of what kind of things?
#    packets
#    regions
#    bits
#    time
#    ???
#  - where do we aggregate?
#    packet
#    region
#  - how do we aggregate?
#    we do not
#    min
#    max
#    mean
#    sum
#  - when do we measure?
#    at a(ny) given moment
#    over a time period
#    over the whole simulation
#    periodically
#    on specific events
#  - where do we measure?
#    network
#    subnetwork
#    network node
#    network interface
#    channel
#    application
#    protocol
#    source/sink/queue/filter/meter/module/etc.
#  - which things?
#    all of them
#    some of them that belongs to flow
#  - what is the result?
#    number
#    continuous function of time
#    discrete function of time
#  - how do we measure?
#    we keep track of the current count of packets only -> scalar
#    we remember how the count of packets change over time -> vector
#  - in what unit?
#    seconds/bits/etc.
#
# Measurement = what x kind x when x where x which x how
# all >> meaningful >> implemented >> enabled by default

def makeMeasurement(what, kind, aggregate, when, where, which, how):
   return "We measure the " + what + " " + kind + " " + when + " " + where + " " + which + " " + how

makeMeasurement("number of existing", "packets", "at any given moment", "in all network interfaces separately", "that belong to a specific flow", "as a vector of values throughout the simulation")

# End-to-end delay: what x kind x aggregate x when x where x which x how?
