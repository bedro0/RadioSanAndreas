/* The San Andreas Audio Toolkit (SAAT): virtual_file.h
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
 * Simple Virtual File Implementation
 * Data structures and helper functions related to using memory buffers
 * instead of files for interfacing with libvorbis and libsndfile.
 * This implementation is specific to SAAT and would have to be modified
 * for general use due mostly to me having to use libsndfile datatypes.
 */

#ifndef _VIRTUAL_FILE_H_
#define _VIRTUAL_FILE_H_

#include <sys/types.h>
#include <unistd.h>

#include <sndfile.h>

// Maximum buffer size for default constructor
const size_t VF_MAX_BUFFER_SIZE = 10485760; // 10 MB max size

// Class definition
class virtual_file {
 public:
  uint8_t *data;
  size_t  size;
  size_t  max_size;
  size_t  offset;

  // Constructors
  virtual_file(size_t max_file_size=0);

  // Destructors
  ~virtual_file();
};

/* Reads 'count' chunks of 'size' bytes from 'vf' and stores them in 'dest'
 * Functionally equivalent to fread() from <stdio>
 *
 * Arguments:
 *  dest  Pointer to destination container
 *  size  Size of a chunk of data to read
 *  count Number of chunks to read
 *  vf    Pointer to virtual_file to read from
 *
 * Returns:
 *  Number of chunks actually read; less than 'count' is an error.
 */
size_t vf_fread(void *dest, size_t size, size_t count, void *vf);

/* Writes 'count' chunks of 'size' bytes from 'source' and stores them in 'vf'
 * Functionally equivalent to fwrite() from <stdio>
 *
 * Arguments:
 *  source Pointer to source container
 *  size   Size of a chunk of data to read
 *  count  Number of chunks to read
 *  vf     Pointer to virtual_file to write to
 *
 * Returns:
 *  Number of bytes actually written; less than 'count' is an error.
 */
size_t vf_fwrite(const void *source, size_t size, size_t count, void *vf);

/* Reads 'count' bytes from 'vf' and stores them in 'dest'
 *
 * Arguments:
 *  dest  Pointer to destination container
 *  count Number of bytes to read
 *  vf    Pointer to virtual_file to read from
 *
 * Returns:
 *  Number of bytes actually read; less than 'count' is an error.
 */
sf_count_t vf_read(void *dest, sf_count_t count, void *vf);

/* Writes 'count' bytes from 'source' and stores them in 'vf'
 *
 * Arguments:
 *  source Pointer to source container
 *  count  Number of bytes to read
 *  vf     Pointer to virtual_file to write to
 *
 * Returns:
 *  Number of bytes actually written; less than 'count' is an error.
 */
sf_count_t vf_write(const void *source, sf_count_t count, void *vf);

/* Gets length of the data inside 'vf'(i.e., vf->size)
 *
 * Arguments:
 *  vf     Pointer to virtual_file to write to
 *
 * Returns:
 *  Length of the data (i.e., vf->size)
 */
sf_count_t vf_get_filelen(void *vf);

/* Seeks to a position in the data of 'vf' based on starting position 'whence'
 * If 'whence' is SEEK_SET, seeks to 'offset' bytes from the data's beginning
 * If 'whence' is SEEK_CUR, seeks to 'offset' bytes from the current position
 * If 'whence' is SEEK_END, seeks to 'offset' bytes from the data's end
 *
 * Arguments:
 *  offset Relative bytes from position
 *  whence Starting position; valid values SEEK_SET, SEEK_CUR, SEEK_END
 *  vf     Pointer to virtual_file to seek within
 *
 * Returns:
 *  Position after seek (number of bytes from start of data)
 */
sf_count_t vf_seek(sf_count_t offset, int whence, void *vf);

/* Returns current offset of seek pointer within 'vf' (i.e. vf->offset)
 *
 * Arguments:
 *  vf Pointer to virtual_file to examine
 *
 * Returns:
 *  Position of seek pointer within vf (i.e. vf->offset)
 */
sf_count_t vf_tell(void *vf);

#endif /* _VIRTUAL_FILE_ */
