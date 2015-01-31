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
#include "tecplotread.hpp"
  
tecplotread::tecplotread(string filename) :
  validation_marker(0.0),
  version(""),
  byte_order(0),
  file_type(0),
  title("")
{
  
  // open the file
  file.open(filename.c_str(), ifstream::binary);

  // validate opening of file
  assert(file.good());
  
  // Header section
  // section i
  char buffer[9];
  file.read (buffer, 8);
  buffer[8] = '\0';
  version = string(buffer);

  // section ii
  readbin(byte_order);
  
  // section iii
  readbin(file_type);
  title = ascii_to_string();
  readbin(number_variables);
  
  variable_names.resize(number_variables);
  for (int i(0); i<number_variables; i++)
    variable_names[i] = ascii_to_string();

  // section iv
  readbin(validation_marker);
  assert(validation_marker == 299.0);
  while (validation_marker == 299.0)
  {
    zone* z = new zone;

    z->zone_name = ascii_to_string();    
    readbin(z->parent_zone);     
    readbin(z->strand_id);  
    readbin(z->solution_time);
    readbin(z->not_used);
    readbin(z->zone_type);
    readbin(z->data_packing);
    readbin(z->var_location);
        
    if (z->var_location == 1)
    {
      z->vars_location.resize(number_variables);
      for (int i(0); i<number_variables; i++)
        readbin(z->vars_location[i]);
    }
    
    // face neighbors
    readbin(z->face_neighbors);
    if (z->face_neighbors == 1)
    {
      readbin(z->number_face_neighbors);
      if (z->number_face_neighbors != 0)
      {
        readbin(z->face_neighbors_mode);
        if (z->zone_type >= 1 and z->zone_type <= 7 )
          readbin(z->fe_face_neighbors);
      }
    }
    
    // ordered zone
    if (z->zone_type == 0)
    {
      readbin(z->imax);
      readbin(z->jmax);
      readbin(z->kmax);
      if (z->jmax == 1 and z->kmax == 1) // 1D data
        z->number_elements = (z->imax-1);
      else if (z->kmax == 1) // 2D data
        z->number_elements = (z->imax-1) * (z->jmax-1);
      else 
        z->number_elements = (z->imax-1) * (z->jmax-1) * (z->kmax-1);
      
      z->number_points = z->imax * z->jmax * z->kmax;
    }
    
    // finite element zone
    if (z->zone_type >= 1 and z->zone_type <= 7)
    {
      readbin(z->number_points);
      // only for FEPOLYGON or FEPOLYHEDRON
      if (z->zone_type == 6 or z->zone_type == 7)
      {
        readbin(z->number_faces);
        readbin(z->total_faces);
        readbin(z->boundary_faces);
        readbin(z->total_boundary_connections);
      }

      readbin(z->number_elements);
      readbin(z->icell); // for future used, set to zero
      readbin(z->jcell); // for future used, set to zero
      readbin(z->kcell); // for future used, set to zero
    }

    // todo create an auxiliary structure
    // add all the structures to a vector
    readbin(validation_marker);
    while (validation_marker == 1)
    {
      auxiliary* temp = new auxiliary;
      temp->auxiliary_name = ascii_to_string();
      readbin(temp->auxiliary_format); // only allow 0
      temp->auxiliary_value = ascii_to_string();
      z->auxiliaries.emplace_back(temp);
    }

    zones.emplace_back(z);

    // read the next marker to check if 
    // there is more than one zone
    readbin(validation_marker);
  } // end of zone section
  
  // section v
  if (validation_marker == 399)
  {
    // read geometries
    cout << "Geometry section not implemented. Stopping." << endl;
    exit(-1);
  }
  // end of geometrie
  
  ////////////////////////
  // end of header section
  ////////////////////////
  
  // Data section
  assert(validation_marker == 357.0);
  for (auto index(0); index<zones.size(); index++)
  {
    readbin(validation_marker);  
    assert(validation_marker == 299.0);
    
    zones[index]->variable_format.resize(number_variables);
    for (int i(0); i<number_variables; i++)
      readbin(zones[index]->variable_format[i]);
    
    readbin(zones[index]->has_passive_variables);
    zones[index]->passive_variables.resize(number_variables, 0);
    if (zones[index]->has_passive_variables)
    {
      for (int i(0); i<number_variables; i++)
        readbin(zones[index]->passive_variables[i]);
    }
    readbin(zones[index]->has_variable_sharing);
    if (zones[index]->has_variable_sharing)
    {
      zones[index]->variable_sharing.resize(number_variables);
      for (int i(0); i<number_variables; i++)
        readbin(zones[index]->variable_sharing[i]);
    }
    
    readbin(zones[index]->zone_share_connectivity); // if -1 no sharing
    zones[index]->min_value.resize(number_variables);
    zones[index]->max_value.resize(number_variables);
    for (int i(0); i<number_variables; i++)
    {
      readbin(zones[index]->min_value[i]);
      readbin(zones[index]->max_value[i]);
    }
      
    // read the data tables
    zones[index]->variable_index.resize(4);
    for (int i(0); i<number_variables; i++)
    {
      if (not zones[index]->passive_variables[i])
      {
        switch (zones[index]->variable_format[i])
        {
          case 1:
          {
            vector<float> tempvalues;
            read_zone_data(tempvalues, index, i);
            zones[index]->data_float.emplace_back(tempvalues);
            zones[index]->variable_index[0].emplace_back(i);
            break;
          }
        
          case 2:
          {
            vector<double> tempvalues;
            read_zone_data(tempvalues, index, i);       
            zones[index]->data_double.emplace_back(tempvalues);
            zones[index]->variable_index[1].emplace_back(i);
            break;
          }
        
          case 3:
          {
            vector<long int> tempvalues;
            read_zone_data(tempvalues, index, i);       
            zones[index]->data_longint.emplace_back(tempvalues);
            zones[index]->variable_index[2].emplace_back(i);
            break;
          }
        
          case 4:
          {
            vector<int> tempvalues;
            read_zone_data(tempvalues, index, i);       
            zones[index]->data_int.emplace_back(tempvalues);
            zones[index]->variable_index[3].emplace_back(i);
            break;
          }
           
          default:
          {
            cout << "type of data not supported: " << zones[index]->variable_format[i] << endl;
            exit(-1);
            break;
          }
        } 
      }
    }
    
    // TODO: complete this part
    // I must say that I don't 100% understand
    // this section ... 
    // specific to ordered zone
    /*
    if (zones[index]->zone_type == 0)
    {
      if (zones[index]->zone_share_connectivity == -1 and zones[index]->number_face_neighbors != 0)
      {
        // Face neighbor connections.
        // N = (number of miscellaneous user defined
        // face neighbor connections) * P
        // (See note 5 below).
      }  
    }
    */
    
    // specific to fe zone
    if (zones[index]->zone_type >= 1 and zones[index]->zone_type <= 7)
    {
      // not FEPOLYGON or FEPOLYHEDRON
      if (zones[index]->zone_type != 6 or zones[index]->zone_type != 7)
      {
        int l(0);
        if (zones[index]->zone_share_connectivity == -1)
        {
          // Set the number of node per element according
          // to the zone type.
          // 1=FELINESEG 2=FETRIANGLE 3=FEQUADRILATERAL 
          // 4=FETETRAHEDRON 5=FEBRICK
          switch (zones[index]->zone_type)
          {
            case 1:
            {
              l = 2;
              break;
            }
            case 2:
            {
              l = 3;
              break;
            }
            case 3:
            {
              l = 4;
              break;
            }
            case 4:
            {
              l = 4;
              break;
            }
            case 5:
            {
              l = 8;
              break;
            }          
            default:
            {
              cout << "element type unknown << (" << zones[index]->zone_type << "), can't read connectivity."  << endl;
              exit(-1);
              break;
            }
          }
          read_zone_connectivity(zones[index]->zone_connectivity, index, l);     
        }
        
      }
      
      // TODO: other section to complete
      // 6=FEPOLYGON 7=FEPOLYHEDRON
      //else
      //{
        
      //}  
    }
  }
};

