mkdir -p fuzz-data && echo '       ' > fuzz-data/SEED && afl-fuzz -m none -i fuzz-data -o fuzz-out ./a.out @@
