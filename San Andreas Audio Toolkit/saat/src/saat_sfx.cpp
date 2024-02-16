/* The San Andreas Audio Toolkit (SAAT): saat_sfx.cpp
 * Copyright (C) 2005 Dave Schmitt <pdescobar@comcast.net>
 * Uses and/or modifies some code from Radio Free San Andreas (GNU GPL)
 * Copyright (C) 2005 Ondra Hosek <white.timberwolf@aon.at> 
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
 * SAAT SFX Tools
 * - Exports GTASA sfx archive into individual WAVe files
 * - Imports individual WAVe files into a GTASA sfx archive
 *
 * Exit codes (compatible with RFSA):
 * 128 - command line error - usage information printed
 * 64  - refusing to run on big-endian processor
 * 32  - unable to determine lookup index for file export
 * 16  - error reading an ini or lookup input file
 * 8   - sanity check of source file contents failed
 * 4   - error writing output files
 * 2   - error accessing source files
 * 1   - error accessing/creating target directory
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sndfile.h>

#include "saat_common.h"
#include "saat_sfx.h"
#include "config.h"
#include "lookup.h"
#include "virtual_file.h"

using namespace std;

int main (int argc, char **argv) {
  string exe_name = basename(argv[0]);

  cout << "San Andreas Audio Toolkit (SAAT) SFX Tool " 
       << SAAT_VERSION_STR << "\n" << endl;

  if (is_bigendian()) {
    print_bigendian_msg();
    exit(64);
  }

  // Simple argument type/number checks. I'm sure there are better methods.
  bool bad_args = 0;
  mode mymode = MODE_UNKNOWN;
  if ( argc < 2 ) {
    bad_args = 1;
  } else if ( ((string)argv[1] == "-e") || ((string)argv[1] == "-E") ) {
    mymode = MODE_EXPORT;
    if (argc < 5) {
      bad_args = 1;
    }
  } else if ( ((string)argv[1] == "-i") || ((string)argv[1] == "-I") ) {
    mymode = MODE_IMPORT;
    if (argc != 5) {
      bad_args = 1;
    }
  }
  if ( bad_args || mymode == MODE_UNKNOWN ) {
    print_usage(exe_name);
    exit(128);
  }

  if (mymode == MODE_EXPORT) {
    exit(sfx_export(argc, argv));
  } else if (mymode == MODE_IMPORT) {
    exit(sfx_import(argc, argv));
  } else {
    // default, should be impossible.
    exit(0);
  }
} // main

void print_usage(const string cmd) {
  cerr << cmd << " converts between GTASA sfx archives and "
       << "individual WAVe sounds." << endl
       << "\nUsage varies by intended mode:" << endl
       << endl << cmd 
       << " -e <archive_file(s)> <target_dir> <lookup_file>" << endl
       << "  Export mode; exports all sounds "
       << "from <archive_file(s)>, storing the WAVe" << endl
       << "  files in an appropriately "
       << "named subdirectory of <target_dir>." << endl
       << "  Uses <lookup_file> to help locate "
       << "sounds within the archive." << endl
       << endl << cmd 
       << " -i <target_archive> <import_ini> <lookup_file>" << endl
       << "  Import mode; creates <target_archive> "
       << "based on the information and filename" << endl
       << "  references in <import_ini>. Also updates "
       << "<lookup_file> to reflect the changes." << endl;
}

int sfx_export (int argc, char **argv) {
  // Make sure the main target directory is ok.
  int last_arg = argc-2;
  char *arg_lookupfile = argv[argc-1];
  char *target_dir = argv[last_arg];

  struct stat stat_buffer;
  int result;
  result = stat(target_dir, &stat_buffer);
  if ( (result == -1) && (create_dir(target_dir) == -1) ) {
    cerr << "Error creating target directory: " << strerror(errno) << endl;
    return(1);
  }
  // re-stat, since maybe the directory was created
  result = stat(target_dir, &stat_buffer);  
  if (!S_ISDIR(stat_buffer.st_mode)) {
    cerr << "Error accessing target directory: Not a directory." << endl;
    return(1);
  }

  // read the lookup table
  lookup_table banklkup(LT_BANKLKUP);
  if (!populate_lookup(banklkup, arg_lookupfile)) {
    return(16);
  }

  // iterate over input archives, processing each
  for (int infile = 2; infile < last_arg; ++infile) {
    config output_ini;
    string infilename = argv[infile];
    string inbasename = basename(infilename);
    output_ini.assign("Archive.basename", inbasename);
    string archive_name = allcaps(inbasename);
    
    // Determine lookup index which is necessary for determining some of 
    // the sound lengths and will be put in the INI file to help importing.
    // We will conveniently use the ALL CAPS basename for this.
    int lookup_index = get_lookup_index(archive_name);
    if ( (lookup_index < 0) || (lookup_index > 8) ) {
      cerr << "Error: Could not match filename " << inbasename 
	   << " to a valid sound bank" << endl 
	   << "lookup index; SAAT sfx exporter can currently only "
	   << "correctly export files which are" << endl
	   << "named the same as the original game files because "
	   << "it needs to read length information" << endl
	   << "for some sounds in the archive from the lookup file. "
	   << "Future versions may include a way" << endl
	   << "to specify the index via argument switches or a "
	   << "configuration file." << endl;
      return(32);
    }
    // The index is valid, but are there entries for it in the lookup file?
    int num_banks = banklkup.count_index((uint8_t)lookup_index);
    if (num_banks == 0) {
      cerr << "Error in lookup file " << arg_lookupfile 
	   << ": No entries for index " << lookup_index << endl
	   << "The exporter needs to read sound lengths from the"
	   << " lookup file. Aborting." << endl;
      return(16);
    }
    output_ini.assign("Archive.lookup_index", (int32_t)lookup_index);
      
    cout << "Processing file " 
	 << setw((int)ceil(log10((float)(last_arg - 2)))) << infile - 1
	 << " of " << last_arg - 2 << ": " << infilename;
    result = stat(argv[infile], &stat_buffer);
    if (result == -1) {
      cerr << "Error accessing input file " << argv[infile] << ": "
	   << strerror(errno) << endl;
      return(2);
    }
    size_t archive_length = stat_buffer.st_size;
    int in_fd = open(argv[infile], O_RDONLY | O_BINARY);
    if (in_fd == -1) {
      cerr << "Error opening input file " << argv[infile] << ": "
	   << strerror(errno) << endl;
      return(2);
    }
    cout << " (length: " << archive_length << ")" << endl
	 << "  Reading Sounds..." << endl;

    // Creating output subdirectory for the archive under target_dir.
    string export_dir = string(target_dir) + SLASH + archive_name;
    if ( (access(export_dir.c_str(), 00) == -1) &&
	 (create_dir(export_dir.c_str()) == -1) ) {
      cerr << "Error creating directory for archive " << archive_name
	   << ": " << strerror(errno) << endl;
      return(1);
    }
  
#ifdef POSIX_FADVISE
    //Advise the kernel not to cache this data, since we will only read it 
    //once. Since this is only a hint, we don't care if it succeeds or not.
    posix_fadvise(in_fd, 0, 0, POSIX_FADV_NOREUSE);
#endif

    // As in the stream tool, the sfx archive will be read piecemeal which
    // saves on memory but is probably slower.
    int bank = 0;
    size_t read_size = 0;
    size_t head_length = sizeof(bank_header);
    bank_header head;
  
    // Iterating over banks in the archive. We know the number of the banks 
    // from earlier; now make sure the lookup file points to the right spot.
    banklkup.seek_to_index((uint8_t)lookup_index);
    while (read_size < archive_length) {
      // First, name the bank and make a subdirectory for it
      ostringstream bank_name;
      bank_name << "Bank_" << setw(3) << setfill('0') << ++bank;
      cout << "    Bank " << bank << ":..." << flush;
      string section = bank_name.str();
      string bank_dir = string(export_dir) + SLASH + bank_name.str();
      if ( (access(bank_dir.c_str(), 00) == -1) &&
	   (create_dir(bank_dir.c_str()) == -1) ) {
	cerr << "Error creating directory for bank " << bank 
	     << "of archive " << archive_name << endl
	     << "Path: " << bank_dir << endl
	     << "Error: " << strerror(errno) << endl;
	return(1);
      }

      // Read bank header first
      result = read_chunk(in_fd, &head, head_length);
      if (result <=0 ) {
	cerr << "Error occurred while reading " << argv[infile] << "; "
	     << read_size << " bytes were read." << endl;
	return(2);
      }
      read_size += result;
      int num_sounds = (int)head.num_sounds;
      if ( num_sounds > BH_NUM_SOUND_ENTRIES ) {
	cerr << "Error reading archive " << archive_name << endl
	     << "Sound bank " << bank_name.str() << " claims to have "
	     << num_sounds << " sounds, but "
	     << "maximum allowed is " << BH_NUM_SOUND_ENTRIES << endl;
	return(2);
      }
      output_ini.assign(section + ".num_sounds", (int32_t)num_sounds); 

      // Iterate over sounds and process each; need to access 'sound' later
      int sound=0;
      for (sound=1; sound<=num_sounds; ++sound) {
	// Because I want sounds to count from 1, I run into problems
	// with array subscripting; using head_index compensates for that
	int head_index = sound-1;
	ostringstream sound_name;
	sound_name << "sound_" << setw(3) << setfill('0') << sound;
	cout << sound << "..." << flush;
	size_t raw_length = 0;
	if (sound == num_sounds) {
	  // Last sound in bank; no easy offset comparison
	  if (bank == num_banks) {
	    // Last bank too. Length must be rest of file.
	    raw_length = archive_length - read_size;
	  } else {
	    // Need offset of next bank in archive from lookup file
	    // so we peek ahead and then come back.
	    banklkup.next();
	    raw_length = (size_t)banklkup.entry().offset - read_size;
	    banklkup.previous();
	  }
	} else {
	  // simplest case; difference of offsets within bank
	  raw_length = (head.sounds[head_index+1].offset - 
			head.sounds[head_index].offset);
	}
	if (head.sounds[head_index].unknown_32 != BH_UNKNOWN_32_DEFAULT) {
	  output_ini.assign(section + "." + sound_name.str() + ".unknown_32",
			    head.sounds[head_index].unknown_32);
	}
	// Need to save unknown_16, but sample_rate is simply reference
	output_ini.assign(section + "." + sound_name.str() + ".sample_rate",
			  head.sounds[head_index].sample_rate);
	output_ini.assign(section + "." + sound_name.str() + ".unknown_16",
			  head.sounds[head_index].unknown_16);

	// Extracting the sound. First, the sound will be extracted to
	// a buffer in memory, then we will use libsndfile to process it
	// and write it. We are using a lot of code from the 
	// tests/virtual_io_test.c file of the libsndfile distribution.
	// The virtual_file interface should handle this reading itself,
	// but I'm too lazy to implement that currently.
	virtual_file vf_buff(raw_length);
	result = read_chunk(in_fd, vf_buff.data, raw_length);
	if (result <=0 ) {
	  cerr << "Error occurred while reading " << argv[infile] << "; "
	       << read_size << " bytes were read." << endl;
	  return(2);
	}
	read_size += result;
	vf_buff.size = result;

	// Setup sndfile data structures
	SF_VIRTUAL_IO vio;
	vio.get_filelen = vf_get_filelen;
	vio.seek = vf_seek;
	vio.read = vf_read;
	vio.write = vf_write;
	vio.tell = vf_tell;

	SF_INFO sf_info;
	sf_info.format = SF_FORMAT_RAW | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
	sf_info.samplerate = head.sounds[head_index].sample_rate;
	sf_info.channels = 1;

	SNDFILE *sf_in;
	sf_in = sf_open_virtual(&vio, SFM_READ, &sf_info, &vf_buff);

	if (sf_in == NULL) {
	  cerr << "Error reading raw sound from memory: "
	       << sf_strerror(NULL) << endl;
	  return (2);
	}

	// Setup output format and open output file; I'm going to use
	// the file descriptor method for consistency with other code.
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	if (!sf_format_check(&sf_info)) {
	  cerr << "Internal sound conversion error; output format invalid"
	       << endl;
	  return (4);
	}

	ostringstream outfilename;
	outfilename << bank_dir << SLASH << sound_name.str() << ".wav";
	output_ini.assign(section + "." + sound_name.str() + ".filename",
			  section + SLASH + sound_name.str() + ".wav");
	
	int out_fd = open(outfilename.str().c_str(), 
			  O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0644);
	if (out_fd == -1) {
	  cerr << "Error opening output file for " << argv[infile] 
	       << " bank " << bank << ": " << strerror(errno) << endl;
	  return(4);
	}

	SNDFILE *sf_out;
	sf_out = sf_open_fd(out_fd, SFM_WRITE, &sf_info, SF_FALSE);
	if (sf_out == NULL) {
	  cerr << "Error opening output file for " << argv[infile] 
	       << " bank " << bank << ": " << sf_strerror(NULL) << endl;
	  return(4);
	}

	// Convert and write (modified from examples/sndfile-convert.c)
	static short data[SF_FRAME_BUFFER_LEN];
	int frames = SF_FRAME_BUFFER_LEN / sf_info.channels;
	int read_count = frames;
	int write_count = 0;

	while (read_count > 0) {
	  read_count = sf_readf_short(sf_in, data, frames);
	  write_count = sf_writef_short(sf_out, data, read_count);
	  if (write_count != read_count) {
	    cerr << "Error writing output file for " << argv[infile] 
		 << " bank " << bank << ": " << sf_strerror(NULL) << endl;
	    return(4);
	  }
	} 
	
	sf_close(sf_in);
	sf_close(sf_out);
	close(out_fd);
      } // for (sound iteration)
      cout << "done." << endl;
      banklkup.next();
    } // while (bank iteration)
    output_ini.assign("Archive.num_banks", (uint32_t)bank);
    close(in_fd);
    ostringstream outfilename;
    outfilename << export_dir << SLASH << "sfx_import.ini";
    ofstream conf_out(outfilename.str().c_str(), ios::out);
    if (!conf_out) {
      cerr << "Error opening config file " << outfilename.str()
	   << " for output" << endl;
      return(4);
    }
    conf_out << "# sfx_import.ini" << endl << "#" << endl
	     << "# This file is automatically generated by"
	     << " the SAAT sfx exporter. If you edit" << endl
	     << "# it, be sure to save it as a plain text"
	     << " file before trying to re-import it." << endl
	     << "# See the documentation for notes and examples." << endl;
    conf_out << output_ini;
  } // for (archive iteration)
  return(0);
} // sfx_export

int sfx_import (int argc, char **argv) {
  // Just some pointers to make my life easier
  char *arg_archivefile = argv[argc-3];
  char *arg_inifile = argv[argc-2];
  char *arg_lookupfile = argv[argc-1];

  // It is expected that the source files will be in the same directory as
  // the import INI file. So we will try and squeeze a directory out of that.
  // We will do this by stripping the basename off the end of arg_inifile.
  string source_dir = pathname(string(arg_inifile));

  // Read and verify ini file
  cout << "Verifying INI file values...";
  config import_ini;
  if (!populate_conf(import_ini, arg_inifile)) {
    return(16);
  }
  if (!verify_sfx_ini(import_ini, source_dir)) {
    return(16);
  }
  cout << "everything looks OK." << endl;

  int lookup_index = 
    atoi(import_ini.lookup("Archive.lookup_index", "-1").c_str());
  lookup_table banklkup(LT_BANKLKUP);
  if (!populate_lookup(banklkup, arg_lookupfile)) {
    return(16);
  }
  long num_banks = atol(import_ini.lookup("Archive.num_banks", "0").c_str());

  // Backup archive or lookup file?? Not yet.
  int out_fd = open(arg_archivefile, 
		    O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0644);
  if (out_fd == -1) {
    cerr << "Error opening output file " << arg_archivefile 
	 << " for writing: " << strerror(errno) << endl;
    return(4);
  }
  cout << "Creating archive " << arg_archivefile << endl
       << "  from INI file " << arg_inifile << endl
       << "  and lookup file " << arg_lookupfile << endl
       << "Processing Banks (" << num_banks << " total)..." << endl;

  // Clear banklkup and then loop over the banks.
  size_t archive_offset = 0;
  banklkup.clear((uint8_t)lookup_index);
  for (int bank=1; bank<=num_banks ; bank++) {
    ostringstream section;
    section << "Bank_" << setw(3) << setfill('0') << dec << bank;
    long num_sounds = atol(import_ini.lookup(section.str() + 
					     ".num_sounds", "0").c_str());
    cout << "  Bank " << bank << ": Sounds (" << num_sounds 
	 << " total) ..." << flush;

    // Initialize bank-related variables
    size_t bank_offset = 0;
    bank_header head;
    memset(&head,0,sizeof(bank_header));
    head.num_sounds = (uint32_t)num_sounds;

    // Okay, here's a problem. The bank header must now be written, but
    // this header contains offsets and sample rates of sounds that are
    // yet to be processed. So, we will write out a "blank" header, do
    // the sound processing and writing, then seek back here and update
    // the header when the sounds are done.
    size_t head_size = sizeof(bank_header);
    size_t written_size = 0;
    while (written_size < head_size) {
      int result = write(out_fd, &head, head_size - written_size);
      if (result == -1 && errno == EINTR)
	continue;
      if (result == -1) {
	cerr << "Error writing to output file for bank " 
	     << bank << ": " << strerror(errno) << endl;
	return(4);
      }
      written_size += result;
    }
    size_t bank_header_position = archive_offset;
    archive_offset += written_size;

    for (int sound=1; sound<=num_sounds ; sound++) {
      cout << sound << "..." << flush;
      int head_index = sound-1;
      ostringstream key;
      key << "sound_" << setw(3) << setfill('0') << dec << sound;
      string filename = import_ini.lookup(section.str() + "." +
					  key.str() + ".filename", "");
      string infile;
      if (is_absolute_pathname(filename)) {
	infile = filename;
      } else {
	infile = source_dir + filename;
      }
      struct stat stat_buffer;
      int result = stat(infile.c_str(), &stat_buffer);
      if (result == -1) {
	cerr << "Error accessing input file " << infile << ": "
	     << strerror(errno) << "Aborting import." << endl;
	return(2);
      }
      // size_t infile_length = stat_buffer.st_size;
      
      head.sounds[head_index].offset = bank_offset;
      head.sounds[head_index].unknown_32 = 
	(int32_t)atol(import_ini.lookup(section.str() + "." + key.str() + 
					".unknown_32", 
					BH_UNKNOWN_32_DEFAULT_STR).c_str());
      head.sounds[head_index].unknown_16 = 
	(uint16_t)atoi(import_ini.lookup(section.str() + "." + key.str() + 
					 ".unknown_16", 
					 BH_UNKNOWN_16_DEFAULT_STR).c_str());

      // Open the input file
      int in_fd = open(infile.c_str(), O_RDONLY | O_BINARY);
      if (in_fd == -1) {
	cerr << "Error opening input file " << infile << ": "
	     << strerror(errno) << endl;
	return(2);
      }

#ifdef POSIX_FADVISE
    //Advise the kernel not to cache this data, since we will only read it 
    //once. Since this is only a hint, we don't care if it succeeds or not.
    posix_fadvise(in_fd, 0, 0, POSIX_FADV_NOREUSE);
#endif

      SF_INFO sf_info;
      memset(&sf_info, 0, sizeof(SF_INFO)); // zero out sf_info
      SNDFILE *sf_in;
      // sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
      sf_in = sf_open_fd(in_fd, SFM_READ, &sf_info, SF_FALSE);
      if (sf_in == NULL) {
	cerr << "Error accessing input file " << infile
	     << "with libsndfile: " << sf_strerror(NULL) << endl;
	return(4);
      }
      head.sounds[head_index].sample_rate = sf_info.samplerate;

      // Setup the conversion specification
      sf_info.format = SF_FORMAT_RAW | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
      sf_info.channels = 1;
      if (!sf_format_check(&sf_info)) {
	cerr << "Internal sound conversion error; output format invalid"
	     << endl;
	return (4);
      }

      // We need to create a buffer to hold the sound after it is converted
      // to 16-bit PCM mono RAW. Unfortunately, the length of the input file
      // is only useful if it is the expected 16-bit PCM mono WAV. Since I
      // intend to allow multiple input types (eventually) the input file
      // might be compressed and so the buffer will be much larger. I will
      // therefore arbitrarily choose a large size like 5MB and use that.
      // The actual size will be defined in saat_sfx.h as SAAT_MAX_FILE_IMPORT
      virtual_file vf_buff;

      // Setup sndfile data structures
      SF_VIRTUAL_IO vio;
      vio.get_filelen = vf_get_filelen;
      vio.seek = vf_seek;
      vio.read = vf_read;
      vio.write = vf_write;
      vio.tell = vf_tell;

      SNDFILE *sf_out;
      sf_out = sf_open_virtual(&vio, SFM_WRITE, &sf_info, &vf_buff);
      if (sf_out == NULL) {
	cerr << "Error accessing memory buffer for sound conversion: "
	     << sf_strerror(NULL) << endl;
	return (2);
      }

      // Convert and write (modified from examples/sndfile-convert.c)
      static short data[SF_FRAME_BUFFER_LEN];
      int frames = SF_FRAME_BUFFER_LEN / sf_info.channels;
      int read_count = frames;
      int write_count = 0;

      while (read_count > 0) {
	read_count = sf_readf_short(sf_in, data, frames);
	write_count = sf_writef_short(sf_out, data, read_count);
	if (write_count != read_count) {
	  cerr << "Error writing to output memory buffer for " << infile 
	       << ": " << sf_strerror(NULL) << endl;
	  return(4);
	}
      } 

      // This should be implemented in virtual_file too
      size_t out_length = vf_buff.size;
      written_size = 0;
      while (written_size < out_length) {
	result = write(out_fd, vf_buff.data, out_length - written_size);
	if (result == -1 && errno == EINTR)
	  continue;
	if (result == -1) {
	  cerr << "Error writing to output file for bank " << bank 
	       << ", sound " << sound << ": " << strerror(errno) << endl;
	  return(4);
	}
	written_size += result;
      }
      bank_offset += written_size;
      archive_offset += written_size;

      sf_close(sf_in);
      sf_close(sf_out);
      close(in_fd);
    } // for (sound iteration)
    cout << "done." << endl;

    // Sounds are done so now we can add the length of the bank to the
    // lookup file and then go back and update the bank header.
    // bank_header_position is the preserved starting offset and
    // bank_offset now contains the total length of the sounds in the bank
    lookup_entry entry(LT_BANKLKUP, lookup_index, 
		       bank_header_position, bank_offset);
    banklkup.insert(entry);

    int result = lseek(out_fd, bank_header_position, SEEK_SET);
    if (result != (int)bank_header_position) {
      cerr << "Seek error attempting to update bank header for bank "
	   << bank << ": " << strerror(errno) << endl;
    }
    written_size = 0;
    while (written_size < head_size) {
      result = write(out_fd, &head, head_size - written_size);
      if (result == -1 && errno == EINTR)
	continue;
      if (result == -1) {
	cerr << "Error writing to output file on updating header for bank " 
	     << bank << ": " << strerror(errno) << endl;
	return(4);
      }
      written_size += result;
    }
    result = lseek(out_fd, 0, SEEK_END);
    if (result != (int)archive_offset) {
      cerr << "Seek error attempting to return to end of file after" << endl
	   << "updating bank header for bank "
	   << bank << ": " << strerror(errno) << endl;
    }
  } // for (bank iteration)
  close(out_fd);
  cout << "Updating Banklkup file " << arg_lookupfile << "...";
  // Write out updated lookup file
  ofstream lookup_out(arg_lookupfile, ios::out|ios::binary);
  if (!lookup_out) {
    cerr << "Error opening lookup file " << arg_lookupfile
	 << " for output" << endl;
    return(4);
  }
  lookup_out << banklkup;

  cout << "done." << endl << "Import completed successfully." << endl;
  return 0;
} // sfx_import

// Returns index from filename string; note that a truly robust modding
// tool would not assume these values but would instead read the pakfiles.dat
// file itself and use the values there. Because I can't imagine anyone
// actually modifiying that file in order to change the filenames, this will
// not be a truly robust implementation and will hardcode the indices.
// Note that there is no index #2; the filename entry is blank in the
// original game files and thus there is no corresponding file for it.
int get_lookup_index(const string filename) {
  // Just to be extra safe, I will explicitly allcaps this here even though
  // my SAAT code will allcaps the name before calling this function.
  string ACfilename = allcaps(filename);
  if (ACfilename == "FEET")
    return(0);
  if (ACfilename == "GENRL")
    return(1);
  if (ACfilename == "PAIN_A")
    return(2);
  if (ACfilename == "SCRIPT")
    return(3);
  if (ACfilename == "SPC_EA")
    return(4);
  if (ACfilename == "SPC_FA")
    return(5);
  if (ACfilename == "SPC_GA")
    return(6);
  if (ACfilename == "SPC_NA")
    return(7);
  if (ACfilename == "SPC_PA")
    return(8);
  // default value; no match
  return -1;
}

bool verify_sfx_ini(config &import_ini, const string source_dir) {
  // Verifies INI is sane; warns and adjusts bad values if it can. If it's not
  // wise to guess an appropriate value, returns false and program will abort.
  // Note, by necessity there is some code duplication between this function
  // and the main import processor, but I wanted to do all the sanity checks 
  // as soon as possible and avoid having to abort the writing of the archive
  // halfway through simply due to an INI typo.
  int lookup_index = 
    atoi(import_ini.lookup("Archive.lookup_index", "-1").c_str());
  if ( (lookup_index < 0) || (lookup_index > 8) ) {
    cerr << "Error: invalid or missing lookup_index (" << lookup_index << ")"
	 << endl 
	 << "[Archive] lookup_index must be an integer from [0-8]." 
	 << endl << "Please edit the import INI file and try again."
	 << endl;
    return(false);
  }

  long num_banks = atol(import_ini.lookup("Archive.num_banks", "0").c_str());
  if ( (num_banks < 1) || (num_banks > BH_NUM_SOUND_ENTRIES) ){
    cerr << "Error: Found invalid number of banks to import (" 
	 << num_banks << ")" << endl
	 << "[Archive] num_banks must be an integer from [1-400]." << endl
	 << "Please edit the import INI file and try again." << endl;
    return(false);
  }

  // Hmmm. Now that I've decided to loop over the banks and sounds in order
  // to verify the existence of the source files, I could someday pass some
  // data structure in here to hold the offsets/lengths and sample rates too.
  // That way I wouldn't have to seek back to write the bank headers. Not 
  // for this version, but this comment will keep the idea around for later.
  for (int bank=1; bank<=num_banks ; bank++) {
    ostringstream section;
    section << "Bank_" << setw(3) << setfill('0') << dec << bank;
    long num_sounds = atol(import_ini.lookup(section.str() + 
					     ".num_sounds", "0").c_str());

    for (int sound=1; sound<=num_sounds ; sound++) {
      // int head_index = sound-1;
      ostringstream key;
      key << "sound_" << setw(3) << setfill('0') << dec << sound;
      string filename = import_ini.lookup(section.str() + "." +
					  key.str() + ".filename", "");
      if (filename == "") {
	cerr << "Error: No filename entry found in INI for [" 
	     << section << "] " << key.str() << " . Aborting import." << endl;
	return(false);
      }
      string infile;
      if (is_absolute_pathname(filename)) {
	infile = filename;
      } else {
	infile = source_dir + filename;
      }
      struct stat stat_buffer;
      int result = stat(infile.c_str(), &stat_buffer);
      if (result == -1) {
	cerr << "Error: Cannot access input file " << infile 
	     << " specified in input INI for [" << section << "] "
	     << key.str() << " :"
	     << strerror(errno) << ". Aborting import." << endl;
	return(false);
      }
    } // for (sound iteration)
  } // for (bank iteration)
  return true;
}
