# Verdict of the StochasticErrorModel module tests, applied to the results/*.sca of the run.
#
# Prints verdict=PASS if everything below matches, verdict=FAIL otherwise.
#
# -v lo=, hi=       range of the number of UDP packets that the sink received
# -v want=...       space separated name=value pairs, e.g. "packetErrorRate.WHOLE=0.3 bitErrorRate.DATA=1e-05",
#                   the value of every <name> that the StochasticErrorModelProbe of the sink recorded
#                   (min and max, to a relative precision of 1e-6; an expected 0 must be exact);
#                   name=nan expects NaN for every request
# -v rate=...       space separated name=value pairs, the histogram of the ErrorRateInd tag of the received
#                   packets recorded by the radio of the sink (packetErrorRate, bitErrorRate, symbolErrorRate)
function rel(a, b) { d = a - b; if (d < 0) d = -d; m = b < 0 ? -b : b; return m == 0 ? d : d / m }
/^scalar .*sink\.udp packetReceived:count/ { n = $4 }
/^scalar .*sink\.wlan\[0\]\.radio\.receiver\.errorModel / { probe[$3] = $4 }
/^statistic .*sink\.wlan\[0\]\.radio (packetErrorRate|bitErrorRate|symbolErrorRate):histogram$/ { cur = $3; sub(/:histogram/, "", cur); next }
/^statistic / { cur = "" }
cur != "" && $1 == "field" && ($2 == "min" || $2 == "max" || $2 == "count") { h[cur, $2] = $3 }
END {
    printf "received=%d\n", n
    printf "inRange=%s\n", (n >= lo && n <= hi) ? "YES" : "NO"
    bad = !(n >= lo && n <= hi)
    nr = split(rate, rs, " ")
    for (i = 1; i <= nr; i++) {
        split(rs[i], kv, "=")
        k = kv[1]; w = kv[2] + 0
        printf "%s count=%d min=%.9g max=%.9g expected=%.9g\n", k, h[k, "count"], h[k, "min"], h[k, "max"], w
        ok = h[k, "count"] > 0 && rel(h[k, "min"], w) < 1e-6 && rel(h[k, "max"], w) < 1e-6
        printf "%s_ok=%s\n", k, ok ? "YES" : "NO"
        bad += !ok
    }
    nw = split(want, ws, " ")
    for (i = 1; i <= nw; i++) {
        split(ws[i], kv, "=")
        k = kv[1]
        if (kv[2] == "nan") {
            # the model returned NaN for every request of this rate and part
            printf "%s count=%d nanCount=%d expected=nan\n", k, probe[k ".count"], probe[k ".nanCount"]
            ok = probe[k ".count"] > 0 && probe[k ".nanCount"] == probe[k ".count"]
        }
        else {
            w = kv[2] + 0
            printf "%s min=%.9g max=%.9g expected=%.9g\n", k, probe[k ".min"], probe[k ".max"], w
            ok = (k ".min") in probe && probe[k ".count"] > 0 && probe[k ".nanCount"] == 0 && rel(probe[k ".min"], w) < 1e-6 && rel(probe[k ".max"], w) < 1e-6
        }
        printf "%s_ok=%s\n", k, ok ? "YES" : "NO"
        bad += !ok
    }
    printf "verdict=%s\n", bad ? "FAIL" : "PASS"
}
