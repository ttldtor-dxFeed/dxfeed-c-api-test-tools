# dxfeed-c-api-test-tools

```shell
git clone --recurse-submodules git@github.com:ttldtor/dxfeed-c-api-test-tools.git
cd dxfeed-c-api-test-tools
mkdir build
cd build
cmake ..
cmake --build .
```

## mt-reader
The multi thread file (candle web service) reader

Example of use:

```
mt-reader <path to file 1> <path to file 2>
```

## bench
The simple benchmark utility.

Supports only TimeAndSale, counts the average number of events per second and writes the result in CSV.

Example of use:

```
bench <endpoint> <event type> <symbol>
```

## collision-detector
The utility for detecting hash collisions for symbols from IPF (file)

Example of use:

```
collision-detector <ipf-file-path>
```

## plb-tester
Utility for checking the functioning of the PriceLevelBook class. 
PriceLevelBook subscribes to snapshot and collects price levels from orders.

Example of use:

```
plb-tester <endpoint> <symbol> <source> <number of levels>
```

`<number of levels>` - The PLB levels number (0 - all levels)

