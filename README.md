Author: Philippe Miron

## Information

Tecplot library to read binary files created with TECIO or from the Tecplot 360 interface. It is based on the information
provided in the "Data Format Guide" available at [ftp://ftp.tecplot.com/pub/doc/tecplot/360/dataformat.pdf](ftp://ftp.tecplot.com/pub/doc/tecplot/360/dataformat.pdf).

This is based on the `#!TDV112` version format. This library is not error-proof, and I have only tested this code using a handful of data files that I use regularly.

## Usage:

```
git clone https://github.com/philippemiron/tecplot-binary-read.git
cd tecplot-binary-read/
make
./main.bin examples/x.plt
```

This will *print* on the console the `complete_information()` of the plt file.

The printed information makes it easy to see the variable tables and names available, the number of zones, etc.

1. Retrieving the variable name(s):
  - `vector<string> vars_name = tpobj->getVariableNames();`
  - `string var1 = tpobj->getVariableName(0);`

2. Retrieving the number of points or elements in the first zone:
  - `int number_points = tpobj->getZone(0)->getNumberPoints();`
  - `int number_elements = tpobj->getZone(0)->getNumberElements();`

3. Retrieving the variables in the data section:
 - if the datatype is` float`, the following would retrieve data from the first variable of the first zone:
   `vector<float> var1_zone1 = tpobj->getZone(0)->getDataFloat(0);`
 - Or you can retrieve all the values of all variables of the first zone:
   `vector<vector<float>> vars_zone1 = tpobj->getZone(0)->getDataFloat();`

All variables read by the library are available through the getter methods. Take a look at the header file.
