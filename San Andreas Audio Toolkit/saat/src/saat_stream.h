/* The San Andreas Audio Toolkit (SAAT): saat_stream.h
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
 * SAAT Stream Headers
 * Data types and function prototypes used for stream processing
 */

#ifndef _SAAT_STREAM_H_
#define _SAAT_STREAM_H_

// Program invocation modes
enum mode { MODE_UNKNOWN, MODE_IMPORT, MODE_EXPORT, MODE_EXRFSA };

// Data structures and constants related to the format of the stream file; 
// hopefully self-evident because I'm not explaining them ;)

const int      TH_NUM_BEAT_ENTRIES = 1000;
const int      TH_NUM_LENGTH_ENTRIES = 8;
const int32_t  TH_BEAT_TIMING_DEFAULT = -1;
const string   TH_BEAT_TIMING_DEFAULT_STR = "-1";
const int32_t  TH_BEAT_CONTROL_DEFAULT = 0;
const string   TH_BEAT_CONTROL_DEFAULT_STR = "0";
const int32_t  TH_BEAT_CONTROL_END = 0x21;
const uint32_t TH_LENGTH_DEFAULT = 0xCDCDCDCD;
const uint32_t TH_TRAILER_DEFAULT = 0xCDCD0001;

// Magic numbers for stream encoder functions
const uint8_t encode_key[] = {0xea, 0x3a, 0xc4, 0xa1, 
			      0x9a, 0xa8, 0x14, 0xf3,
			      0x48, 0xb0, 0xd7, 0x23, 
			      0x9d, 0xe8, 0xff, 0xf1};

typedef struct {
  int32_t timing;
  int32_t control;
} beat_entry;

typedef struct {
  uint32_t length;
  uint32_t extra;
} length_entry;

typedef struct {
  beat_entry beats[TH_NUM_BEAT_ENTRIES];
  length_entry lengths[TH_NUM_LENGTH_ENTRIES];
  uint32_t trailer;
} track_header;

/* Prints usage message using the supplied command name.
 *
 * Arguments:
 *  cmd  The command name used to start the program.
 *
 * Returns:
 *  Nothing
 */
void print_usage(const string cmd);

/* Main stream export processor
 *
 * Arguments:
 *  mymode  The specific export mode to use
 *  argc    Number of commandline arguments (unmodified from main)
 *  argv    Pointer to the commandline argument list (unmodified from main)
 *
 * Returns:
 *  0 on success, >0 for error; see source for specifics.
 */
int stream_export (const mode mymode, int argc, char **argv);

/* Main stream import processor
 *
 * Arguments:
 *  argc  Number of commandline arguments (unmodified from main)
 *  argv  Pointer to the commandline argument list (unmodified from main)
 *
 * Returns:
 *  0 on success, >0 for error; see source for specifics.
 */
int stream_import (int argc, char **argv);

/* Reads a chunk of data from a file descriptor with stream encoding.
 * Basically a wrapper around the read_chunk() call in saat_common.cpp
 * which is itself a wrapper around the standard libc read() system call.
 *
 * Arguments:
 *  fd     The file descriptor to read from
 *  buff   The buffer into which the data read is stored
 *  size   How much data to read
 *  index  Index used internally by the encoder routine
 *
 * Returns:
 *  Number of bytes actually read; generally the value returned by read()
 */
ssize_t read_encoded_chunk(const int fd, void *buff, const size_t size, 
			   int &index);

/* Encodes a chunk of data using the GTASA stream encoding method.
 *
 * Arguments:
 *  buff   Pointer to data to be encoded
 *  size   Size of data to be encoded
 *  index  Index used internally by the encoder routine
 *
 * Returns:
 *  Nothing
 */
void stream_encode(int8_t *buff, const size_t size, int &index);

/* Returns the md5 fingerprint of the unmodifed stream whose name is
 * supplied; used simply to fool the program into RFSA tagging compatibility
 *
 * Arguments:
 *  filename  The filename to match
 *
 * Returns:
 *  The md5 fingerprint string corresponding to that filename or "" if no match
 */
string get_fingerprint(const string filename);

/* Returns the lookup index for the supplied filename string
 *
 * Arguments:
 *  filename  The filename to lookup
 *
 * Returns:
 *  The index corresponding to that filename or -1 if no match
 */
int get_lookup_index(const string filename);

/* Does various sanity checks on the stream_import.ini file
 *
 * Arguments:
 *  import_ini  The INI to process
 *  source_dir  The default directory for the source files
 *
 * Returns:
 *  true if INI is OK, false if there's a problem.
 */
bool verify_stream_ini(config &import_ini, const string source_dir);

#endif /* _SAAT_STREAM_H_ */
