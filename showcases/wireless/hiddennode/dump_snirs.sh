inet -u Cmdenv -c WallOnRtsOff -r 0 --sim-time-limit 100ms --cmdenv-log-level=INFO --cmdenv-express-mode=false | grep "Computing SNIR mean" -A 1 > snirs.txt
