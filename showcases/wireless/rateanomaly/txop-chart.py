#!/usr/bin/env python3
# Generates doc/media/txop-throughput-vs-rate.png: aggregate and fast-station-average
# application throughput vs the slow station's bitrate, plain DCF vs 802.11e TXOP.
# BOTH configs are averaged over 3 repetitions -- DCF is not deterministic (same RNG/backoff),
# so it is averaged the same way as TXOP.
#
# Reproduce (from this showcase directory):
#   inet -u Cmdenv -c RateAnomaly  -r 0..17 --repeat=3 --result-dir=results/solve
#   inet -u Cmdenv -c Txop         -r 0..17 --repeat=3 --result-dir=results/solve
#   inet -u Cmdenv -c Homogeneous  -r 0               --result-dir=results
#   python3 txop-chart.py
#
# The sweep results live in results/solve/ (not results/) so they do not contaminate the
# DCF-only .anf charts, whose filters match server.app packetReceived:count of any config.
import re, glob
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

def per_station(path):
    v = {}
    for line in open(path):
        m = re.search(r'server\.app\[(\d)\]\s+packetReceived:count\s+([\d.eE+]+)', line)
        if m:
            v[int(m.group(1))] = float(m.group(2)) * 0.002  # frames x 1000 B x 8 / 4 s -> Mbps
    return [v.get(i, 0) for i in range(5)]

def series(config, rate, which):
    reps = [per_station(f) for f in glob.glob(f"results/solve/{config}-slowBitrate={rate}-#*.sca")]
    vals = [sum(x) for x in reps] if which == "agg" else [sum(x[1:]) / 4 for x in reps]
    return sum(vals) / len(vals)   # mean over the repetitions

rates = [6, 9, 12, 18, 24, 36]
dcf_agg  = [series("RateAnomaly", r, "agg")  for r in rates]
dcf_fast = [series("RateAnomaly", r, "fast") for r in rates]
tx_agg   = [series("Txop", r, "agg")  for r in rates]
tx_fast  = [series("Txop", r, "fast") for r in rates]
baseline = sum(per_station(sorted(glob.glob("results/Homogeneous-*.sca"))[0]))  # all-fast aggregate

plt.figure(figsize=(8, 6))
plt.axhline(baseline, color="gray", ls=":", lw=1.4, label=f"All-fast baseline (~{baseline:.0f} Mbps)")
plt.plot(rates, dcf_agg,  "o-",  color="#c0392b", lw=2,   label="Aggregate — plain DCF")
plt.plot(rates, tx_agg,   "s-",  color="#1f618d", lw=2,   label="Aggregate — 802.11e TXOP")
plt.plot(rates, dcf_fast, "o--", color="#e59866", lw=1.6, label="Fast-station avg — plain DCF")
plt.plot(rates, tx_fast,  "s--", color="#5dade2", lw=1.6, label="Fast-station avg — 802.11e TXOP")
plt.xlabel("Slow station's bitrate [Mbps]")
plt.ylabel("Application throughput [Mbps]")
plt.title("Airtime-based TXOP prevents the rate-anomaly collapse")
plt.xticks(rates)
plt.ylim(0, 30)
plt.grid(True, linestyle="--", alpha=0.6)
plt.gca().set_axisbelow(True)
plt.legend(loc="center right", framealpha=0.9, fontsize=9)
plt.savefig("doc/media/txop-throughput-vs-rate.png", dpi=150, bbox_inches="tight")
print("saved doc/media/txop-throughput-vs-rate.png")
