#!/usr/bin/env python3
# Generates doc/media/downlink-throughput.png: per-station application throughput on the
# downlink, frame-fair (anomaly) vs airtime-fair AP transmit queue. sta[0] is a far, rate-
# adapted client (~6 Mbps); the other four are close (54 Mbps). Both configs are averaged
# over 3 repetitions (DCF backoff uses the RNG, so both are averaged the same way).
#
# Reproduce (from this showcase directory):
#   inet -u Cmdenv -c DownlinkAnomaly     -r 0..2 --repeat=3 --result-dir=results/dl
#   inet -u Cmdenv -c DownlinkAirtimeFair -r 0..2 --repeat=3 --result-dir=results/dl
#   python3 dl-chart.py
import re, glob
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

def per_station_mean(config):
    reps = {}
    for path in glob.glob(f"results/dl/{config}-*.sca"):
        for line in open(path):
            m = re.search(r'sta\[(\d)\]\.app\[0\]\s+packetReceived:count\s+([\d.eE+]+)', line)
            if m:
                reps.setdefault(int(m.group(1)), []).append(float(m.group(2)) * 0.002)  # frames x 1000 B x 8 / 4 s
    return [sum(reps.get(i, [0])) / len(reps.get(i, [1])) for i in range(5)]

anomaly = per_station_mean("DownlinkAnomaly")
fair = per_station_mean("DownlinkAirtimeFair")

x = np.arange(5)
w = 0.38
plt.figure(figsize=(8, 6))
ax = plt.gca()
b1 = ax.bar(x - w/2, anomaly, w, color="#c0392b", label=f"Frame-fair queue (anomaly, agg {sum(anomaly):.1f} Mbps)")
b2 = ax.bar(x + w/2, fair,    w, color="#1f618d", label=f"Airtime-fair queue (fix, agg {sum(fair):.1f} Mbps)")
ax.bar_label(b1, fmt="%.1f", padding=2, fontsize=8)
ax.bar_label(b2, fmt="%.1f", padding=2, fontsize=8)
ax.set_xticks(x)
ax.set_xticklabels(["sta[0]\n(far, ~6 Mbps)", "sta[1]", "sta[2]", "sta[3]", "sta[4]"])
ax.set_ylabel("Application throughput [Mbps]")
ax.set_title("Downlink: airtime fairness lets the fast clients recover")
ax.grid(True, axis="y", linestyle="--", alpha=0.6)
ax.set_axisbelow(True)
ax.set_ylim(0, 6.5)
ax.legend(loc="upper left", framealpha=0.9, fontsize=9)
plt.savefig("doc/media/downlink-throughput.png", dpi=150, bbox_inches="tight")
print("saved doc/media/downlink-throughput.png  anomaly=%s  fair=%s" % (
    [round(v, 2) for v in anomaly], [round(v, 2) for v in fair]))
