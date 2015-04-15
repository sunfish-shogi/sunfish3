Sunfish
========

Sunfish is computer Shogi program.

Build
-----

### GCC/Clang

```
make release

# parallel
make release -j

# clean
make clean
```

### Xcode

Open `sunfish3.xcodeproj` and build `release/debug > My Mac`.

### VC

TODO..

Usage
-----

### Execute

```
./sunfish

# help
./sunfish --help

# network (csa protocol)
./sunfish -n

# unit test
./sunfish --test
```

### Import fv.bin

```
rm eval.bin
cp path/to/bonanza/fv.bin ./fv.bin
./sunfish
```

License
-------

MIT License
