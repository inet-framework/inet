import itertools
import logging
import numpy
import random

from omnetpp.scave.results import *

from inet.simulation.project import *
from inet.test.simulation import *

logger = logging.getLogger(__name__)

class ValidationTestTask(SimulationTestTask):
    def __init__(self, simulation_task, check_function, name="validation test", **kwargs):
        super().__init__(simulation_task, name=name, **kwargs)
        self.check_function = check_function

    def check_simulation_task_result(self, simulation_task_result, **kwargs):
        return self.check_function(**kwargs)

def compare_test_results(result1, result2, accuracy=0.01):
    return abs(result1 - result2) / result1 < accuracy

############################
# TSN frame replication test

# Observed result
# 0.6578411405295316
def compute_frame_replication_success_rate_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND ((module =~ "*.destination.udp" AND name =~ packetReceived:count) OR (module =~ "*.source.udp" AND name =~ packetSent:count))"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/framereplication/results/*.sca"), filter_expression=filter_expression)
    df = get_scalars(df)
    packetSent = float(df[df.name == "packetSent:count"].value)
    packetReceived = float(df[df.name == "packetReceived:count"].value)
    return packetReceived / packetSent

def compute_frame_replication_success_rate_analytically1():
    combinations = numpy.array(list(itertools.product([0, 1], repeat=7)))
    probabilities = numpy.array([0.8, 0.8, 0.64, 0.8, 0.64, 0.8, 0.8])
    solutions = numpy.array([[1, 1, 1, 0, 0, 0, 0], [1, 1, 0, 0, 1, 1, 0], [1, 0, 0, 1, 1, 0, 0,], [1, 0, 1, 1, 0, 0, 1]])
    p = 0
    for combination in combinations:
        probability = (combination * probabilities + (1 - combination) * (1 - probabilities)).prod()
        for solution in solutions:
            if (solution * combination == solution).all():
                p += probability
                break
    return p

def compute_frame_replication_success_rate_analytically2():
    successful = 0
    n = 1000000
    for i in range(n):
        s1 = random.random() < 0.8
        s1_s2a = random.random() < 0.8
        s1_s2b = random.random() < 0.8
        s2a_s2b = random.random() < 0.8
        s2b_s2a = random.random() < 0.8
        s2a = (s1 and s1_s2a) or (s1 and s1_s2b and s2b_s2a)
        s2b = (s1 and s1_s2b) or (s1 and s1_s2a and s2a_s2b)
        s3a = s2a and (random.random() < 0.8)
        s3b = s2b and (random.random() < 0.8)
        s3a_s4 = random.random() < 0.8
        s3b_s4 = random.random() < 0.8
        s4 = (s3a and s3a_s4) or (s3b and s3b_s4)
        if s4:
            successful += 1
    return successful / n

def compute_tsn_framereplication_validation_test_results(test_accuracy=0.01, **kwargs):
    ps = compute_frame_replication_success_rate_from_simulation_results(**kwargs)
    pa1 = compute_frame_replication_success_rate_analytically1()
    pa2 = compute_frame_replication_success_rate_analytically2()
    test_result1 = compare_test_results(ps, pa1, test_accuracy)
    test_result2 = compare_test_results(ps, pa2, test_accuracy)
    return TestTaskResult(task=TestTask(), bool_result=test_result1 and test_result2)

def get_tsn_framereplication_simulation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(inet_project, working_directory_filter="tests/validation/tsn/framereplication", sim_time_limit="0.2s", **kwargs).tasks[0]
    return ValidationTestTask(simulation_task, compute_tsn_framereplication_validation_test_results, **kwargs)

def run_tsn_framereplication_validation_test(test_accuracy=0.01, **kwargs):
    return get_tsn_framereplication_simulation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

###################################################
# TSN traffic shaping asynchronous shaper ICCT test

