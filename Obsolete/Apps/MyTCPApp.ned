// from MPLS models  --- FIXME merge or eliminate!

simple MyTCPApp
    parameters:
        local_addr        : string,
        server_addr        : string,
        timeout            : numeric,
        data_interarrival_time    : numeric,
        start_time        : numeric;
    gates: 
        in: in; 
        out: out; 
endsimple 