// method to read a data type from a binary file
template <typename T>
void tecplotread::readbin(T& obj) {
    file.read(reinterpret_cast<char *>(addressof(obj)), sizeof(T));
}

template <typename T>
void tecplotread::read_zone_data(vector<T>& values, int zoneindex, int varindex)
{
  // all variables located at nodes
  if (zones[zoneindex]->var_location == 0)
  {
    values.resize(zones[zoneindex]->number_points);
  }
  else 
  {
    // if not we have to look at the location of 
    // each particular variables
    if (zones[zoneindex]->vars_location[varindex] == 0)
    {
      values.resize(zones[zoneindex]->number_points);
    }
    else
    {
      values.resize(zones[zoneindex]->number_elements);
    }
  }
  
  for (auto i(0); i<values.size(); i++)
    readbin(values[i]);    
}

template <typename T>
void tecplotread::read_zone_connectivity(vector<T>& values, int zoneindex, int node_per_element)
{
  // resize vector
  values.resize(zones[zoneindex]->number_elements * node_per_element);
  
  for (auto i(0); i<values.size(); i++)
    readbin(values[i]);    
}

void tecplotread::basic_information()
{
// Validation
cout << "Basic file information" << endl;
cout << "version: " << version << endl;
cout << "byte_order: " << byte_order << endl; 

cout << "file_type: " << file_type << endl;
cout << "title: " << title << endl;
cout << "number_variables: " << number_variables << endl;
cout << "variable_names: " << variable_names << endl;

// zone stats
cout << "Contains " << zones.size() << " zones." << endl;
cout << endl;
}