# Observed result
# name                      max
# module
# Flow 1, CDT            505.16
# Flow 2, Class A       1579.72
# Flow 3, Class B       3371.72
# Flow 4, Class A        932.04
# Flow 5, Class B       2162.12
# Flow 6, Class A        585.16
# Flow 7, Class B       1963.24
# Flow 8, Best Effort  34913.16
def compute_asynchronousshaper_icct_endtoend_delay_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND name =~ meanBitLifeTimePerPacket:histogram:max"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/asynchronousshaper/icct/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    df["name"] = df["name"].map(lambda name: re.sub(".*(min|max)", "\\1", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*N6.app\\[[0-4]\\].*", "Flow 4, Class A", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*N6.app\\[[5-9]\\].*", "Flow 5, Class B", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*N7.app\\[[0-9]\\].*", "Flow 1, CDT", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*N7.app\\[1[0-9]\\].*", "Flow 2, Class A", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*N7.app\\[2[0-9]\\].*", "Flow 3, Class B", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*N7.app\\[3[0-4]\\].*", "Flow 6, Class A", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*N7.app\\[3[5-9]\\].*", "Flow 7, Class B", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*N7.app\\[40\\].*", "Flow 8, Best Effort", name))
    df = pd.pivot_table(df, index="module", columns="name", values="value", aggfunc=max)
    return df * 1000000

def compute_asynchronousshaper_icct_endtoend_delay_alternatively():
    # This validation test compares simulation results to analytical results presented
    # in the paper titled "The Delay Bound Analysis Based on Network Calculus for
    # Asynchronous Traffic Shaping under Parameter Inconsistency" from the 2020 IEEE
    # 20th International Conference on Communication Technology
    return pd.DataFrame(index = ["Flow 1, CDT", "Flow 2, Class A", "Flow 3, Class B", "Flow 4, Class A", "Flow 5, Class B", "Flow 6, Class A", "Flow 7, Class B", "Flow 8, Best Effort"],
                        data = {"max": [867.6, 4665.5, 10902, 2505.2, 5678.3, float('inf'), float('inf'), float('inf')]})

def compute_tsn_trafficshaping_asynchronousshaper_icct_validation_test_results(**kwargs):
    df1 = compute_asynchronousshaper_icct_endtoend_delay_from_simulation_results(**kwargs)
    df2 = compute_asynchronousshaper_icct_endtoend_delay_alternatively()
    test_result = (df1["max"] < df2["max"]).all()
    return TestTaskResult(task=TestTask(), bool_result=test_result)

def get_tsn_trafficshaping_asynchronousshaper_icct_simulation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(inet_project, working_directory_filter="tests/validation/tsn/trafficshaping/asynchronousshaper/icct", sim_time_limit="0.1s", **kwargs).tasks[0]
    return ValidationTestTask(simulation_task, compute_tsn_trafficshaping_asynchronousshaper_icct_validation_test_results, **kwargs)

def run_tsn_trafficshaping_asynchronousshaper_icct_validation_test(**kwargs):
    return get_tsn_trafficshaping_asynchronousshaper_icct_simulation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

########################################################
# TSN traffic shaping asynchronous shaper Core4INET test

# Observed result
# name          max        mean      min      stddev
# module
# Critical  375.780  298.477239  252.900   36.668923
# High      307.260  161.309427   60.900   73.625945
# Medium    535.939  247.258720   88.259  106.588325
def compute_asynchronousshaper_core4inet_endtoend_delay_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND (name =~ meanBitLifeTimePerPacket:histogram:min OR name =~ meanBitLifeTimePerPacket:histogram:max OR name =~ meanBitLifeTimePerPacket:histogram:mean OR name =~ meanBitLifeTimePerPacket:histogram:stddev)"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/asynchronousshaper/core4inet/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    df["name"] = df["name"].map(lambda name: re.sub(".*(min|max|mean|stddev)", "\\1", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*app\\[0\\].*", "Best effort", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*app\\[1\\].*", "Medium", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*app\\[2\\].*", "High", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*app\\[3\\].*", "Critical", name))
    df = df.loc[df["module"]!="Best effort"]
    df = pd.pivot_table(df, index="module", columns="name", values="value")
    return df * 1000000

# Observed result
# 2.0
def compute_asynchronousshaper_core4inet_max_queuelength_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND module =~ \"*.switch.eth[4].macLayer.queue.queue[5..7]\" AND name =~ queueLength:max"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/asynchronousshaper/core4inet/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    return numpy.max(df["value"])

