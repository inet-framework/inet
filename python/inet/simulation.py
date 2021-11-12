import csv
import functools
import glob
import multiprocessing
import omnetpp
import omnetpp.scave.analysis
import os
import re
import shutil
import subprocess

def get_full_path(resource):
    return os.environ['INET_ROOT'] + "/" + resource

def parse_simulations(csv_file):
    def commentRemover(csv_data):
        p = re.compile(' *#.*$')
        for line in csv_data:
            yield p.sub('',line.decode('utf-8'))
    simulations = []
    f = open(csv_file, 'rb')
    csvReader = csv.reader(commentRemover(f), delimiter=str(','), quotechar=str('"'), skipinitialspace=True)
    for fields in csvReader:
        if len(fields) == 0:
            pass        # empty line
        elif len(fields) == 6:
            if fields[4] in ['PASS', 'FAIL', 'ERROR']:
                simulations.append({
                        'file': csv_file,
                        'line' : csvReader.line_num,
                        'wd': fields[0],
                        'args': fields[1],
                        'simtimelimit': fields[2],
                        'fingerprint': fields[3],
                        'expectedResult': fields[4],
                        'tags': fields[5]
                        })
            else:
                raise Exception(csv_file + " Line " + str(csvReader.line_num) + ": the 5th item must contain one of 'PASS', 'FAIL', 'ERROR'" + ": " + '"' + '", "'.join(fields) + '"')
        else:
            raise Exception(csv_file + " Line " + str(csvReader.line_num) + " must contain 6 items, but contains " + str(len(fields)) + ": " + '"' + '", "'.join(fields) + '"')
    f.close()
    return simulations

def read_simulations(csv_file):
    return parse_simulations(csv_file)

def read_examples():
    return read_simulations(get_full_path("tests/fingerprint/examples.csv"))

def read_showcases():
    return read_simulations(get_full_path("tests/fingerprint/showcases.csv"))

def read_tutorials():
    return read_simulations(get_full_path("tests/fingerprint/tutorials.csv"))

examples = read_examples()
showcases = read_showcases()
tutorials = read_tutorials()
simulations = examples + showcases + tutorials

def get_simulations(simulations = simulations, path_filter = ".*", fullMatch = False):
    return filter(lambda simulation: re.search(path_filter if fullMatch else ".*" + path_filter + ".*", simulation['wd']), simulations)

def clean_simulation_results(simulation):
    print("Cleaning simulation results, folder = " + simulation['wd'])
    path = get_full_path(simulation['wd']) + "results"
    if not re.search(".*home.*", path):
        raise Exception("Path is not in home")
    if os.path.exists(path):
        shutil.rmtree(path)

def clean_simulations_results(simulations = None, **kwargs):
    if not simulations:
        simulations = get_simulations(**kwargs)
    for simulation in simulations:
        clean_simulation_results(simulation)

def run_simulation(simulation, sim_time_limit = None, ui = "Cmdenv", mode = "debug", **kwargs):
    if sim_time_limit is None:
        sim_time_limit = simulation['simtimelimit']
    print("Running simulation, folder = " + simulation['wd'] + ", arguments = " + simulation['args'])
    return subprocess.run(["inet", "--" + mode, "-s", "-u", ui, *simulation['args'].split(), "--sim-time-limit", sim_time_limit, "--cmdenv-redirect-output", "true"], cwd = "/home/levy/workspace/inet" + simulation['wd'])

def run_simulations(simulations = None, **kwargs):
    if not simulations:
        simulations = get_simulations(**kwargs)
    pool = multiprocessing.Pool(multiprocessing.cpu_count())
    partial = functools.partial(run_simulation, **kwargs)
    pool.map(partial, simulations)

def get_analysis_files(path_filter = ".*", fullMatch = False):
    analysisFiles = glob.glob("examples/**/*.anf", recursive = True) + \
                    glob.glob("showcases/**/*.anf", recursive = True) + \
                    glob.glob("tutorials/**/*.anf", recursive = True)
    return filter(lambda path: re.search(path_filter if fullMatch else ".*" + path_filter + ".*", path), analysisFiles)

def export_charts(**kwargs):
    for analysisFile in get_analysis_files(**kwargs):
        print("Exporting charts, analysisFile = " + analysisFile)
        analysis = omnetpp.scave.analysis.load_anf_file(analysisFile)
        for chart in analysis.charts:
            folder = os.path.dirname(analysisFile)
            analysis.export_image(chart, get_full_path(folder), workspace, format="png", dpi=150, target_folder="doc/media")

def generate_charts(**kwargs):
    clean_simulations_results(**kwargs)
    run_simulations(**kwargs)
    export_charts(**kwargs)

workspace = omnetpp.scave.analysis.Workspace(omnetpp.scave.analysis.Workspace.find_workspace(), [])
