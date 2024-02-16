/* The San Andreas Audio Toolkit (SAAT): lookup.h
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
 * Lookup file functions modeled somewhat on the RFSA config handlers
 *
 * Should handle more things internally and better shield the user from
 * implementation details. Because the GTASA lookup files are sorted and
 * this implementation allows essentially random access, you can really
 * screw things up. To fix it, one would have to make sure the table is
 * still properly sorted when doing the insert or assign operations and
 * the whole seek pointer access might have to be rethought. Also, it
 * could use better documentation. ;)
 */

#ifndef LOOKUP_H
#define LOOKUP_H

#include <iostream>
#include <list>

#include <sys/types.h>
#include <unistd.h>

enum LOOKUP_TYPE { LT_UNKNOWN, LT_TRAKLKUP, LT_BANKLKUP };

class lookup_entry {
 public:
  uint8_t index[4];
  uint32_t offset;
  uint32_t length;

  // Constructors
  lookup_entry(uint8_t i=0x00, uint32_t o=0, uint32_t l=0);
  lookup_entry(LOOKUP_TYPE t, uint8_t i=0x00, uint32_t o=0, uint32_t l=0);

  /* Write out this lookup_entry object to an ostream.
   *
   * Arguments:
   *  os  The ostream to write to.
   *  le  The lookup_entry object to use.
   *
   * Returns:
   *  The ostream.
   */
  friend std::ostream &operator<<(std::ostream &os, lookup_entry &le);
  friend std::ostream &operator>>(lookup_entry &le, std::ostream &os);
};
  
typedef std::list<lookup_entry> lookup_list;

// An instance of this class holds the contents of a GTA San Andreas
// lookup file in a linked list of lookup_entry objects.
class lookup_table {
 public:

  // Constructor
  lookup_table(LOOKUP_TYPE t=LT_UNKNOWN) {type = t; p=table.begin();}

  /* Read a lookup file into this lookup_table object from an istream.
   *
   * Arguments:
   *  is  The istream to read from.
   *  lt  The lookup_table object to populate.
   *
   * Returns:
   *  The istream.
   */
  friend std::istream &operator<<(lookup_table &lt, std::istream &is);
  friend std::istream &operator>>(std::istream &is, lookup_table &lt);
  
  /* Write a lookup file from this lookup_table object to an ostream.
   *
   * Arguments:
   *  os  The ostream to write to.
   *  lt  The lookup_table object to use.
   *
   * Returns:
   *  The ostream.
   */
  friend std::ostream &operator<<(std::ostream &os, lookup_table &lt);
  friend std::ostream &operator>>(lookup_table &lt, std::ostream &os);
  
  /* Seek to beginning of table.
   *
   * Arguments:
   *  None.
   *
   * Returns:
   *  Nothing.
   */
  void begin(void);

  /* Seek to end of table.
   *
   * Arguments:
   *  None.
   *
   * Returns:
   *  Nothing.
   */
  void end(void);

  /* Seek to next entry in table.
   *
   * Arguments:
   *  None.
   *
   * Returns:
   *  Nothing.
   */
  void next(void);

  /* Seek to previous entry in table.
   *
   * Arguments:
   *  None.
   *
   * Returns:
   *  Nothing.
   */
  void previous(void);

  /* Count entries for specified index.
   *
   * Arguments:
   *  index  The index to count and seek to.
   *
   * Returns:
   *  The number of entries for that index.
   */
  int count_index(uint8_t i);

  /* Seek to first entry for specified index.
   *
   * Arguments:
   *  index  The index to seek to.
   *
   * Returns:
   *  true if successful, false if not (probably because index not in lookup)
   */
  bool seek_to_index(uint8_t i);

  /* Returns the current entry.
   *
   * Arguments:
   *  None.
   *
   * Returns:
   *  The current entry.
   */
  lookup_entry entry(void);

  /* Copies the values of the supplied entry into the current table entry.
   *
   * Arguments:
   *  e  The entry whose values should be duplicated in the current entry.
   *
   * Returns:
   *  A copy of the current entry after assignment.
   */
  lookup_entry assign(lookup_entry e);

  /* Inserts a new entry at the current seekpoint and increments seekpoint
   *
   * Arguments:
   *  e  The entry whose values should be duplicated in the new entry.
   *
   * Returns:
   *  A copy of the new entry after insertion.
   */
  lookup_entry insert(lookup_entry e);
  
  /* Clears all lookup_table entries for the supplied index; sets the
   * seekpoint to the previous start location for that index. The idea
   * is that you can clear a given index and then insert in new values
   * for that index immediately. 
   *
   * Arguments:
   *  index  The index to clear
   *
   * Returns:
   *  Nothing.
   */
  void clear(uint8_t i);

 private:

  /**
   * Internals; type of table, the actual list of values, and seekpoint
   */
  LOOKUP_TYPE type;
  lookup_list table;
  lookup_list::iterator p;
};

#endif
