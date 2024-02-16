/* The San Andreas Audio Toolkit (SAAT): lookup.cpp
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
 * Lookup file functions modeled on the RFSA config handlers
 *
 * Should handle more things internally and better shield the user from
 * implementation details.
 */

#include <algorithm>
#include <list>
#include <sstream>

#include <sys/types.h>
#include <unistd.h>

#include "lookup.h"

using namespace std;

// Constructors for lookup_entry class; there are enough options and
// enough initializing that I wanted them out of the header file.
lookup_entry::lookup_entry(uint8_t i, uint32_t o, uint32_t l) {
  index[0]=i; 
  index[1]=index[2]=index[3]=0x00; 
  offset=o; 
  length=l;
}

lookup_entry::lookup_entry(LOOKUP_TYPE t, uint8_t i, uint32_t o, uint32_t l) {
  uint8_t temp = 0x00;
  if (t == LT_TRAKLKUP) { 
    temp = 0xCD; 
  }
  if (t == LT_BANKLKUP) { 
    temp = 0xCC; 
  }
  index[0]=i; 
  index[1]=index[2]=index[3]=temp; 
  offset=o;
  length=l;
}

// lookup_entry member functions
  
ostream &operator<<(ostream &os, lookup_entry &le) {
  // For binary output
  uint8_t *p = (uint8_t *)&le;
  for (unsigned int i=0; i<sizeof(lookup_entry); i++) {
    os << p[i];
  }
  
  /*
  // For Plain text output
  os << "INDEX: ";
  for (int i = 0; i<4 ; i++) {
    os << "0x" << hex << 0+le.index[i] << " " << dec;
  }
  os << "; OFFSET: " << le.offset 
     << " ; LENGTH: " << le.length;
  */

  os << flush;
  return os;
}

ostream &operator>>(lookup_entry &le, ostream &os) {
  // opposite of previous
  os << le;
  return os;
}

// Internal predicate functions; see example in Stroustrup 18.4.2.1 (p.517)
// useful for comparing entry indices

class index_eq : public unary_function<lookup_entry,bool> {
  uint8_t i;
public:
  explicit index_eq(const uint8_t idx) : i(idx) { }
  bool operator()(const lookup_entry& le) const {return (le.index[0] == i);}
};

class index_lt : public unary_function<lookup_entry,bool> {
  uint8_t i;
public:
  explicit index_lt(const uint8_t idx) : i(idx) { }
  bool operator()(const lookup_entry& le) const {return (le.index[0] < i);}
};

class index_gt : public unary_function<lookup_entry,bool> {
  uint8_t i;
public:
  explicit index_gt(const uint8_t idx) : i(idx) { }
  bool operator()(const lookup_entry& le) const {return (le.index[0] > i);}
};

// lookup_table member functions

istream &operator>>(istream &is, lookup_table &lt) {
  lookup_entry entry;

  while(is.read((char*)&entry,sizeof(lookup_entry))) {
    lt.table.push_back(entry);
  }

  return is;
}

istream &operator<<(lookup_table &lt, istream &is) {
  // opposite of previous
  is >> lt;
  return is;
}

ostream &operator>>(lookup_table &lt, ostream &os) {
  lookup_list::iterator q;
  //int counter = 0;

  for (q=lt.table.begin() ; q != lt.table.end() ; q++) {
    os << *q;
    //++counter;
    //os << "Entry " << counter << ": " << *q << endl;
  }
  return os;
}

ostream &operator<<(ostream &os, lookup_table &lt) {
  // opposite of previous
  lt >> os;
  return os;
}

void lookup_table::begin(void) {
  this->p = this->table.begin();
}

void lookup_table::end(void) {
  this->p = this->table.end();
}

void lookup_table::next(void) {
  (this->p)++;
}

void lookup_table::previous(void) {
  (this->p)--;
}

int lookup_table::count_index(uint8_t i) {
  return (int)count_if(this->table.begin(), this->table.end(), index_eq(i));
}

bool lookup_table::seek_to_index(uint8_t i) {
  this->p = find_if(this->table.begin(), this->table.end(), index_eq(i));
  // Here's an odd way of returning true if 'i' was found and false if not
  return (this->p != this->table.end());
}

lookup_entry lookup_table::entry(void) {
  if ( this->p == this->table.end() ) {
    // Should throw an exception here; instead, drop seek to last entry
    (this->p)--;
  }
  return *(this->p);
}

lookup_entry lookup_table::assign(lookup_entry e) {
  for (int i=0 ; i<4 ; i++) {
    (*(this->p)).index[i] = e.index[i];
  }
  (*(this->p)).offset = e.offset;
  (*(this->p)).length = e.length;
  return *(this->p);
}

lookup_entry lookup_table::insert(lookup_entry e) {
  this->table.insert(this->p, e);
  return e;
}
  
void lookup_table::clear(uint8_t i) {
  this->table.remove_if(index_eq(i));
  this->p = find_if(this->table.begin(), this->table.end(), index_gt(i));
}
