Copyright (C) 2015 Philippe Miron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses

Tecplot library to read binary file create with TECIO
or from Tecplot 360 interface. It is based on the information
provided in the "Data Format Guide" available here:
*ftp://ftp.tecplot.com/pub/doc/tecplot/360/dataformat.pdf*.

I believe this is the #!TDV112 format. This library is in
no case errors proofs and I have tested and using it only
with data files I'm using on a regular basic.

Usage:

1. `git clone the repo`
2. `make`
3. `./main.bin examples/x.plt`
4. This will *print* on screen the
   information of the plt file

With the printed information, it is easy to see the variable 
tables and name available, the number of zones, etc.

1. Retrieving variables name
  - `vector<string> vars_name = tpobj->getVariableNames(0);`
  - `string var1 = tpobj->getVariableNames(0);`

2. Retrieving number of points and elements in zone1 :
  - `int number_points = tpobj->getZone(0)->getNumberPoints();`
  - `int number_elements = tpobj->getZone(0)->getNumberElements();`

3. Retrieving variables data section:
 - if datatype is float
 - This would retrieve data from 1st variable of 1st zone:
   `vector<float> var1_zone1 = tpobj->getZone(0)->getDataFloat(0);`
 - Or you could retrive all the values of all variable of 1st zone:
   `vector<vector<float>> vars_zone1 = tpobj->getZone(0)->getDataFloat();`

All variables read by the library are available throught getters method
*take a look in the .hpp file*!
   

  
