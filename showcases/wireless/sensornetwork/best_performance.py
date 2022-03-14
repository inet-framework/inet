from omnetpp.scave import results, chart, utils

debug = False

def get_best_performing_dataframe_separately_packetreceived():
    """Selects the runs corresponding to the most received packets,
    and returns them in three separate dataframes for the three configurations"""
    
    # filter expressions for packetReceived:count for the three configurations
    filter_expression_bmac = """type =~ scalar AND isfield =~ false AND runattr:configname =~ "StatisticBMac" AND module =~ "*.server.app[0]" AND name =~ packetReceived:count"""
    filter_expression_xmac = """type =~ scalar AND isfield =~ false AND runattr:configname =~ "StatisticXMac" AND module =~ "*.server.app[0]" AND name =~ packetReceived:count"""
    filter_expression_lmac = """type =~ scalar AND isfield =~ false AND runattr:configname =~ "StatisticLMac" AND module =~ "*.server.app[0]" AND name =~ packetReceived:count"""

    # dataframes for packetReceived:count    
    df_b = results.get_scalars(filter_expression_bmac, include_fields=False, include_attrs=True, include_runattrs=True, include_itervars=True)
    df_x = results.get_scalars(filter_expression_xmac, include_fields=False, include_attrs=True, include_runattrs=True, include_itervars=True)
    df_l = results.get_scalars(filter_expression_lmac, include_fields=False, include_attrs=True, include_runattrs=True, include_itervars=True)
    
    # average repetitions    
    df_b = df_b.groupby(['name','configname','module','iterationvars','inifile'], as_index=False).mean()
    if debug:
        print("\ndf_b after averaging:\n-------------------",df_b)
    df_x = df_x.groupby(['name','configname','module','iterationvars','inifile'], as_index=False).mean()
    if debug:
        print("\ndf_x after averaging:\n-------------------",df_x)
    df_l = df_l.groupby(['name','configname','module','iterationvars','inifile'], as_index=False).mean()
    if debug:
        print("\ndf_l after averaging:\n-------------------",df_l)
    
    # get index of the most received packets
    df_b_max_index = df_b[['value']].idxmax()
    df_x_max_index = df_x[['value']].idxmax()
    df_l_max_index = df_l[['value']].idxmax()

    # create dataframe containing only the run with the most received packets
    df_b = df_b.iloc[df_b_max_index]
    df_x = df_x.iloc[df_x_max_index]
    df_l = df_l.iloc[df_l_max_index]
    
    if debug:
        print("----\nget_best_performing_dataframe_separately_packetreceived()\n----")
        print("returning ", df_b, "\n", df_x, "\n", df_l, "\n")
    
    return df_b, df_x, df_l
    
    # print("Return: ", df_b, df_l, df_x)
    
def get_itervars_from_multiple_dataframes(df_b, df_x, df_l):
    """Returns the iteration variables from three dataframes supplied as arguments,
    so the return values can be used to index into dataframes by iteration variable."""
    
    best_itervar_b = df_b['iterationvars'].iloc[0]
    best_itervar_x = df_x['iterationvars'].iloc[0]
    best_itervar_l = df_l['iterationvars'].iloc[0]
    
    if debug:
        print("Best itervars:", best_itervar_b, best_itervar_l, best_itervar_x)
    
    return best_itervar_b, best_itervar_x, best_itervar_l

def merge_dataframes(*args):
    df = args[0]
    for i in range(1,len(args)):
        df = df.append(args[i])
    return df

def merge_dataframes(*args):
    df = args[0]
    for i in args[1:]:
        df = df.append(i)
    return df

def sum_dataframe(df):
    """For summing per-module residualEnergyCapacity values"""
    
    df_sum = df.groupby(['name','configname','iterationvars','inifile'], as_index=False).sum() 
    return df_sum

def get_power_for_each_module_dataframes():
    """Returns the per-module residualEnergyCapacity values for the three configurations in three separate dataframes."""
    
    filter_expression = """* AND name =~ residualEnergyCapacity:last"""
    
    filter_expression_bmac = """type =~ scalar AND isfield =~ false AND runattr:configname =~ "StatisticBMac" AND """ + filter_expression
    filter_expression_xmac = """type =~ scalar AND isfield =~ false AND runattr:configname =~ "StatisticXMac" AND """ + filter_expression
    filter_expression_lmac = """type =~ scalar AND isfield =~ false AND runattr:configname =~ "StatisticLMac" AND """ + filter_expression
    
    df_b = results.get_scalars(filter_expression_bmac, include_fields=False, include_attrs=True, include_runattrs=True, include_itervars=True)
    df_x = results.get_scalars(filter_expression_xmac, include_fields=False, include_attrs=True, include_runattrs=True, include_itervars=True)
    df_l = results.get_scalars(filter_expression_lmac, include_fields=False, include_attrs=True, include_runattrs=True, include_itervars=True)
    
    # average repetitions    
    df_b = df_b.groupby(['name','configname','module', 'iterationvars','inifile'], as_index=False).mean()
    # print("\ndf_b after averaging:\n-------------------",df_b)
    df_x = df_x.groupby(['name','configname','module', 'iterationvars','inifile'], as_index=False).mean()
    # print("\ndf_x after averaging:\n-------------------",df_x)
    df_l = df_l.groupby(['name','configname','module', 'iterationvars','inifile'], as_index=False).mean()
    # print("\ndf_l after averaging:\n-------------------",df_l)
    
    print("get_power_dataframes\nreturning:")
    print("df_b:\n", df_b)
    print("df_x:\n", df_x)
    print("df_l:\n", df_l)
    
    return df_b, df_x, df_l

def index_into_dataframe_with_itervar(df, itervar):
    
    return_df = df[df['iterationvars']==itervar]
    print("index_into_dataframe_with_itervar\nreturning ", return_df)
    return return_df

def get_best_power_dataframes_for_most_packets_received():
    """Returns the residualEnergyCapacity values summed for all modules, in three separate dataframes for the three configurations."""
    
    bmac_power, xmac_power, lmac_power = get_power_for_each_module_dataframes()

    best_itervar_b, best_itervar_x, best_itervar_l = get_itervars_from_multiple_dataframes(*get_best_performing_dataframe_separately_packetreceived())
    
    bmac_best_power = index_into_dataframe_with_itervar(bmac_power, best_itervar_b)
    print("BMAC BEST POWER: ", bmac_best_power)
    
    xmac_best_power = index_into_dataframe_with_itervar(xmac_power, best_itervar_x)
    print("XMAC BEST POWER: ", xmac_best_power)
    
    lmac_best_power = index_into_dataframe_with_itervar(lmac_power, best_itervar_l)
    print("LMAC BEST POWER: ", lmac_best_power)
    
    bmac_best_power_sum = sum_dataframe(bmac_best_power)
    print("BMAC BEST POWER SUM: ", bmac_best_power_sum)
    
    xmac_best_power_sum = sum_dataframe(xmac_best_power)
    print("XMAC BEST POWER SUM: ", xmac_best_power_sum)
    
    lmac_best_power_sum = sum_dataframe(lmac_best_power)
    print("LMAC BEST POWER SUM: ", lmac_best_power_sum)
    
    return bmac_best_power_sum, xmac_best_power_sum, lmac_best_power_sum
    
    