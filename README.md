Sunfish
========

Sunfish is computer Shogi program.

Build and Execute
------------------

### GCC

#### Compile

```
make

# multi process
make -j

# clean
make clean
```

#### Execute

```
./sunfish

# help
./sunfish --help

# network(csa protocol)
./sunfish -n

# unit test
./sunfish --test
```

### OS X

Open `sunfish3.xcodeproj` and build `release/debug > My Mac`.

## Usage

### Show help

```
./sunfish --help
```

### Import fv.bin

```
rm evdata
cp /hoge/bonanza/fv.bin ./fv.bin
./sunfish
```
