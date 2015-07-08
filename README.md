Sunfish
========

Sunfish is computer Shogi program.

Build
-----

### Linux / OS X

```
make release

# parallel
make release -j

# clean
make clean
```

### Other Platforms

```
cmake path/to/src
```

Usage
-----

### Execute

```
./sunfish

# help
./sunfish --help

# network (CSA protocol)
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