void tecplotread::complete_information()
{
  // Validation
  cout << "Complete file information" << endl;
  cout << "version: " << version << endl;
  cout << "byte_order: " << byte_order << endl; 

  cout << "file_type: " << file_type << endl;
  cout << "title: " << title << endl;
  cout << "number_variables: " << number_variables << endl;
  cout << "variable_names: " << variable_names << endl;

  // zone stats
  cout << "Contains " << zones.size() << " zones." << endl; 
  cout << endl;
  
  for (size_t i(0); i<zones.size(); i++)
  {
    cout << "zonename: " << zones[i]->zone_name << endl;
    cout << "parent_zone: " << zones[i]->parent_zone << endl;
    cout << "strand_id: " << zones[i]->strand_id << endl;
    cout << "solution_time: " << zones[i]->solution_time << endl;
    cout << "not_used: " << zones[i]->not_used << endl;
    cout << "zone_type: " << zones[i]->zone_type << endl;
    cout << "data_packing: " << zones[i]->data_packing << endl;
    cout << "var_location: " << zones[i]->var_location << endl;
    cout << "vars_location: " << zones[i]->vars_location << endl;
    cout << "face_neighbors: " << zones[i]->face_neighbors << endl;
    cout << "number_face_neighbors: " << zones[i]->number_face_neighbors << endl;
    cout << "imax: " << zones[i]->imax << endl;
    cout << "jmax: " << zones[i]->jmax << endl;
    cout << "kmax: " << zones[i]->kmax << endl;
    cout << "number_points: " << zones[i]->number_points << endl;
    cout << "number_faces: " << zones[i]->number_faces << endl;
    cout << "total_faces: " << zones[i]->total_faces << endl;
    cout << "boundary_faces: " << zones[i]->boundary_faces << endl;
    cout << "total_boundary_connections: " << zones[i]->total_boundary_connections << endl;
    cout << "number_elements: " << zones[i]->number_elements << endl;
    cout << "icell: " << zones[i]->icell << endl;
    cout << "jcell: " << zones[i]->jcell << endl;
    cout << "kcell: " << zones[i]->kcell << endl; 
    cout << "variable_format: " << zones[i]->variable_format << endl;
    cout << "has_passive_variables: " << zones[i]->has_passive_variables << endl;
    cout << "passive_variable: " << zones[i]->passive_variables << endl;
    cout << "has_variable_sharing: " << zones[i]->has_variable_sharing << endl;
    cout << "variable_sharing: " << zones[i]->variable_sharing << endl;
    cout << "zone_share_connectivity: " << zones[i]->zone_share_connectivity << endl;
    cout << "min_value: " << zones[i]->min_value << endl;
    cout << "max_value: " << zones[i]->max_value << endl;
    cout << "Size of the float vector: " << zones[i]->data_float.size() << endl;
    for (size_t j(0); j<zones[i]->data_float.size(); j++)
      cout << "\t" << "[" << j << "]: " << variable_names[zones[i]->variable_index[0][j]] << " " << zones[i]->data_float[j].size() << " values." << endl; 
    cout << "Size of the double vector: " << zones[i]->data_double.size() << endl;
    for (size_t j(0); j<zones[i]->data_double.size(); j++)
      cout << "\t" << "[" << j << "]: " << variable_names[zones[i]->variable_index[1][j]] << " " << zones[i]->data_double[j].size() << " values." << endl; 
    cout << "Size of the long int vector: " << zones[i]->data_longint.size() << endl;
    for (size_t j(0); j<zones[i]->data_longint.size(); j++)
      cout << "\t" << "[" << j << "]: " << variable_names[zones[i]->variable_index[2][j]] << " " << zones[i]->data_longint[j].size() << " values." << endl; 
    cout << "Size of the int vector: " << zones[i]->data_int.size() << endl;
    for (size_t j(0); j<zones[i]->data_int.size(); j++)
      cout << "\t" << "[" << j << "]: " << variable_names[zones[i]->variable_index[3][j]] << " " << zones[i]->data_int[j].size() << " values." << endl; 
    if (zones[i]->zone_share_connectivity == -1)
      cout << "Size of connectivity: " << zones[i]->zone_connectivity.size() << endl;
    cout << endl;
  
  }  
}

