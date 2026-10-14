#!/usr/bin/env python3
"""Check Basic exchanges in the retained demonstrations with a module-test observer."""

import argparse
import json
from pathlib import Path
import re
import subprocess


CASES = [
    ("showcases/wireless/blockack", "omnetpp.ini", "General", 0, "50ms", False, 1),
    ("showcases/wireless/blockack", "omnetpp.ini", "NoFragmentation", 0, "50ms", False, 1),
    ("showcases/wireless/blockack", "omnetpp.ini", "MixedTraffic", 0, "50ms", False, 1),
    ("showcases/wireless/blockack", "omnetpp.ini", "Fragmentation", 0, "50ms", True, 1),
    ("showcases/wireless/txop", "omnetpp.ini", "General", 0, "50ms", False, 1),
    ("examples/wireless/qos", "omnetpp.ini", "MacQosWithBlockAck", 0, "1.2s", False, 2),
    ("examples/adhoc/qos", "omnetpp.ini", "MacQos", 0, "0.8s", False, 0),
    ("examples/adhoc/qos", "omnetpp.ini", "MacQos", 1, "0.8s", False, 1),
    ("showcases/wireless/fragmentation", "omnetpp.ini", "HCFfragblockack", 0, "100ms", True, 1),
    ("showcases/wireless/fragmentation", "parameterstudy.ini", "PacketSizeHCFfragblockack", 8, "100ms", True, 1),
    ("showcases/wireless/fragmentation", "parameterstudy.ini", "TxPowerHCFfragblockack", 12, "100ms", True, 1),
    ("showcases/wireless/fragmentation", "parameterstudy.ini", "ThresholdHCFfragblockack", 0, "100ms", True, 1),
    ("showcases/wireless/fragmentation", "parameterstudy.ini", "OnlyPacketSizeHCFfragblockack", 8, "100ms", True, 1),
]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--compare", type=Path)
    parser.add_argument("--protocol-inventory", action="store_true", help="Observe HT state without the protocol assertions; run those assertions separately.")
    parser.add_argument("--case-filter", default=".", help="Select demonstration configurations or protocol fixture names with a regular expression.")
    args = parser.parse_args()
    root = args.root.resolve()
    work = root / "tests/module/work/Ieee80211BasicCompatibility_1"
    binary = work / "Ieee80211BasicCompatibility_1_dbg"
    args.output.mkdir(parents=True, exist_ok=False)
    ned_path = ";".join(str(path) for path in [root / "src", root / "examples", root / "showcases", work, root / "tests/module/lib"])
    cases = CASES
    if args.protocol_inventory:
        ned_path += ";" + ";".join(str(root / path) for path in ["tests/protocol/lib", "tests/protocol/wifi/ned"])
        cases = [(f"tests/protocol/wifi/work/{name}", "test.ini", "General", 0, "1.2s", False, None)
                 for name in ["N_BlockAck", "N_CompressedBa", "N_Amsdu", "N_Ampdu", "Ac_BlockAck", "Ac_Ampdu", "Ac_MultiTidBa", "WifiQosDelayedBa"]]
    results = []
    cases = [case for case in cases if re.search(args.case_filter, case[0] + "/" + case[2])]
    if not cases:
        parser.error("The case filter selects no cases.")
    for index, (directory, ini, config, run, limit, fragments, hop_count) in enumerate(cases):
        command = [
            str(binary), "-u", "Cmdenv", "-n", ned_path, "-f", ini, "-c", config, "-r", str(run),
            "--seed-set=0", f"--sim-time-limit={limit}", "--cmdenv-express-mode=true",
            "--record-vector-results=false", "--record-scalar-results=false", "--check-signals=false",
            '--**.wlan[*].mac.typename="BasicCompatibilityMac"',
            f"--result-dir={args.output.resolve() / ('results-' + str(index))}",
        ]
        if args.protocol_inventory:
            command += ["-l", str(root / "tests/protocol/lib/libprotocoltest_dbg.so"),
                        '--*.tester.testName=""', '--*.tester.logEvents=false']
        result = subprocess.run(command, cwd=root / directory, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        log = args.output / f"{index:02d}-{config}-r{run}.log"
        log.write_text(result.stdout)
        hops = []
        for line in result.stdout.splitlines():
            if "BASIC-HOP " in line:
                text = line.split("BASIC-HOP ", 1)[1]
                module, fields = text.split(" ", 1)
                hop = dict(re.findall(r"(\w+)=(\S+)", fields))
                hop["module"] = module
                for field in ("tid", "localHt", "peerHt", "addba", "data", "fragments", "bar", "positive", "fragmentBits"):
                    hop[field] = int(hop[field])
                hops.append(hop)
        active = [hop for hop in hops if hop["data"] > 0]
        complete = [hop for hop in active if hop["addba"] > 0 and hop["bar"] > 0 and hop["positive"] > 0]
        valid = True
        links = {(hop["module"], hop["peer"]) for hop in complete}
        peers = [dict(re.findall(r"(\w+)=(\S+)", line)) for line in result.stdout.splitlines() if "BASIC-PEER " in line]
        if fragments:
            valid &= any(hop["fragments"] > 0 and hop["fragmentBits"] > 0 for hop in complete)
        normal_ack = sum(int(value) for value in re.findall(r"BASIC-CONTROL .* normalAckData=(\d+)", result.stdout))
        requests = sum(int(value) for value in re.findall(r"BASIC-CONTROL .* addbaRequests=(\d+)", result.stdout))
        responses = sum(int(value) for value in re.findall(r"BASIC-CONTROL .* blockAckResponses=(\d+)", result.stdout))
        if hop_count is None:
            valid = bool(peers) and all(peer["localHt"] == "1" and peer["peerHt"] == "1" for peer in peers)
        elif hop_count == 0:
            valid &= not hops and normal_ack > 0 and requests == 0 and responses == 0
        else:
            valid &= len(links) >= hop_count
        record = dict(directory=directory, ini=ini, config=config, run=run, seed=0, limit=limit,
                      command=command, cwd=str(root / directory), exit_code=result.returncode,
                      hops=hops, normal_ack_data=normal_ack, addba_requests=requests, block_ack_responses=responses,
                      ht_peers=peers, registered_protocol_test_enabled=False,
                      check="HT state" if args.protocol_inventory else "Basic exchange",
                      incomplete_tids=[hop for hop in active if hop not in complete],
                      passed=result.returncode == 0 and valid, log=str(log.resolve()))
        results.append(record)
        label = Path(directory).name if args.protocol_inventory else config
        print(f"{label} run {run}: {'PASS' if record['passed'] else 'FAIL'}; {len(links)} Basic links", flush=True)
    comparison = None
    if args.compare:
        parent = json.loads(args.compare.read_text())["results"]
        comparison = len(parent) == len(results) and all(
            all(old[key] == new[key] for key in ("directory", "ini", "config", "run", "seed", "limit", "exit_code", "hops", "normal_ack_data", "addba_requests", "block_ack_responses", "passed"))
            for old, new in zip(parent, results)
        )
        print(f"Parent exchange comparison: {'PASS' if comparison else 'FAIL'}", flush=True)
    (args.output / "evidence.json").write_text(json.dumps(dict(results=results, parent_equal=comparison), indent=2) + "\n")
    return 0 if all(record["passed"] for record in results) and comparison is not False else 1


if __name__ == "__main__":
    raise SystemExit(main())
