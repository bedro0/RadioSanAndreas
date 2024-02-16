/* The San Andreas Audio Toolkit (SAAT): saat_sfx.h
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

#ifndef _SAAT_SFX_H_
#define _SAAT_SFX_H_

// Data structures and constants related to the format of the stream file; 
// hopefully self-evident because I'm not explaining them ;)

const int       BH_NUM_SOUND_ENTRIES = 400;
const int32_t   BH_UNKNOWN_32_DEFAULT = -1;
const string    BH_UNKNOWN_32_DEFAULT_STR = "-1";
const uint16_t  BH_UNKNOWN_16_DEFAULT = 0;
const string    BH_UNKNOWN_16_DEFAULT_STR = "0";
const size_t    SF_FRAME_BUFFER_LEN = 1024;

// Invocation modes
enum mode { MODE_UNKNOWN, MODE_IMPORT, MODE_EXPORT };

typedef struct {
  uint32_t offset;
  int32_t unknown_32;
  uint16_t sample_rate;
  uint16_t unknown_16;
} sound_entry;

typedef struct {
  uint32_t num_sounds;
  sound_entry sounds[BH_NUM_SOUND_ENTRIES];
} bank_header;

/* Prints usage message using the supplied command name.
 *
 * Arguments:
 *  cmd  The command name used to start the program.
 *
 * Returns:
 *  Nothing
 */
void print_usage(const string cmd);

/* Main sfx export processor
 *
 * Arguments:
 *  argc  Number of commandline arguments (unmodified from main)
 *  argv  Pointer to the commandline argument list (unmodified from main)
 *
 * Returns:
 *  0 on success, >0 for error; see source for specifics.
 */
int sfx_export (int argc, char **argv);

/* Main sfx import processor
 *
 * Arguments:
 *  argc  Number of commandline arguments (unmodified from main)
 *  argv  Pointer to the commandline argument list (unmodified from main)
 *
 * Returns:
 *  0 on success, >0 for error; see source for specifics.
 */
int sfx_import (int argc, char **argv);

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
bool verify_sfx_ini(config &import_ini, const string source_dir);

#endif /* _SAAT_SFX_H_ */
