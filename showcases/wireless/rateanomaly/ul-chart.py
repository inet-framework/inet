#!/usr/bin/env python3
# Generates doc/media/txop-throughput.png: uplink application throughput under plain DCF (the
# rate anomaly) vs 802.11e airtime-based TXOP, at a single 6-versus-54 Mbps rate gap. Both
# configs are averaged over 10 repetitions -- DCF is not deterministic (the backoff draws from
# the RNG), so both are averaged the same way. Ten rather than three because the bars carry
# error bars, and three points do not estimate a spread.
#
# Plotted as the fast-station GROUP average and the slow station, with the aggregates carried
# in the legend -- deliberately NOT per station. EDCA allocates wins at random and only bounds
# what a win is worth, so over ten seeds the four fast stations scatter between 4.1 and 6.8 Mbps
# around a 5.6 Mbps mean; a per-station plot would show that scatter rather than the effect
# being demonstrated. The downlink counterpart (dl-chart.py) DOES plot per station, because a
# single transmitter running a deterministic scheduler has no contention scatter to average away.
#
# The dashed line is the all-fast reference: UplinkHomogeneous, the same cell with no rate gap.
#
# Reproduce (from this showcase directory):
#   inet -u Cmdenv -c UplinkHomogeneous -r 0..9 --repeat=10 --vector-recording=false --result-dir=results/solve
#   inet -u Cmdenv -c UplinkAnomaly     -r 0..9 --repeat=10 --vector-recording=false --result-dir=results/solve
#   inet -u Cmdenv -c UplinkTxop        -r 0..9 --repeat=10 --vector-recording=false --result-dir=results/solve
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
    """Slow station and fast-station group, each as (mean, sd), plus the mean aggregate.

    The error bars are the spread of the INDIVIDUAL station values -- every sta[1..4] reading
    from every repetition for the fast group, sta[0]'s ten readings for the slow one -- not the
    uncertainty of the mean. They answer "how differently do two stations fare in one run",
    which is what the page's caveat claims.
    """
    r = runs(config)
    n = len(r)
    slow = [x[0] for x in r]
    fast = [v for x in r for v in x[1:]]
    return (mean_sd(slow), mean_sd(fast), sum(sum(x) for x in r) / n)

def mean_sd(v):
    m = sum(v) / len(v)
    return m, (sum((x - m) ** 2 for x in v) / (len(v) - 1)) ** 0.5

(dcf_slow, dcf_slow_sd), (dcf_fast, dcf_fast_sd), dcf_agg = summarise("UplinkAnomaly")
(tx_slow, tx_slow_sd), (tx_fast, tx_fast_sd), tx_agg = summarise("UplinkTxop")
(base_slow, _), (base_fast, _), base_agg = summarise("UplinkHomogeneous")
baseline = base_agg / 5   # all-fast, per-station

x = np.arange(2)
w = 0.38
plt.figure(figsize=(8, 6))
ax = plt.gca()
ax.axhline(baseline, color="0.35", ls="--", lw=1.4, zorder=3,
           label=f"All-fast baseline ({baseline:.1f} Mbps/station, agg {base_agg:.1f})")
ebar = dict(capsize=4, ecolor="0.25", error_kw={"lw": 1.2, "zorder": 4})
b1 = ax.bar(x - w/2, [dcf_fast, dcf_slow], w, color="#c0392b",
            yerr=[dcf_fast_sd, dcf_slow_sd], **ebar,
            label=f"Plain DCF (anomaly, agg {dcf_agg:.1f} Mbps)")
b2 = ax.bar(x + w/2, [tx_fast, tx_slow], w, color="#1f618d",
            yerr=[tx_fast_sd, tx_slow_sd], **ebar,
            label=f"802.11e TXOP (fix, agg {tx_agg:.1f} Mbps)")
ax.bar_label(b1, fmt="%.1f", padding=2, fontsize=9)
ax.bar_label(b2, fmt="%.1f", padding=2, fontsize=9)
ax.set_xticks(x)
ax.set_xticklabels(["Fast stations\n(avg of the four at 54 Mbps)", "Slow station\nsta[0] (6 Mbps)"])
ax.set_ylabel("Application throughput [Mbps]")
ax.set_title("Uplink: an airtime-based TXOP lets the fast stations recover")
ax.grid(True, axis="y", linestyle="--", alpha=0.6)
ax.set_axisbelow(True)
ax.set_ylim(0, 8)
ax.legend(loc="upper right", framealpha=0.9, fontsize=9,
          title="error bars: \u00b11 s.d. of individual stations", title_fontsize=8)
plt.tight_layout()
plt.savefig("doc/media/txop-throughput.png", dpi=150, transparent=True)
print("saved doc/media/txop-throughput.png")
print(f"  DCF   slow={dcf_slow:.2f}+-{dcf_slow_sd:.2f} fast={dcf_fast:.2f}+-{dcf_fast_sd:.2f} agg={dcf_agg:.1f}")
print(f"  TXOP  slow={tx_slow:.2f}+-{tx_slow_sd:.2f} fast={tx_fast:.2f}+-{tx_fast_sd:.2f} agg={tx_agg:.1f}")
print(f"  base  per-station={baseline:.2f} agg={base_agg:.1f}")