def compute_asynchronousshaper_core4inet_endtoend_delay_alternatively(**kwargs):
    # This validation test compares simulation results to analytical results and also
    # to results from a different simulation using Core4INET.
    # https://github.com/CoRE-RG/CoRE4INET/tree/master/examples/tsn/medium_network
    df = pd.DataFrame(index=["Medium", "High", "Critical"],
                      data={"min": [88.16, 60.8, 252.8],
                            "max": [540, 307.2, 375.84],
                            "mean": [247.1, 161.19, 298.52],
                            "stddev": [106.53, 73.621, 36.633]})
    df["min"] -= 0.001 # 1 ns initial production offset, see INI file
    df = df + 0.05 * 2 # 50 ns propagation delay per hop
    df.index.set_names(["trafficclass"], inplace=True)
    df.columns.set_names(["name"], inplace=True)
    return df

def compute_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_results(test_accuracy=0.01, **kwargs):
    df1 = compute_asynchronousshaper_core4inet_endtoend_delay_from_simulation_results(**kwargs)
    df2 = compute_asynchronousshaper_core4inet_endtoend_delay_alternatively(**kwargs)
    df1 = df1.sort_index(axis=0).sort_index(axis=1)
    df2 = df2.sort_index(axis=0).sort_index(axis=1)
    maxQueueLength = compute_asynchronousshaper_core4inet_max_queuelength_from_simulation_results()
    test_result = maxQueueLength < 4 and \
                  (df1["min"] >= df2["min"]).all() and \
                  (df1["max"] <= df2["max"]).all() and \
                  numpy.allclose(df1["min"], df2["min"], rtol=test_accuracy, atol=0) and \
                  numpy.allclose(df1["max"], df2["max"], rtol=test_accuracy, atol=0) and \
                  numpy.allclose(df1["mean"], df2["mean"], rtol=test_accuracy * 7, atol=0) and \
                  numpy.allclose(df1["stddev"], df2["stddev"], rtol=test_accuracy * 30, atol=0)
    return TestTaskResult(task=TestTask(), bool_result=test_result)

def get_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(inet_project, working_directory_filter="tests/validation/tsn/trafficshaping/asynchronousshaper/core4inet", sim_time_limit="1s", **kwargs).tasks[0]
    return ValidationTestTask(simulation_task, compute_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_results, **kwargs)

def run_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test(test_accuracy=0.01, **kwargs):
    return get_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_task(**kwargs).run(test_accuracy=test_accuracy, **kwargs)

##############################################
# TSN traffic shaping credit-based shaper test

# Observed result
# name          max        mean      min      stddev
# module
# Critical  375.780  298.477239  252.900   36.668923
# High      307.260  161.309427   60.900   73.625945
# Medium    535.939  247.258720   88.259  106.588325
def compute_creditbasedshaper_endtoend_delay_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND (name =~ meanBitLifeTimePerPacket:histogram:min OR name =~ meanBitLifeTimePerPacket:histogram:max OR name =~ meanBitLifeTimePerPacket:histogram:mean OR name =~ meanBitLifeTimePerPacket:histogram:stddev)"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/creditbasedshaper/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    df["name"] = df["name"].map(lambda name: re.sub(".*(min|max|mean|stddev)", "\\1", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*app\\[0\\].*", "Best effort", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*app\\[1\\].*", "Medium", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*app\\[2\\].*", "High", name))
    df["module"] = df["module"].map(lambda name: re.sub(".*app\\[3\\].*", "Critical", name))
    df = df.loc[df["module"]!="Best effort"]
    df = pd.pivot_table(df, index="module", columns="name", values="value")
    return df * 1000000

# Observed result
# 2.0
def compute_creditbasedshaper_max_queuelength_from_simulation_results(**kwargs):
    filter_expression = """type =~ scalar AND module =~ \"*.switch.eth[4].macLayer.queue.queue[5..7]\" AND name =~ queueLength:max"""
    df = read_result_files(inet_project.get_full_path("tests/validation/tsn/trafficshaping/creditbasedshaper/results/*.sca"), filter_expression=filter_expression, include_fields_as_scalars=True)
    df = get_scalars(df)
    return numpy.max(df["value"])

