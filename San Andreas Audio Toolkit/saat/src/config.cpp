/* The San Andreas Audio Toolkit (SAAT)
 * Copyright (C) 2005 Dave Schmitt <pdescobar@comcast.net>
 * Uses and/or modifies some code from Radio Free San Andreas (GNU GPL)
 * Copyright (C) 2005 Marcus Eriksson <marerk@lysator.liu.se>
 *
 *
 * This file is part of the San Andreas Audio Toolkit (SAAT)
 *
 * The San Andreas Audio Toolkit is free software; you can redistribute it 
 * and/or modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the 
 * License, or (at your option) any later version.
 *
 * The San Andreas Audio Toolkit is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along 
 * with the San Andreas Audio Toolkit; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * INI/Configuration file functions adapted from RFSA. 
 *
 * Changes include:
 * - Contains DOS line-ending fix (different from Ondra Hosek's fix)
 * - Adds an output operation, a simple assign function, and simple 
 *   << operators to go along with the >> operators
 * - Also changes the bracketing/indenting conventions for no good reason
 *   other than uniformity with the rest of SAAT code.
 */

#include <stdexcept>
#include <sstream>

#include "config.h"

using namespace std;

// In RFSA, this was the >> operator, but I am more comfortable with the 
// syntax of the << operator. So I originally changed this to << and then 
// realized that a simple >> operator which calls this one would easily
// allow either syntax. Actual code is almost entirely unchanged from RFSA.
istream &operator<<(config &conf, istream &is) {
  string section;
  string line;
  size_t lineno = 0;

  while (getline(is, line)) {
    lineno++;

    size_t idx = line.find_first_not_of(" \t\r");
    if (idx == string::npos)
      continue;
    line.erase(0, idx);
    idx = line.find_last_not_of(" \t\r");
    line.erase(idx + 1);
      
    switch(line[0]) {
    case '#': // Comments
      {
	continue;
      }
      break;
      
    case '[':
      {
	idx = line.find(']');
	if (idx == string::npos) {
	  ostringstream oss;
	  oss << "Config file error at line " << lineno << ".";
	  throw runtime_error(oss.str());
	} else {
	  section = line.substr(1, idx - 1);
	  section.erase(0, section.find_first_not_of(" \t"));
	  if (section.size() == 0) {
	    ostringstream oss;
	    oss << "Config file error at line " << lineno << ".";
	    throw runtime_error(oss.str());
	  } else {
	    section.erase(section.find_last_not_of(" \t") + 1);
	  }
	}
      }
      break;

    default:
      {
	idx = line.find('=');
	if (idx == string::npos || idx == 0) {
	  ostringstream oss;
	  oss << "Config file error at line " << lineno << ".";
	  throw runtime_error(oss.str());
	} else if (section.size() == 0) {
	  throw runtime_error("No section defined before first setting.");
	} else {
	  string key = line.substr(0, idx);
	  string val = line.substr(idx + 1);
	  key.erase(key.find_last_not_of(" \t") + 1);
	  val.erase(0, val.find_first_not_of(" \t"));
	  // Associate section.key with val
	  conf.settings[section + "." + key] = val;
	}
      }
      break;
    }
  }
  return is;
}

// Reverse of the above input operation
istream &operator>>(istream &is, config &conf) {
  conf << is;
  return is;
}


// Output based on (default) lexicographical sort
ostream &operator<<(ostream &os, config &conf) {
  string this_section = "";
  string last_section = "";

  typedef map<string, string>::const_iterator CI;
  for (CI c = conf.settings.begin() ; c != conf.settings.end() ; ++c) {
    size_t idx = c->first.find('.');
    this_section = c->first.substr(0,idx);
    if (this_section != last_section) {
      last_section = this_section;
      os << endl << '[' << this_section << ']' << endl;
    }
    os << c->first.substr(idx+1) << " = " << c->second << endl;
  }
  
  return os;
}

// Reverse of the above output operation
ostream &operator>>(config &conf, ostream &os) {
  os << conf;
  return os;
}

// square bracket operator; unchanged from RFSA
string config::operator[](const string &key) const {
  map<string, string>::const_iterator it = settings.find(key);

  if(it == settings.end())
    throw directive_not_found(key);
  else
    return it->second;
}

// lookup; unchanged from RFSA
string config::lookup(const string &key, const string &def) const {
  try {
    return (*this)[key];
  } 
  catch (const directive_not_found &e) {
    return def;
  }
}

// I need to modify/create keys myself in SAAT, so here's a bunch of
// assignment operations. A simple string assignment followed by a bunch
// of integer types which are converted to decimal strings and then stored.
// I wouldn't be surprised if there was some clever way to do this with
// less code duplication.
string config::assign(const string &key, const string &val) {
  (this->settings)[key] = val;
  return val;
}

string config::assign(const string &key, const uint32_t val) {
  ostringstream str_val;
  str_val << dec << val;
  (this->settings)[key] = str_val.str();
  return str_val.str();
}

string config::assign(const string &key, const int32_t val) {
  ostringstream str_val;
  str_val << dec << val;
  (this->settings)[key] = str_val.str();
  return str_val.str();
}

string config::assign(const string &key, const uint16_t val) {
  ostringstream str_val;
  str_val << dec << val;
  (this->settings)[key] = str_val.str();
  return str_val.str();
}

string config::assign(const string &key, const uint8_t val) {
  // Note the (0 + val) to coerce a readable number rather than a char
  ostringstream str_val;
  str_val << dec << (0 + val);
  (this->settings)[key] = str_val.str();
  return str_val.str();
}
