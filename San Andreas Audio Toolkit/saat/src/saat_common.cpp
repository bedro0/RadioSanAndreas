/* The San Andreas Audio Toolkit (SAAT): saat_common.cpp
 * Copyright (C) 2005 Dave Schmitt <pdescobar@comcast.net>
 * Uses and/or modifies some code from Radio Free San Andreas (GNU GPL)
 * Copyright (C) 2005 Ondra Hosek <white.timberwolf@aon.at> 
 * Copyright (C) 2005 Karl-Johan Karlsson <san-andreas@creideiki.user.lysator.liu.se>
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
 * SAAT Common
 * Functions used by multiple SAAT programs. Actually, a few of them are
 * only used by one of the programs, but I'm too lazy to break this into
 * three different parts...
 */

#include <fstream>
#include <iostream>
#include <string>

#include <cstring>

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "saat_common.h"

using namespace std;

// Makes a string ALL CAPS
string allcaps(const string source) {
  string dest = "";

  string::const_iterator p = source.begin();
  while (p != source.end()) {
    dest += toupper(*p);
    ++p;
  }
  return dest;
}

// Simple endian test, modified from RFSA
bool is_bigendian(void) {
  union {
    uint16_t two_bytes;
    uint8_t  one_byte[2];
  } endian_test;

  endian_test.two_bytes = 0x0102;
  return( (endian_test.one_byte[0] == 1) && (endian_test.one_byte[1] == 2) );
}

// The generic big-endian incompatibility message
void print_bigendian_msg(void) {
  cerr << "SAAT has detected a big-endian processor,"
       << " but Grand Theft Auto: San Andreas" << endl
       << "stores its audio files in little-endian"
       << " format. Unfortunately, SAAT is not" << endl
       << "currently capable of handling this"
       << " data mismatch and must abort." << endl;
}

// Get everything after the last SLASH (defined based on platform) 
// If WIN32, also make sure it's after colon ("d:foo" is a valid win32
// filename meaning file "foo" in current directory of drive "d:")
string basename(const string filename) {
  string::size_type last_slash = filename.rfind(SLASH);
  if (last_slash == string::npos) {
    last_slash = 0;
  } else {
    ++last_slash;
  }
  string base = filename.substr(last_slash);
  string::size_type last_colon = 0;
#if defined WIN32_PATHS || defined WIN32
  last_colon = base.rfind(':');
  if (last_colon == string::npos) {
    last_colon = 0;
  } else {
    ++last_colon;
  }
#endif
  return base.substr(last_colon);
}

// Gets the pathname for a file by stripping the basename out of it.
string pathname(const string filename) {
  string::size_type end_path = filename.rfind(basename(filename));
  if (end_path == 0) {
    // there is no pathname; filename and basename are the same
    return("");
  } else if (end_path == string::npos) {
    // the filename didn't contain the basename?? err, okay...
    return(filename);
  } else {
    return(filename.substr(0,end_path));
  }
}

// Is this an absolute path?
bool is_absolute_pathname(const string filename) {
  if (filename.length() < 1) {
    // A blank path should be an error, but we'll consider it "relative"
    return false;
  }
  if (filename[0] == SLASH) {
    // Starts with the directory separator
    return true;
  }
#if defined WIN32_PATHS || defined WIN32
  // WIN32 only, drivename + colon; e.g. "x:"
  if (filename.length() > 1) { // Makes sure filename[1] is in range
    if ( (isalpha(filename[0])) && (filename[1] == ':') ) {
      return true;
    }
  }
#endif
  return false; // Default
}

// Removes bad chars from filename; currently only deals with the 9 bad
// chars for win32 when compiled with -DWIN32_FILENAMES.
string sanitize_filename(const string source) {
  // In perl, I would throw together a few regexps to do this, but I know
  // nothing of regexps in C++. Since the replacements are fairly simple, I 
  // will use a simple looping mechanism and a switch which would probably 
  // be god-awful slow for general use.
  string dest;

  string::const_iterator p = source.begin();
  while (p != source.end()) {
#if defined WIN32_FILENAMES || defined WIN32
    // Win32 hates the following 9 characters \/:*?"<>|
    // Double quote -> single quote
    // Angle brackets -> square brackets
    // Colons -> double-dashes (with leading space)
    // Slashes, asterisks, pipes, and question marks simply removed
    switch (*p) {
    case '\"':
      dest += "'";
      break;
    case '<':
      dest += "[";
      break;
    case '>':
      dest += "]";
      break;
    case ':':
      dest += " --";
      break;
    case '\\':
    case '/':
    case '*':
    case '?':
    case '|':
      break;
    default:
      dest += (*p);
    }
#endif
    ++p;
  }
  return dest;
}

// Wrapper to make our mkdir calls more platform independent
int create_dir(const char *dir_name) {
#if defined WIN32
  return mkdir(dir_name);
#else
  return mkdir(dir_name, 0755);
#endif
}

// Read part of file; basicly a wrapper to read() used to handle some of 
// the error checking outside of the main code.
ssize_t read_chunk(int fd, void *buff, size_t size) {
  int result = 0;
  size_t bytes_read = 0;
  // This loop might be a bad idea; I don't completely understand it, but
  // I believe it is necessary because of the EINTR possibility.
  while (bytes_read < size) {
    result = read(fd, buff, size);
    if (result == -1 && errno == EINTR)
      continue;
    if (result == -1) {
      cerr << "Error reading input file " << strerror(errno) << endl;
      return result;
    }
    if (result == 0) {
      cerr << "Unexpected end of input file while reading " << size
	   << " byte chunk" << endl;
      return result;
    }
    bytes_read += result;
  }
  return result;
}

// Reads file into config data structure
bool populate_conf(config &conf, const char *filename) {
  ifstream conf_file(filename, ios::in | ios::binary);
  if (!conf_file) {
    cerr << "Error opening configuration file:" << filename << endl;
    return false;
  }
  try {
    conf << conf_file;
  }
  catch (const runtime_error &e) {
    cerr << "Error reading configuration file (" << filename << "): "
	 << e.what() << endl;
    return false;
  }
  conf_file.close();
  return true;
}

// Reads file into lookup data structure
bool populate_lookup(lookup_table &look, const char *filename) {
  ifstream lookup_file(filename, ios::in | ios::binary);
  if (!lookup_file) {
    cerr << "Error opening lookup file:" << filename << endl;
    return false;
  }
  try {
    look << lookup_file;
  }
  catch (const runtime_error &e) {
    cerr << "Error reading lookup file (" << filename << "): "
	 << e.what() << endl;
    return false;
  }
  lookup_file.close();
  return true;
}
