#!/usr/bin/env python3
# Generates doc/media/txop-throughput.png: uplink application throughput under plain DCF (the
# rate anomaly) vs 802.11e airtime-based TXOP, at a single 6-versus-54 Mbps rate gap. Both
# configs are averaged over 3 repetitions -- DCF is not deterministic (the backoff draws from
# the RNG), so both are averaged the same way.
#
# Plotted as the fast-station GROUP average and the slow station, with the aggregates carried
# in the legend -- deliberately NOT per station. Under permanent saturation INET's EDCA
# sometimes locks one station out entirely (in these runs UplinkTxop rep #1 leaves sta[1] at
# 0 Mbps while its neighbours absorb the share), so a per-station plot would be dominated by
# that run-to-run artifact rather than by the effect being shown. The downlink counterpart
# (dl-chart.py) DOES plot per station, because a single transmitter running a deterministic
# scheduler has no contention lockout to average away.
#
# The dashed line is the all-fast reference: UplinkHomogeneous, the same cell with no rate gap.
#
# Reproduce (from this showcase directory):
#   inet -u Cmdenv -c UplinkHomogeneous -r 0..2 --repeat=3 --result-dir=results/solve
#   inet -u Cmdenv -c UplinkAnomaly     -r 0..2 --repeat=3 --result-dir=results/solve
#   inet -u Cmdenv -c UplinkTxop        -r 0..2 --repeat=3 --result-dir=results/solve
#   python3 ul-chart.py
#
# The comparison results live in results/solve/ (not results/) so they do not contaminate the
# .anf bar charts, whose filters match server.app packetReceived:count of any config.
import re, glob
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

def runs(config):
    """One [sta0..sta4] throughput vector per repetition, in Mbps."""
    out = []
    for path in sorted(glob.glob(f"results/solve/{config}-#*.sca")):
        v = {}
        for line in open(path):
            m = re.search(r'server\.app\[(\d)\]\s+packetReceived:count\s+([\d.eE+]+)', line)
            if m:
                v[int(m.group(1))] = float(m.group(2)) * 0.002  # frames x 1000 B x 8 / 4 s
        out.append([v.get(i, 0) for i in range(5)])
    return out

def summarise(config):
    r = runs(config)
    n = len(r)
    return (sum(x[0] for x in r) / n,                 # slow station
            sum(sum(x[1:]) / 4 for x in r) / n,       # fast-station average
            sum(sum(x) for x in r) / n)               # aggregate

dcf_slow, dcf_fast, dcf_agg = summarise("UplinkAnomaly")
tx_slow, tx_fast, tx_agg = summarise("UplinkTxop")
base_slow, base_fast, base_agg = summarise("UplinkHomogeneous")
baseline = base_agg / 5   # all-fast, per-station

x = np.arange(2)
w = 0.38
plt.figure(figsize=(8, 6))
ax = plt.gca()
ax.axhline(baseline, color="0.35", ls="--", lw=1.4, zorder=3,
           label=f"All-fast baseline ({baseline:.1f} Mbps/station, agg {base_agg:.1f})")
b1 = ax.bar(x - w/2, [dcf_fast, dcf_slow], w, color="#c0392b",
            label=f"Plain DCF (anomaly, agg {dcf_agg:.1f} Mbps)")
b2 = ax.bar(x + w/2, [tx_fast, tx_slow], w, color="#1f618d",
            label=f"802.11e TXOP (fix, agg {tx_agg:.1f} Mbps)")
ax.bar_label(b1, fmt="%.1f", padding=2, fontsize=9)
ax.bar_label(b2, fmt="%.1f", padding=2, fontsize=9)
ax.set_xticks(x)
ax.set_xticklabels(["Fast stations\n(avg of the four at 54 Mbps)", "Slow station\nsta[0] (6 Mbps)"])
ax.set_ylabel("Application throughput [Mbps]")
ax.set_title("Uplink: an airtime-based TXOP lets the fast stations recover")
ax.grid(True, axis="y", linestyle="--", alpha=0.6)
ax.set_axisbelow(True)
ax.set_ylim(0, 7)
ax.legend(loc="upper right", framealpha=0.9, fontsize=9)
plt.tight_layout()
plt.savefig("doc/media/txop-throughput.png", dpi=150, transparent=True)
print("saved doc/media/txop-throughput.png")
print(f"  DCF   slow={dcf_slow:.2f} fast={dcf_fast:.2f} agg={dcf_agg:.1f}")
print(f"  TXOP  slow={tx_slow:.2f} fast={tx_fast:.2f} agg={tx_agg:.1f}")
print(f"  base  per-station={baseline:.2f} agg={base_agg:.1f}")