void tecplotread::zone_information(int zone_id)
{
  // Validation
  cout << "Information of zones: " << zone_id << endl;
  cout << "version: " << version << endl;
  cout << "byte_order: " << byte_order << endl; 
  cout << "file_type: " << file_type << endl;
  cout << "title: " << title << endl;
  cout << "number_variables: " << number_variables << endl;
  cout << "variable_names: " << variable_names << endl;

  // zone stats
  cout << "zonename: " << zones[zone_id]->zone_name << endl;
  cout << "parent_zone: " << zones[zone_id]->parent_zone << endl;
  cout << "strand_id: " << zones[zone_id]->strand_id << endl;
  cout << "solution_time: " << zones[zone_id]->solution_time << endl;
  cout << "not_used: " << zones[zone_id]->not_used << endl;
  cout << "zone_type: " << zones[zone_id]->zone_type << endl;
  cout << "data_packing: " << zones[zone_id]->data_packing << endl;
  cout << "var_location: " << zones[zone_id]->var_location << endl;
  cout << "vars_location: " << zones[zone_id]->vars_location << endl;
  cout << "face_neighbors: " << zones[zone_id]->face_neighbors << endl;
  cout << "number_face_neighbors: " << zones[zone_id]->number_face_neighbors << endl;
  cout << "imax: " << zones[zone_id]->imax << endl;
  cout << "jmax: " << zones[zone_id]->jmax << endl;
  cout << "kmax: " << zones[zone_id]->kmax << endl;
  cout << "number_points: " << zones[zone_id]->number_points << endl;
  cout << "number_faces: " << zones[zone_id]->number_faces << endl;
  cout << "total_faces: " << zones[zone_id]->total_faces << endl;
  cout << "boundary_faces: " << zones[zone_id]->boundary_faces << endl;
  cout << "total_boundary_connections: " << zones[zone_id]->total_boundary_connections << endl;
  cout << "number_elements: " << zones[zone_id]->number_elements << endl;
  cout << "icell: " << zones[zone_id]->icell << endl;
  cout << "jcell: " << zones[zone_id]->jcell << endl;
  cout << "kcell: " << zones[zone_id]->kcell << endl; 
  cout << "variable_format: " << zones[zone_id]->variable_format << endl;
  cout << "has_passive_variable: " << zones[zone_id]->has_passive_variables << endl;
  cout << "passive_variable: " << zones[zone_id]->passive_variables << endl;
  cout << "has_variable_sharing: " << zones[zone_id]->has_variable_sharing << endl;
  cout << "variable_sharing: " << zones[zone_id]->variable_sharing << endl;
  cout << "zone_share_connectivity: " << zones[zone_id]->zone_share_connectivity << endl;
  cout << "min_value: " << zones[zone_id]->min_value << endl;
  cout << "max_value: " << zones[zone_id]->max_value << endl;
  cout << "Size of the float vector: " << zones[zone_id]->data_float.size() << endl;
  for (size_t j(0); j<zones[zone_id]->data_float.size(); j++)
    cout << "\t" << "[" << j << "]: " << variable_names[zones[zone_id]->variable_index[0][j]] << " " << zones[zone_id]->data_float[j].size() << " values." << endl; 
  cout << "Size of the double vector: " << zones[zone_id]->data_double.size() << endl;
  for (size_t j(0); j<zones[zone_id]->data_double.size(); j++)
    cout << "\t" << "[" << j << "]: " << variable_names[zones[zone_id]->variable_index[1][j]] << " " << zones[zone_id]->data_double[j].size() << " values." << endl; 
  cout << "Size of the long int vector: " << zones[zone_id]->data_longint.size() << endl;
  for (size_t j(0); j<zones[zone_id]->data_longint.size(); j++)
    cout << "\t" << "[" << j << "]: " << variable_names[zones[zone_id]->variable_index[2][j]] << " " << zones[zone_id]->data_longint[j].size() << " values." << endl; 
  cout << "Size of the int vector: " << zones[zone_id]->data_int.size() << endl;
  for (size_t j(0); j<zones[zone_id]->data_int.size(); j++)
    cout << "\t" << "[" << j << "]: " << variable_names[zones[zone_id]->variable_index[3][j]] << " " << zones[zone_id]->data_int[j].size() << " values." << endl; 
  if (zones[zone_id]->zone_share_connectivity == -1)
    cout << "Size of connectivity: " << zones[zone_id]->zone_connectivity.size() << endl;
  cout << endl;
}

