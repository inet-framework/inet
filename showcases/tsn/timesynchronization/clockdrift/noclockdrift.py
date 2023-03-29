import math
from omnetpp.scave import results, chart, utils

def get_noclockdrift_delay(unit_factor=1, debug=False):
    filter_expression = """type =~ scalar AND runattr:experiment =~ NoClockDrift AND runattr:replication =~ "#0" AND module =~ "ClockDriftShowcase.sink1.app[0].sink" AND name =~ meanBitLifeTimePerPacket:histogram:mean"""
    fe_stddev = """type =~ scalar AND runattr:experiment =~ NoClockDrift AND runattr:replication =~ "#0" AND module =~ "ClockDriftShowcase.sink1.app[0].sink" AND name =~ meanBitLifeTimePerPacket:histogram:stddev"""
    
    start_time = float(-math.inf)
    end_time = float(math.inf)
    
    try:
        df = results.get_scalars(filter_expression, include_fields=True, include_attrs=True, include_runattrs=True, include_itervars=True)
        df_stddev = results.get_scalars(fe_stddev, include_fields=True, include_attrs=True, include_runattrs=True, include_itervars=True)
    except results.ResultQueryError as e:
        raise chart.ChartScriptError("Error while querying results: " + str(e))
    
    if df.empty or df_stddev.empty:
        raise chart.ChartScriptError("The result filter returned no data.")
    # assert float(df_stddev.value) == 0, f"the stddev is not 0, but {float(df_stddev.value)}\n"
    if debug: print('noclockdrift delay: ' + str(float(df.value*unit_factor)))
    return float(df.value*unit_factor)