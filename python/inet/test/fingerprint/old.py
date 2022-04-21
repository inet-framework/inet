import argparse
import csv
import glob
import logging
import re
import shlex

from inet.common import *
from inet.simulation.project import *
from inet.test.fingerprint.task import *

logger = logging.getLogger(__name__)

def update_correct_fingerprints_from_csv(csv_file, correct_fingerprints, filter=None, exclude_filter=None, full_match=False):
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
            if not matches_filter(str(fields), filter, exclude_filter, full_match):
                continue
            elif fields[4] in ["PASS", "FAIL", "ERROR"]:
                parser = argparse.ArgumentParser()
                parser.add_argument("-f", "--ini_file", default="omnetpp.ini")
                parser.add_argument("-c", "--config", default="General")
                parser.add_argument("-r", "--run", default="0")
                args = parser.parse_args(shlex.split(fields[1]))
                fingerprints = map(lambda text: Fingerprint(text), fields[3].split(";"))
                working_directory = fields[0] if fields[0] != "." else "tests/fingerprint"
                if working_directory[0] == '/':
                    working_directory = working_directory[1:]
                if working_directory[-1] == '/':
                    working_directory = working_directory[:-1]
                logger.info("Updating correct fingerprint: " + working_directory + " -f " + args.ini_file + " -c " + args.config + " -r " + args.run)
                for fingerprint in fingerprints:
                    try:
                        correct_fingerprints.update_fingerprint(fingerprint.fingerprint,
                                                                working_directory=working_directory,
                                                                ini_file=args.ini_file,
                                                                config=args.config,
                                                                run=int(args.run),
                                                                sim_time_limit=fields[2],
                                                                ingredients=fingerprint.ingredients,
                                                                test_result=fields[4])
                    except Exception as e:
                        logger.warn("Failed to update correct fingerprint: " + str(e))
            else:
                raise Exception(csv_file + " Line " + str(csvReader.line_num) + ": the 5th item must contain one of 'PASS', 'FAIL', 'ERROR'" + ": " + '"' + '", "'.join(fields) + '"')
        else:
            raise Exception(csv_file + " Line " + str(csvReader.line_num) + " must contain 6 items, but contains " + str(len(fields)) + ": " + '"' + '", "'.join(fields) + '"')
    f.close()
    return entries

def update_correct_fingerprints_from_csvs(**kwargs):
    correct_fingerprints = get_correct_fingerprint_store(inet_project)
    for csv_file in glob.glob(inet_project.get_full_path("tests/fingerprint/*.csv"), recursive=True):
        update_correct_fingerprints_from_csv(csv_file, correct_fingerprints, **kwargs)
    correct_fingerprints.write()
