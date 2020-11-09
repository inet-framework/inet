clang-tidy -p $INET_ROOT/src/ --fix --checks=\
bugprone-*,\
modernize-*,\
-bugprone-narrowing-conversions,\
-bugprone-reserved-identifier,\
-modernize-avoid-c-arrays,\
-modernize-use-trailing-return-type,\
-modernize-use-emplace,\
-modernize-use-auto,\
-modernize-pass-by-value,\
-clang-analyzer-cplusplus.NewDeleteLeaks,\
-clang-analyzer-deadcode.DeadStores\
 src/inet/$1/**/*.cc > clang-tidy.out
 
