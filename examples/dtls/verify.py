#!/usr/bin/env python3
"""Run fixed-seed DTLS demonstrations and assert endpoint/callback outcomes."""

import argparse
import csv
import json
from pathlib import Path
import re
import subprocess


CONFIGS = (
    "Basic", "Certificate", "Fragmentation", "LossyHandshake", "ProcessingCost", "Ipv6",
    "Reconnect", "EarlyData", "Migration", "DeferredAuthentication", "AlgorithmCost",
    "PathMtu", "Legacy", "MixedVersions", "LegacyReconnect", "LegacyMigration",
)


def verify(mode, output, configs):
    here = Path(__file__).resolve().parent
    output.mkdir(parents=True, exist_ok=False)
    report = {}
    for config in configs:
        run_dir = output / config
        run_dir.mkdir()
        command = ["inet", f"--{mode}"] + [
            "-u", "Cmdenv", "-c", config, "-r", "0", "--seed-set=0",
            "--cmdenv-express-mode=false", f"--result-dir={run_dir}",
        ]
        (run_dir / "command.json").write_text(json.dumps(command) + "\n")
        with (run_dir / "simulation.log").open("w") as log:
            subprocess.run(command, cwd=here, stdout=log, stderr=subprocess.STDOUT, check=True)
        inputs = list(run_dir.glob("*.sca"))
        assert len(inputs) == 1, (config, "one scalar run required", inputs)
        export = run_dir / "scalars.csv"
        with (run_dir / "export.log").open("w") as log:
            subprocess.run([
                "opp_scavetool", "export", "-F", "CSV-R", "-o", str(export), "-f",
                'module =~ "**.app[0].dtls" AND type =~ scalar', str(inputs[0]),
            ], stdout=log, stderr=subprocess.STDOUT, check=True)
        with export.open() as stream:
            rows = list(csv.DictReader(stream))
        assert {r["attrvalue"] for r in rows if r["type"] == "runattr" and r["attrname"] == "configname"} == {config}
        assert {r["attrvalue"] for r in rows if r["type"] == "runattr" and r["attrname"] == "seedset"} == {"0"}
        scalars = {(r["module"], r["name"]): float(r["value"]) for r in rows if r["type"] == "scalar"}

        def value(endpoint, name):
            matches = [v for (module, result), v in scalars.items() if module.endswith(f".{endpoint}.app[0].dtls") and result == name]
            assert len(matches) == 1, (config, endpoint, name, matches)
            return matches[0]

        counts = {}
        for endpoint in ("client", "server"):
            expected = 2 if config in ("Reconnect", "EarlyData", "LegacyReconnect") else 1
            assert value(endpoint, "associationEstablished:count") == expected, (config, endpoint, "establishment count")
            assert value(endpoint, "associationFailed:count") == 0, (config, endpoint, "association failure")
            counts[endpoint] = {name: value(endpoint, name) for name in (
                "associationEstablished:count", "associationFailed:count", "packetReceived:count",
                "recordOverhead:mean", "processingDelay:mean",
            )}
        assert value("server", "packetReceived:count") > 0, (config, "no application delivery")
        text = (run_dir / "simulation.log").read_text()
        if config in ("Reconnect", "EarlyData", "LegacyReconnect"):
            assert text.count("established, resumed=1") == 2, (config, "both endpoints must resume")
        if config == "EarlyData":
            assert text.count("early data accepted") == 1, "explicit early admission"
        if config in ("Migration", "LegacyMigration"):
            assert len(re.findall(r"DTLS association \d+ path .*44001", text)) == 2, (config, "both paths must validate")
        if config == "DeferredAuthentication":
            assert text.count("authentication succeeded:") == 1, "deferred certificate result"
        if config == "AlgorithmCost":
            assert value("client", "processingDelay:mean") > 0, "selected algorithm service cost"
        if config == "PathMtu":
            assert "maximum application datagram: 770 bytes" in text, "confirmed routed budget"
        if config in ("Legacy", "MixedVersions", "LegacyReconnect"):
            assert value("client", "recordOverhead:mean") == 37, (config, "legacy record selection")
        if config == "LegacyMigration":
            assert value("client", "recordOverhead:mean") == 46, "legacy CID expansion"
        report[config] = counts
        print(f"{mode} {config}: PASS", flush=True)
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("debug", "release"), required=True)
    parser.add_argument("--output", type=Path, required=True, help="new directory outside the example")
    parser.add_argument("--configs", nargs="+", choices=CONFIGS, default=CONFIGS)
    args = parser.parse_args()
    verify(args.mode, args.output.resolve(), args.configs)
