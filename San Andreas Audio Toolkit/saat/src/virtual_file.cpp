/* The San Andreas Audio Toolkit (SAAT): virtual_file.cpp
 * Copyright (C) 2005 Dave Schmitt <pdescobar@comcast.net>
 * Uses and/or modifies some code from Radio Free San Andreas (GNU GPL)
 * Copyright (C) 2005 Karl-Johan Karlsson <san-andreas@creideiki.user.lysator.liu.se>
 * Also uses and/or modifies some code from libsndfile (also GNU GPL)
 * Copyright (C) 1999-2005 Erik de Castro Lopo <erikd@mega-nerd.com>
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
 * Using memory buffers as virtual files for library interfacing.
 */

#include <cstring>

#include <unistd.h>

#include "virtual_file.h"

// Constructor
virtual_file::virtual_file(size_t max_file_size) {
  if ( (max_file_size > 0) && (max_file_size < VF_MAX_BUFFER_SIZE) ){
    max_size = max_file_size;
  } else {
    max_size = VF_MAX_BUFFER_SIZE;
  }
  size = 0;
  offset = 0;
  data = new uint8_t[max_size];
}

// Destructor
virtual_file::~virtual_file() {
  delete[] data;
}

// Non-member helper functions
size_t vf_fread(void *dest, size_t size, size_t count, void *buff) {
  virtual_file *vf = (virtual_file *)buff;

  // This will break badly for files over 2Gig in length; ok for our use
  size_t read_size = count*size; 
  if (vf->offset + read_size > vf->size)
    read_size = vf->size - vf->offset;
  
  memcpy (dest, vf->data + vf->offset, read_size);
  vf->offset += read_size;

  return read_size;
} // vf_fread

size_t vf_fwrite(const void *source, size_t size, size_t count, void *buff) {
  virtual_file *vf = (virtual_file *)buff;

  // This will break badly for files over 2Gig in length; ok for our use
  if (vf->offset >= vf->max_size)
    return 0;

  size_t write_size = count*size; 
  if ( (vf->offset + write_size) > vf->max_size )
    write_size = vf->max_size - vf->offset;

  memcpy (vf->data + vf->offset, source, write_size);
  vf->offset += write_size;

  if (vf->offset > vf->size)
    vf->size = vf->offset;

  return write_size;
} // vf_fwrite

sf_count_t vf_read(void *dest, sf_count_t count, void *buff) {
  virtual_file *vf = (virtual_file *)buff;

  // This will break badly for files over 2Gig in length; ok for our use
  if (vf->offset + count > vf->size)
    count = vf->size - vf->offset;
  
  memcpy (dest, vf->data + vf->offset, count);
  vf->offset += count;

  return count;
} // vf_read

sf_count_t vf_write(const void *source, sf_count_t count, void *buff) {
  virtual_file *vf = (virtual_file *)buff;

  // This will break badly for files over 2Gig in length; ok for our use
  if (vf->offset >= vf->max_size)
    return 0;
  if ( (vf->offset + count) > vf->max_size )
    count = vf->max_size - vf->offset;

  memcpy (vf->data + vf->offset, source, (sf_count_t) count);
  vf->offset += count;

  if (vf->offset > vf->size)
    vf->size = vf->offset;

  return count;
} // vf_write

sf_count_t vf_get_filelen(void *buff) {
  virtual_file *vf = (virtual_file *)buff;

  return vf->size;
} // vf_get_filelen

sf_count_t vf_seek(sf_count_t offset, int whence, void *buff) {
  virtual_file *vf = (virtual_file *)buff;

  switch (whence) {
  case SEEK_SET:
    vf->offset = offset;
    break;
  case SEEK_CUR:
    vf->offset = vf->offset + offset;
    break;
  case SEEK_END:
    vf->offset = vf->size + offset;
    break;
  default:
    break;
  };
  return vf->offset;
} // vf_seek

sf_count_t vf_tell(void *buff) {
  virtual_file *vf = (virtual_file *)buff;

  return vf->offset ;
} // vf_tell


