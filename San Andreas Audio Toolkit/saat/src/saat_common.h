/* The San Andreas Audio Toolkit (SAAT): saat_common.h
 * Copyright (C) 2005 Dave Schmitt <pdescobar@comcast.net>
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
 * SAAT Common Headers
 * Data types and function prototypes used by multiple SAAT programs.
 */

#ifndef _SAAT_COMMON_H_
#define _SAAT_COMMON_H_

#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "lookup.h"

#if defined WIN32_PATHS || defined WIN32
#define SLASH '\\'
#else
#define SLASH '/'
#endif

#define SAAT_VERSION 1.10
#define SAAT_VERSION_STR "Version 1.10"

using namespace std;

/* Creates an ALL CAPS version of a string
 *
 * Arguments:
 *  source  The string to capitalize
 *
 * Returns:
 *  The new string
 */
string allcaps(const string source);

/* Simple check to see if we are on a big-endian platform
 *
 * Arguments:
 *  None
 *
 * Returns:
 *  true if big-endian; false if not
 */
bool is_bigendian(void);

/*
 * Prints a message saying that the software doesn't run on big-endian
 * machines; since I use it in a couple places, it's a function.
 *
 * Arguments:
 *  None
 *
 * Returns:
 *  Nothing
 */
void print_bigendian_msg(void);

/* Returns the basename of a filepath; that is the filename only without
 * the parent drive/directory names.
 *
 * Arguments:
 *  filename  The original filepath
 *
 * Returns:
 *  The basename
 */
string basename(const string filename);

/* Returns the pathname of a filepath; that is the basename is stripped
 * and the filename is reduced to only the parent drive/directory names.
 *
 * Arguments:
 *  filename  The original filepath
 *
 * Returns:
 *  The pathname
 */
string pathname(const string filename);

/* Checks to see if a filename contains an absolute or relative path
 * by checking if the first char is the directory separator or a drive name
 *
 * Arguments:
 *  filename  The filepath
 *
 * Returns:
 *  true if an absolute path, false if relative
 */
bool is_absolute_pathname(const string filename);

/* Remove potential problem characters from a filename
 *
 * Arguments:
 *  filename  The filename to sanitize
 *
 * Returns:
 *  The resulting sanitized filename string
 */
string sanitize_filename(const string filename);

/* Creates directory using the correct mkdir() call for this platform; if
 * on a unix-like platform, the directory will be created with mode 0755.
 *
 * Arguments:
 *  dir_name  The name of the directory to create
 *
 * Returns:
 *  value returned by internal mkdir call
 */
int create_dir(const char *dir_name);

/* Reads a chunk of data from a file descriptor without encoding.
 * Basically a wrapper around the standard libc read() system call.
 *
 * Arguments:
 *  fd    The file descriptor to read from
 *  buff  The buffer into which the data read is stored
 *  size  How much data to read
 *
 * Returns:
 *  number of bytes actually read; generally the value returned by read()
 */
ssize_t read_chunk(int fd, void *buff, size_t size);

/* Populates a configuration data structure from a file
 *
 * Arguments:
 *  conf      The data structure to fill
 *  filename  The file to read
 *
 * Returns:
 *  true if successful, false if an error
 */
bool populate_conf(config &conf, const char *filename);

/* Populates a lookup_table data structure from a file
 *
 * Arguments:
 *  look      The data structure to fill
 *  filename  The file to read
 *
 * Returns:
 *  true if successful, false if an error
 */
bool populate_lookup(lookup_table &look, const char *filename);

#endif /* _SAAT_COMMON_H_ */