tecplotread::~tecplotread()
{
  // delete all the created zones
  for (auto i(0); i<zones.size(); i++)
    delete zones[i];
  zones.resize(0);
}

// method to read a series of ASCII values until
// it reaches a zero. It outputs the resulting string
string tecplotread::ascii_to_string()
{
  assert(file.good());
  
  string value;
  int ascii(-1);
  
  file.read((char*) &ascii, sizeof(ascii));
  while (ascii != 0)
  {
    char temp = (char) ascii;
    value.append(sizeof(char), temp);
    file.read(reinterpret_cast<char *>(&ascii), sizeof(ascii));
  }
  
  return value;
};

zone::zone() :
  zone_name(""),
  parent_zone(0),
  strand_id(0),
  solution_time(0.0),
  not_used(0),
  zone_type(0),
  data_packing(0),
  var_location(0),
  face_neighbors(0),
  number_face_neighbors(0),
  face_neighbors_mode(0),
  fe_face_neighbors(0),
  imax(0),
  jmax(0),
  kmax(0),
  number_points(0),
  number_faces(0),
  total_faces(0),
  boundary_faces(0),
  total_boundary_connections(0),
  number_elements(0),
  icell(0),
  jcell(0),
  kcell(0),
  has_passive_variables(0),
  has_variable_sharing(0),
  zone_share_connectivity(0)
{};

int zone::getVariableFormat(int id) const { 
  assert(id >= 0 and id < variable_format.size()); 
  return variable_format[id]; 
};

vector<int> zone::getVariableIndex(int id) const { 
  assert(id >= 0 and id < variable_index.size()); 
  return variable_index[id]; 
};

int zone::getVariableIndex(int type, int id) const { 
  assert(type >= 0 and type < variable_index.size()); 
  assert(id >= 0 and id < variable_index[type].size()); 
  return variable_index[type][id]; 
};

int zone::getPassiveVariables(int id) const { 
  assert(id >= 0 and id < passive_variables.size()); 
  return passive_variables[id]; 
};

int zone::getVariableSharing(int id) const { 
  assert(id >= 0 and id < variable_sharing.size()); 
  return variable_sharing[id]; 
};

double zone::getMinValue(int id) const { 
  assert(id >= 0 and id < min_value.size()); 
  return min_value[id]; 
};

double zone::getMaxValue(int id) const { 
  assert(id >= 0 and id < max_value.size()); 
  return max_value[id]; 
};

int zone::getVarsLocation(int id) const { 
  assert(id >= 0 and id < vars_location.size()); 
  return vars_location[id]; 
};

vector<float> zone::getDataFloat(int id) const { 
  assert(id >= 0 and id < data_float.size()); 
  return data_float[id]; 
};
vector<double> zone::getDataDouble(int id) const { 
  assert(id >= 0 and id < data_double.size()); 
  return data_double[id]; 
};
vector<long int> zone::getDataLongInt(int id) const { 
  assert(id >= 0 and id < data_longint.size()); 
  return data_longint[id]; 
};
vector<int> zone::getDataInt(int id) const { 
  assert(id >= 0 and id < data_int.size()); 
  return data_int[id]; 
};

auxiliary::auxiliary() :
  auxiliary_name(""),
  auxiliary_format(-1), 
  auxiliary_value("")
{};
