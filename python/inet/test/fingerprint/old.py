import argparse
import csv
import glob
import re
import shlex

from inet.common import *
from inet.simulation.project import *
from inet.test.fingerprint.task import *

def update_correct_fingerprints_from_csv(csv_file):
    def commentRemover(csv_data):
        p = re.compile(" *#.*$")
        for line in csv_data:
            yield p.sub("",line.decode("utf-8"))
    entries = []
    f = open(csv_file, "rb")
    csvReader = csv.reader(commentRemover(f), delimiter=str(","), quotechar=str("'"), skipinitialspace=True)
    for fields in csvReader:
        if len(fields) == 0:
            pass        # empty line
        elif len(fields) == 6:
            if fields[4] in ["PASS", "FAIL", "ERROR"]:
                parser = argparse.ArgumentParser()
                parser.add_argument("-f", "--ini_file", default="omnetpp.ini")
                parser.add_argument("-c", "--config", default="General")
                parser.add_argument("-r", "--run", default="0")
                args = parser.parse_args(shlex.split(fields[1]))
                print("-f " + args.ini_file + " -c " + args.config + " -r " + args.run)
                fingerprints = map(lambda text: Fingerprint(text), fields[3].split(";"))
                for fingerprint in fingerprints:
                    correct_fingerprints.update_fingerprint(fingerprint.fingerprint,
                                                            working_directory=fields[0][1:-1] if fields[0] != "." else "tests/fingerprint",
                                                            ini_file=args.ini_file,
                                                            config=args.config,
                                                            run=args.run,
                                                            sim_time_limit=fields[2],
                                                            ingredients=fingerprint.ingredients,
                                                            test_result=fields[4])
            else:
                raise Exception(csv_file + " Line " + str(csvReader.line_num) + ": the 5th item must contain one of 'PASS', 'FAIL', 'ERROR'" + ": " + '"' + '", "'.join(fields) + '"')
        else:
            raise Exception(csv_file + " Line " + str(csvReader.line_num) + " must contain 6 items, but contains " + str(len(fields)) + ": " + '"' + '", "'.join(fields) + '"')
    f.close()
    return entries

def update_correct_fingerprints_from_csvs():
    list(map(update_correct_fingerprints_from_csv, glob.glob(inet_project.get_full_path("tests/fingerprint/*.csv"), recursive=True)))
    correct_fingerprints.write()
