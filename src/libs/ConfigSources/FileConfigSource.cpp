/*  
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>. 
*/

#include "libs/Kernel.h"
#include "ConfigValue.h"
#include "FileConfigSource.h"
#include "ConfigCache.h"

using namespace std;
#include <string>


FileConfigSource::FileConfigSource(){
    this->name_checksum = FILE_CONFIGSOURCE_CHECKSUM;
    this->config_file_found = false;
}

// Transfer all values found in the file to the passed cache
void FileConfigSource::transfer_values_to_cache( ConfigCache* cache ){
    
    // Default empty value
    ConfigValue* result = new ConfigValue;
    
    if( this->has_config_file() == false ){return;}
    // Open the config file ( find it if we haven't already found it ) 
    FILE *lp = fopen(this->get_config_file().c_str(), "r");
    string buffer;
    int c; 
    // For each line 
    do {
        c = fgetc (lp);
        if (c == '\n' || c == EOF){
            // We have a new line
            if( buffer[0] == '#' ){ buffer.clear(); continue; } // Ignore comments
            if( buffer.length() < 3 ){ buffer.clear(); continue; } //Ignore empty lines
            size_t begin_key = buffer.find_first_not_of(" ");
            size_t begin_value = buffer.find_first_not_of(" ", buffer.find_first_of(" ", begin_key));
            string key = buffer.substr(begin_key,  buffer.find_first_of(" ", begin_key) - begin_key).append(" ");
            vector<uint16_t> check_sums = get_checksums(key);
            
            result = new ConfigValue;
            result->found = true;
            result->check_sums = check_sums;
            result->value = buffer.substr(begin_value, buffer.find_first_of("\r\n# ", begin_value+1)-begin_value);
            
            // Append the newly found value to the cache we were passed 
            cache->replace_or_push_back(result);
            
            buffer.clear();
        }else{
            buffer += c;
        }
    } while (c != EOF);  
    fclose(lp);

}

bool FileConfigSource::is_named( uint16_t check_sum ){
    return check_sum == this->name_checksum;
}

void FileConfigSource::write( vector<uint16_t> check_sums, string value ){}

string FileConfigSource::read( vector<uint16_t> check_sums ){}

// Return wether or not we have a readable config file
bool FileConfigSource::has_config_file(){
    if( this->config_file_found ){ return true; }
    this->try_config_file("/local/config");
    this->try_config_file("/sd/config");
    if( this->config_file_found ){
        return true;
    }else{
        return false;
    }

}

// Tool function for get_config_file
inline void FileConfigSource::try_config_file(string candidate){
    FILE *lp = fopen(candidate.c_str(), "r");
    if(lp){ this->config_file_found = true; this->config_file = candidate; }
    fclose(lp);
}

// Get the filename for the config file
string FileConfigSource::get_config_file(){
    if( this->config_file_found ){ return this->config_file; }
    if( this->has_config_file() ){
        return this->config_file;
    }else{
        printf("ERROR: no config file found\r\n");
    }
}