def compute_creditbasedshaper_endtoend_delay_alternatively(**kwargs):
    # This validation test compares simulation results to analytical results and also
    # to results from a different simulation using Core4INET.
    # https://github.com/CoRE-RG/CoRE4INET/tree/master/examples/tsn/medium_network
    df = pd.DataFrame(index=["Medium", "High", "Critical"],
                      data={"min": [88.16, 60.8, 252.8],
                            "max": [540, 307.2, 375.84],
                            "mean": [247.1, 161.19, 298.52],
                            "stddev": [106.53, 73.621, 36.633]})
    df["min"] -= 0.001 # 1 ns initial production offset, see INI file
    df = df + 0.05 * 2 # 50 ns propagation delay per hop
    df.index.set_names(["trafficclass"], inplace=True)
    df.columns.set_names(["name"], inplace=True)
    return df

def compute_tsn_trafficshaping_creditbasedshaper_validation_test_results(test_accuracy=0.01, **kwargs):
    df1 = compute_creditbasedshaper_endtoend_delay_from_simulation_results(**kwargs)
    df2 = compute_creditbasedshaper_endtoend_delay_alternatively(**kwargs)
    df1 = df1.sort_index(axis=0).sort_index(axis=1)
    df2 = df2.sort_index(axis=0).sort_index(axis=1)
    maxQueueLength = compute_creditbasedshaper_max_queuelength_from_simulation_results()
    test_result = maxQueueLength < 4 and \
                  (df1["min"] >= df2["min"]).all() and \
                  (df1["max"] <= df2["max"]).all() and \
                  numpy.allclose(df1["min"], df2["min"], rtol=test_accuracy, atol=0) and \
                  numpy.allclose(df1["max"], df2["max"], rtol=test_accuracy, atol=0) and \
                  numpy.allclose(df1["mean"], df2["mean"], rtol=test_accuracy * 7, atol=0) and \
                  numpy.allclose(df1["stddev"], df2["stddev"], rtol=test_accuracy * 30, atol=0)
    return TestTaskResult(task=TestTask(), bool_result=test_result)

def get_tsn_trafficshaping_creditbasedshaper_validation_test_task(**kwargs):
    simulation_task = get_simulation_tasks(inet_project, working_directory_filter="tests/validation/tsn/trafficshaping/creditbasedshaper", sim_time_limit="1s", **kwargs).tasks[0]
    return ValidationTestTask(simulation_task, compute_tsn_trafficshaping_creditbasedshaper_validation_test_results, **kwargs)

def run_tsn_trafficshaping_creditbasedshaper_validation_test(**kwargs):
    return get_tsn_trafficshaping_creditbasedshaper_validation_test_task(**kwargs).run(**kwargs)

###########################
# Multiple validation tests

def get_validation_test_tasks(**kwargs):
    validation_test_task_functions = [get_tsn_framereplication_simulation_test_task,
                                      get_tsn_trafficshaping_creditbasedshaper_validation_test_task,
                                      get_tsn_trafficshaping_asynchronousshaper_core4inet_validation_test_task,
                                      get_tsn_trafficshaping_asynchronousshaper_icct_simulation_test_task]
    validation_test_tasks = []
    for validation_test_task_function in validation_test_task_functions:
        validation_test_task_function_kwargs = kwargs.copy()
        for key in ["filter", "working_directory_filter", "ini_file_filter", "config_filter", "run_filter"]:
            validation_test_task_function_kwargs.pop(key, None)
        validation_test_task = validation_test_task_function(**validation_test_task_function_kwargs)
        simulation_config = validation_test_task.simulation_task.simulation_config
        if simulation_config.matches_filter(**kwargs):
            validation_test_tasks.append(validation_test_task)
    return MultipleTestTasks(tasks=validation_test_tasks, name="validation test", **kwargs)

def run_validation_tests(**kwargs):
    return get_validation_test_tasks(**kwargs).run(**kwargs)
