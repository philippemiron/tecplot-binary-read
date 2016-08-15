/*
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
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <vector>
#include <assert.h>
#include <memory>

using namespace std;

// TODO: the geometry & auxiliary class
// is currently not used in the code
// It has to be validate but for now
// I don't have need for this :|
class auxiliary 
{
  friend class zone;
public:
  auxiliary();
  ~auxiliary() {};
  
  string auxiliary_name;
  int auxiliary_format;
  string auxiliary_value;
};

class geometry
{
  friend class tecplotread;
public:
  geometry();
  ~geometry() {};
  
  int coord_sys;
  int scope;
  double x;   // or theta
  double y;   // or r
  double z;   // or dummy
  int zone;   // 0 = all
  int color;
  int fill_color;
  int is_filled;  // 0=no 1=yes
  int geom_type;  // 0=Line 1=Rectangle 2=Square 3=Circle 4=ellipse
  int line_pattern; // 0=Solid 1=Dashed 2=DashDot 3=DashDotDot 4=Dotted 5=LongDash
  int pattern_length;
  double line_thickness;
  int num_ellipse_pts;
  int arrowhead_style; // 0=Plain 1=Filled 2=Hollow
  int arrowhead_attachement; // 0=None 1=Beg 2=End 3=Both
  double arrowhead_size;
  double arrowhead_angle;
  string macro_name;
  int polyline_data_type; // 1= Float, 2=Double
  int clipping; // Clipping 0=ClipToAxes 1=ClipToViewport 2=ClipToFrame

  // line
  int number_of_polylines;
  int number_of_points; // line 1.
  vector<float> x_float;
  vector<double> x_double;  
  vector<float> y_float;
  vector<double> y_double;
  vector<float> z_float; // Grid3D Only
  vector<double> z_double; // Grid3D Only
};

class zone
{
  friend class tecplotread;
public:
  zone();
  ~zone() {};
  
  string getZoneName() const { return zone_name; };
  int getParentZone() const { return parent_zone; };
  int getStrandId() const { return strand_id; };
  double getSolutionTime() const { return solution_time; };
  int getZoneType() const { return zone_type; };
  int getDataPacking() const { return data_packing; };
  int getVarLocation() const { return var_location; };
  vector<int> getVarsLocation() const { return vars_location; };
  int getVarsLocation(int id) const;
  
  int getFaceNeighbors() const { return face_neighbors; };
  int getNumberFaceNeighbors() const { return number_face_neighbors; };
  int getFaceNeighborsMode() const { return face_neighbors_mode; };
  int getFeFaceNeighbors() const { return fe_face_neighbors; };
  
  int getIMax() const { return imax; };
  int getJMax() const { return jmax; };
  int getKMax() const { return kmax; };
  
  int getNumberPoints() const { return number_points; };
  int getNumberFaces() const { return number_faces; };
  int getTotalFaces() const { return total_faces; };
  int getTotalBoundaryConnections() const { return total_boundary_connections; };
  int getNumberElements() const { return number_elements; };
  int getICell() const { return icell; };
  int getJCell() const { return jcell; };
  int getKCell() const { return kcell; };
  
  vector<int> getVariableFormat() const { return variable_format; };
  vector<vector<int>> getVariableIndex() const { return variable_index; };
  int getHasPassiveVariables() const { return has_passive_variables; };
  vector<int> getPassiveVariables() const { return passive_variables; };
  int getHasVariableSharing() const { return has_variable_sharing; };
  vector<int> getVariableSharing() const { return passive_variables; };
  int getZoneShareConnectivity() const { return zone_share_connectivity; };
  vector<double> getMinValue() const { return min_value; };
  vector<double> getMaxValue() const { return max_value; };
  int getVariableFormat(int id) const;
  vector<int> getVariableIndex(int id) const;
  int getVariableIndex(int type, int id) const;
  int getPassiveVariables(int id) const;
  int getVariableSharing(int id) const;
  double getMinValue(int id) const;
  double getMaxValue(int id) const;
    
  vector<vector<float>> getDataFloat() const { return data_float; };
  vector<vector<double>> getDataDouble() const { return data_double; };
  vector<vector<long int>> getDataLongInt() const { return data_longint; };
  vector<vector<int>> getDataInt() const { return data_int; };
  vector<float> getDataFloat(int id) const;
  vector<double> getDataDouble(int id) const;
  vector<long int> getDataLongInt(int id) const;
  vector<int> getDataInt(int id) const;
  vector<int> getZoneConnectity() const { return zone_connectivity; };

private:   
  string zone_name;
  int parent_zone;
  int strand_id;
  double solution_time;
  int not_used;
  int zone_type;  // 0=ORDERED 1=FELINESEG 2=FETRIANGLE 3=FEQUADRILATERAL 
                  // 4=FETETRAHEDRON 5=FEBRICK 6=FEPOLYGON 7=FEPOLYHEDRON
  int data_packing; // 0=Block, 1=Point
  int var_location;
  vector<int> vars_location;

  // face neighbors
  int face_neighbors;
  int number_face_neighbors;
  int face_neighbors_mode;
  int fe_face_neighbors;

  // ordered zone
  int imax;
  int jmax;
  int kmax;

  // fe zone
  int number_points;

  // if fepolygon or fepolyhedron
  int number_faces;
  int total_faces;
  int boundary_faces;
  int total_boundary_connections;
  int number_elements;
  int icell; // not used set to 0
  int jcell; // not used set to 0
  int kcell; // not used set to 0

  // auxiliary data name structure
  vector<auxiliary*> auxiliaries;

  // data
  // section i
  vector<int> variable_format;
  vector<vector<int>> variable_index;
  int has_passive_variables;
  vector<int> passive_variables;
  int has_variable_sharing;
  vector<int> variable_sharing;
  int zone_share_connectivity;
  vector<double> min_value;
  vector<double> max_value;
    
  // data vectors
  // theses vectors are empty if
  // none variable is of one type
  vector<vector<float>>   data_float;
  vector<vector<double>>  data_double;
  vector<vector<long int>>data_longint;
  vector<vector<int>>     data_int;

  // section ii: specific to order zone
  vector<int> face_neighbors_connections;

  // section iii: specific to fe zone
  vector<int> zone_connectivity;
};

// basic class
class tecplotread
{
public:
  tecplotread(string filename);
  ~tecplotread();
  // output information about binary file
  void basic_information();
  void complete_information();
  void zone_information(int zone_id);
  
  float getValidationMarker() const { return validation_marker; };
  string getVersion() const { return version; };
  int getByteOrder() const { return byte_order; };
  int getFileType() const { return file_type; };
  string getTitle() const { return title; };
  int getNumberVariables() const { return number_variables; };
  vector<string> getVariableNames() const { return variable_names; };
  
  string getVariableName(int id) const { 
    assert(id >= 0 and id < int(variable_names.size())); 
    return variable_names[id]; 
  };
  
  int getNumberZones() const { return zones.size(); };
  zone* getZone(int id) const { 
    assert(id >= 0 and id < int(zones.size())); 
    return zones[id]; 
  };
  
private:
  string ascii_to_string();
  template <typename T> void readbin(T& obj);
  template <typename T> void read_zone_data(vector<T>& values, int zone_index, int var_index);
  template <typename T> void read_zone_connectivity(vector<T>& values, int zone_index, int node_per_element);
  
  // iostream
  ifstream file;
  
  // section marker
  float validation_marker;
  
  // section i
  string version;
  
  // section ii
  int byte_order;
  
  // section iii
  int file_type; // 0=FULL 1=GRID 2=SOLUTION
  string title;
  int number_variables;
  vector<string> variable_names;
  
  // section iv : zone with data
  // each zone is added to this
  // vector on read
  vector<zone*> zones;
  
  // TODO
  // section v: geometries
  // vector<geometry*> geometries;
};

// template to define operator << for vector
template < class T >
ostream& operator << (ostream& os, const vector<T>& v) 
{
    os << "[";
    bool first(true);
    for (typename vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii)
    {
      if (!first) 
      {
        os << ", " << *ii;
      }  
      else
      {
        os << *ii;
        first = false;
      }
    }
    os << "]";
  
    return os;
}
