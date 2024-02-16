/* The San Andreas Audio Toolkit (SAAT): saat_stream.cpp
 * Copyright (C) 2005 Dave Schmitt <pdescobar@comcast.net>
 * Uses and/or modifies some code from Radio Free San Andreas (GNU GPL)
 * Copyright (C) 2005 Ondra Hosek <white.timberwolf@aon.at> 
 * Copyright (C) 2005 Karl-Johan Karlsson <san-andreas@creideiki.user.lysator.liu.se>
 * Some of the RFSA code used here was, in turn, adapted from 
 * vorbis-tools/vorbiscomment (also released under GNU GPL)
 * (c) 2000-2002 Michael Smith <msmith@xiph.org>
 * (c) 2001 Ralph Giles <giles@ashlu.bc.ca>
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
 * SAAT Stream Tools
 * - Exports GTASA audio streams into individual Ogg Vorbis files
 * - Imports individual Ogg Vorbis files into a GTASA audio stream
 * - Also features an RFSA compatible export which tags and names the
 *   exported Ogg Vorbis files descriptively
 *
 * Exit codes (compatible with RFSA; should probably be changed):
 * 128 - command line error - usage information printed
 * 64  - refusing to run on big-endian processor
 * 32  - error adding metadata to output file
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

#include <ogg/ogg.h>
#include <vorbis/codec.h>

#include "saat_common.h"
#include "saat_stream.h"
#include "config.h"
#include "lookup.h"
#include "vcedit.h"
#include "virtual_file.h"

using namespace std;

int main (int argc, char **argv) {
  string exe_name = basename(argv[0]);

  cout << "San Andreas Audio Toolkit (SAAT) Stream Tool " 
       << SAAT_VERSION_STR << "\n" << endl;

  if (is_bigendian()) {
    print_bigendian_msg();
    exit(64);
  }

  // Simple argument type/number checks. Someday maybe use get_opt()
  bool bad_args = 0;
  mode mymode = MODE_UNKNOWN;
  if ( argc < 2 ) {
    bad_args = 1;
  } else if ( ((string)argv[1] == "-r") || ((string)argv[1] == "-R") ) {
    mymode = MODE_EXRFSA;
    if (argc < 5) {
      bad_args = 1;
    }
  } else if ( ((string)argv[1] == "-e") || ((string)argv[1] == "-E") ) {
    mymode = MODE_EXPORT;
    if (argc < 4) {
      bad_args = 1;
    }
  } else if ( ((string)argv[1] == "-i") || ((string)argv[1] == "-I") ) {
    mymode = MODE_IMPORT;
    if (argc != 5) {
      bad_args = 1;
    }
  }
  if ( bad_args || (mymode == MODE_UNKNOWN) ) {
    print_usage(exe_name);
    exit(128);
  }

  if ( (mymode == MODE_EXPORT) || (mymode == MODE_EXRFSA) ) {
    exit(stream_export(mymode, argc, argv));
  } else if (mymode == MODE_IMPORT) {
    exit(stream_import(argc, argv));
  } else {
    // default, should be impossible.
    exit(0);
  }
} // main

void print_usage(const string cmd) {
  cerr << cmd << " converts between GTASA audio streams and "
       << "Ogg Vorbis tracks." << endl
       << "\nUsage varies by intended mode:" << endl
       << endl << cmd 
       << " -e <stream_file(s)> <target_dir>" << endl
       << "  Simple export mode; exports all tracks "
       << "from <stream_file(s)>, storing the Ogg" << endl
       << "  Vorbis files in an appropriately "
       << "named subdirectory of <target_dir>." << endl
       << endl << cmd 
       << " -r <stream_file(s)> <target_dir> <metadata_file>" << endl
       << "  RFSA export mode; like -e but uses "
       << "contents of <metadata_file> for additional" << endl
       << "  descriptive information to tag and "
       << "name the exported files." << endl
       << "  Note: running RFSA mode on modified "
       << "streams could lead to confusion." << endl
       << endl << cmd 
       << " -i <target_stream> <import_ini> <lookup_file>" << endl
       << "  Import mode; creates <target_stream> "
       << "based on the information and filename" << endl
       << "  references in <import_ini>. Also updates "
       << "<lookup_file> to reflect the changes." << endl;
}

int stream_export (const mode mymode, int argc, char **argv) {
  // Read metadata if necessary
  config metadata;
  if (mymode == MODE_EXRFSA) {
    if (!populate_conf(metadata, argv[argc-1])) {
      return(16);
    }
  }

  // Make sure the main target directory is ok.
  int last_arg = 0;
  if (mymode == MODE_EXRFSA) {
    last_arg = argc-2;
  } else {
    last_arg = argc-1;
  }
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
    
  // iterate over input streams, processing each
  for (int infile = 2; infile < last_arg; ++infile) {
    config output_ini;
    string infilename = argv[infile];
    string inbasename = basename(infilename);
    output_ini.assign("Stream.basename", inbasename);
    cout << "Processing file " 
	 << setw((int)ceil(log10((float)(last_arg - 2)))) << infile - 1
	 << " of " << last_arg - 2 << ": " << infilename;
    result = stat(argv[infile], &stat_buffer);
    if (result == -1) {
      cerr << "Error accessing input file " << argv[infile] << ": "
	   << strerror(errno) << endl;
      return(2);
    }
    size_t file_length = stat_buffer.st_size;
    int in_fd = open(argv[infile], O_RDONLY | O_BINARY);
    if (in_fd == -1) {
      cerr << "Error opening input file " << argv[infile] << ": "
	   << strerror(errno) << endl;
      return(2);
    }
    cout << " (length: " << file_length << ")" << endl
	 << "  Reading Tracks..." << flush;
    
    // Potential tagging variables. Due to scope issues and laziness, there
    // are likely to be a bunch of unused variables lying around when not
    // tagging (i.e. simple export mode). This doesn't bother me too much. ;)
    string station;
    string section_name;
    string albumprefix;
    string global_def_artist;
    string default_station = allcaps(inbasename);
    bool add_metadata = false;
    
    // Creating output subdirectory for the stream under target_dir.
    // Here's where RFSA does the md5 fingerprint check. Rather than
    // reading the entire stream a second time to do the check, SAAT
    // will assume the user knows what s/he is doing and will use 
    // the fingerprint of the unmodified stream which has the same name.
    // Nothing should break if the stream is not the original, but the
    // tags will be based on the original tracks so there could be
    // some confusion/inaccuracy.
    if (mymode == MODE_EXRFSA) {
      add_metadata = true;
      // SAAT will allow the INI section names to be the stream name
      // (in ALL CAPS) or the md5 fingerprint to be compatible with RFSA.
      station = metadata.lookup(default_station + ".station", "");
      if (station != "") {
	// Lookup worked, so set section_name and move on.
	section_name = default_station;
      } else {
	// Lookup failed, so we try the fingerprint method instead
	section_name = get_fingerprint(default_station);
	station = metadata.lookup(section_name + ".station", "");
	if (station == "") {
	  // Still failed; warn user and give up.
	  cerr << "   Didn't recognize MD5 hash \"" << section_name
	       << "\" for file " << argv[infile]
	       << " - not adding metadata." << endl;
	  add_metadata = false;
	}
      }
      albumprefix = metadata.lookup("global.albumprefix", "");
      if (albumprefix != "")
	albumprefix += " ";
      global_def_artist = metadata.lookup("global.defaultartist", 
					  "Rockstar North");
    }
    if (station == "") {
      // In simple mode or if there's an INI problem, station is empty
      // Since it will be used as a directory name, it needs a default.
      station = default_station;
    }
    string export_dir = string(target_dir) + SLASH + station;
    if ( (access(export_dir.c_str(), 00) == -1) &&
	 (create_dir(export_dir.c_str()) == -1) ) {
      cerr << "Error creating directory for station " << station
	   << ": " << strerror(errno) << endl;
      return(1);
    }

    // Determine lookup index which is necessary for later importing
    // We will conveniently use the ALL CAPS directory names for this too.
    int lookup_index = get_lookup_index(default_station);
    output_ini.assign("Stream.lookup_index", (int32_t)lookup_index);
    if (lookup_index < 0) {
      cerr << "Warning: Could not match filename " << inbasename 
	   << " to a valid track" << endl 
	   << "lookup index; you must edit the stream_import.ini file" << endl
	   << "to include a valid index in order to reimport these files."
	   << endl;
    }

#ifdef POSIX_FADVISE
    //Advise the kernel not to cache this data, since we will only read it 
    //once. Since this is only a hint, we don't care if it succeeds or not.
    posix_fadvise(in_fd, 0, 0, POSIX_FADV_NOREUSE);
#endif
    
    // Here is one of the major changes from RFSA. RFSA slurps the whole
    // file at once and then does the decryption, fingerprinting, and
    // extraction of the image in memory. Because of limitations on my
    // development machine and the desire for flexibility when this code
    // is partly reused on the much larger sound effects archives, the
    // files will be processed in pieces. This means each track will be
    // read, decrypted, and then output before the next track is read.
    // A result of this is that I have abandoned the md5 fingerprint as a 
    // method of identifying the stream and will resort to a potentially 
    // less reliable method of using the base filename of the stream.
    int encode_index = 0;
    int track_num = 0;
    size_t read_size = 0;
    size_t head_length = sizeof(track_header);
    size_t ogg_length = 0;
    track_header head;

    // Iterating over tracks in the stream
    while (read_size < file_length) {
      ostringstream default_filename;
      default_filename << "Track_" << setw(3) << setfill('0') << ++track_num;
      cout << track_num << "..." << flush;
      string section = default_filename.str();
      
      // Read track header first
      result = read_encoded_chunk(in_fd, &head, head_length, encode_index);
      if (result <=0 ) {
	cerr << "Error occurred while reading " << argv[infile] << "; "
	     << read_size << " bytes were read." << endl;
	return(2);
      }
      read_size += result;
      
      // Processing beat data
      for (int i=0; i<TH_NUM_BEAT_ENTRIES; ++i) {
	if (head.beats[i].timing == TH_BEAT_TIMING_DEFAULT) {
	  //End of beats reached previously (or no beats)
	  output_ini.assign(section + ".beat_total", (uint32_t)i);
	  break;
	} else if (head.beats[i].control == TH_BEAT_CONTROL_END) {
	  //This is the last beat; necessary if all are used
	  output_ini.assign(section + ".beat_total", (uint32_t)(i+1));
	}
	ostringstream keyname;
	keyname << section << ".beat_" << setw(3) << setfill('0') 
		<< dec << i;
	output_ini.assign(keyname.str() + ".timing", head.beats[i].timing);
	output_ini.assign(keyname.str() + ".control", head.beats[i].control);
      }

      // Processing length data and trailer
      for (int i=0; i<TH_NUM_LENGTH_ENTRIES; ++i) {
	if (head.lengths[i].length != TH_LENGTH_DEFAULT) {
	  ogg_length = head.lengths[i].length;
	  output_ini.assign(section + ".length_index", (uint8_t)i);
	  //output_ini.assign(section + ".length_old", (uint32_t)ogg_length);
	  output_ini.assign(section + ".length_extra", 
			    head.lengths[i].extra);
	}
      }
      if (head.trailer != TH_TRAILER_DEFAULT) {
	output_ini.assign(section + ".trailer", (uint32_t)head.trailer);
      }

      // Okay, now the Ogg Vorbis file itself can be processed since
      // it's next in the stream and we (hopefully) know its length
      virtual_file vf_buff(ogg_length);
      result = read_encoded_chunk(in_fd, vf_buff.data, ogg_length,
				  encode_index);
      if (result <=0 ) {
	cerr << "Error occurred while reading " << argv[infile] << "; "
	     << read_size << " bytes were read." << endl;
	return(2);
      }
      read_size += result;
      vf_buff.size = result;

      // Note: Remember to check if mp3s actually work; if so, this sanity
      // check will need to be changed slightly to instead set a variable
      // related to tagging and not return an error.
      // Very basic sanity check of the Ogg Vorbis file
      if (string((char *)vf_buff.data, 4) != "OggS") {
	cerr << "This doesn't look like an Ogg Vorbis stream to me." << endl
	     << "Expected first 4 bytes to be \"0x"
	     << hex
	     << (unsigned int)'O' << (unsigned int)'g'
	     << (unsigned int)'g' << (unsigned int)'S'
	     << "\", found \"0x"
	     << (unsigned int)vf_buff.data[0] << (unsigned int)vf_buff.data[1]
	     << (unsigned int)vf_buff.data[2] << (unsigned int)vf_buff.data[3]
	     << dec << "\"." << endl;
	return(8);
      }

      // Tag it and save it. Note, we use the add_metadata check because
      // there are situations where you can be in RFSA mode but still are 
      // not adding metadata due to some INI issue. This part of the code
      // is almost completely unchanged from RFSA. The only substantive
      // changes are using Ondra's idea of allowing a filename INI entry and
      // removing a variable redundancy.
      if (add_metadata) {
	vcedit_state *state = vcedit_new_state();
	vorbis_comment *vc;
	
	if (vcedit_open_callbacks(state, &vf_buff, &vf_fread,
				  (vcedit_write_func)fwrite) < 0) {
	  cerr << "Error initializing vorbiscomment edit library while "
	       << "adding metadata for "
	       << argv[infile] << " track " << track_num << ": "
	       << vcedit_error(state) << endl;
	  return(32);
	}

	// Note, previous comments are torched here.
	vc = vcedit_comments(state);
	vorbis_comment_clear(vc);
	vorbis_comment_init(vc);

	// ALBUM tag
	vorbis_comment_add_tag(vc, "ALBUM",
			       const_cast<char *>((albumprefix 
						   + station).c_str()));

	// TRACK tag
	ostringstream track_number;
	track_number << track_num;
	vorbis_comment_add_tag(vc, "TRACKNUMBER",
			       const_cast<char *>(track_number.str().c_str()));

	// ARTIST tag is kind of odd; SAAT allows a global .defaultartist,
	// a section/stream .defaultartist and an individual track .artist
	// The global was handled earlier, so we check for the others now.
	string stream_def_artist = metadata.lookup(section_name +
						   ".defaultartist",
						   global_def_artist);
	string artist = metadata.lookup(section_name + ".track" 
					+ track_number.str() + ".artist",
					stream_def_artist);
	vorbis_comment_add_tag(vc, "ARTIST",
			       const_cast<char *>(artist.c_str()));

	// TITLE tag
	string title = metadata.lookup(section_name + ".track" 
				       + track_number.str() + ".title",
				       default_filename.str());
	vorbis_comment_add_tag(vc, "TITLE",
			       const_cast<char *>(title.c_str()));

	// SAAT allows a .filename entry, but defaults to the TITLE tag
	string filename = sanitize_filename(metadata.lookup(section_name + 
							    ".track" + 
							    track_number.str() 
							    + ".filename",
							    title));
	string full_filename = (export_dir + SLASH + filename + ".ogg");
	output_ini.assign(section + ".filename", basename(full_filename));

	FILE *taggedfile = fopen(full_filename.c_str(), "wb");
	if (!taggedfile) {
	  cerr << "Error creating tagged file for " << argv[infile]
	       << " track " << track_num << ": "
	       << strerror(errno) << endl;
	  return(32);
	}
	if (vcedit_write(state, taggedfile) < 0) {
	  cerr << "Error writing tagged file for " << argv[infile]
	       << " track " << track_num << ": "
	       << vcedit_error(state) << endl;
	  return(32);
	}
	if (fclose(taggedfile) == -1) {
	  cerr << "Error closing output file for " << argv[infile]
	       << " track " << track_num << ": "
	       << strerror(errno) << endl;
	  return(32);
	}
	vorbis_comment_clear(vc);
	vcedit_clear(state);
      } else {
	//Not adding metadata
	ostringstream outfilename;
	outfilename << export_dir << SLASH 
		    << default_filename.str() << ".ogg";
	output_ini.assign(section + ".filename", 
			  basename(outfilename.str()));
	
	int out_fd = open(outfilename.str().c_str(), 
			  O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0644);
	if (out_fd == -1) {
	  cerr << "Error opening output file for " << argv[infile] 
	       << " track " << track_num << ": " << strerror(errno) << endl;
	  return(4);
	}

	size_t written_size = 0;
	uint8_t *write_pointer = vf_buff.data;
	while (written_size < ogg_length) {
	  result = write(out_fd, write_pointer, ogg_length - written_size);
	  if (result == -1 && errno == EINTR)
	    continue;
	  if (result == -1) {
	    cerr << "Error writing to output file for " << argv[infile] 
		 << " track " << track_num << ": " << strerror(errno) 
		 << endl;
	    return(4);
	  }
	  written_size += result;
	  write_pointer += result;
	}
	close(out_fd);
      } // else (not adding metadata)
    } // while (reading track)
    close(in_fd);
    output_ini.assign("Stream.num_tracks", (uint32_t)track_num);
    ostringstream outfilename;
    outfilename << export_dir << SLASH << "stream_import.ini";
    ofstream conf_out(outfilename.str().c_str(), ios::out);
    if (!conf_out) {
      cerr << "Error opening config file " << outfilename.str()
	   << " for output" << endl;
      return(4);
    }
    conf_out << "# stream_import.ini" << endl << "#" << endl
	     << "# This file is automatically generated by"
	     << " the SAAT stream exporter. If you edit" << endl
	     << "# it, be sure to save it as a plain text"
	     << " file before trying to re-import it." << endl
	     << "# See the documentation for notes and examples." << endl;
    conf_out << output_ini;
    cout << "done." << endl;
  } // for (reading stream)
  return 0;
} // stream_export

int stream_import (int argc, char **argv) {
  // Just some pointers to make my life easier
  char *arg_streamfile = argv[argc-3];
  char *arg_inifile = argv[argc-2];
  char *arg_lookupfile = argv[argc-1];

  // It is expected that the source files will be in the same directory as
  // the import INI file. So we will try and squeeze a directory out of that.
  // We will do this by stripping the basename off the end of arg_inifile.
  string source_dir = pathname(string(arg_inifile));

  // Read and verify ini file
  config import_ini;
  if (!populate_conf(import_ini, arg_inifile)) {
    return(16);
  }
  if (!verify_stream_ini(import_ini, source_dir)) {
    return(16);
  }
  
  int lookup_index = 
    atoi(import_ini.lookup("Stream.lookup_index", "-1").c_str());
  lookup_table traklkup(LT_TRAKLKUP);
  if (!populate_lookup(traklkup, arg_lookupfile)) {
    return(16);
  }
  long num_tracks = atol(import_ini.lookup("Stream.num_tracks", "0").c_str());

  // Backup stream or lookup file?? Not yet.
  int out_fd = open(arg_streamfile, 
		    O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0644);
  if (out_fd == -1) {
    cerr << "Error opening output file " << arg_streamfile 
	 << " for writing: " << strerror(errno) << endl;
    return(4);
  }

  cout << "Creating stream " << arg_streamfile << endl
       << "  from INI file " << arg_inifile << endl
       << "  and lookup file " << arg_lookupfile << endl
       << "Processing Tracks (" << num_tracks << " total)..." << flush;
  // Clear traklkup and then loop over the tracks.
  int encode_index = 0;
  size_t stream_offset = 0;
  traklkup.clear((uint8_t)lookup_index);
  for (int track=1; track<=num_tracks ; track++) {
    cout << track << "..." << flush;
    ostringstream section;
    section << "Track_" << setw(3) << setfill('0') << dec << track;
    string filename = import_ini.lookup(section.str() + ".filename", "");
    string infile;
    if (is_absolute_pathname(filename)) {
      infile = filename;
    } else {
      infile = source_dir + filename;
    }

    // We can't write the header or lookup entry until we process the file, 
    // because they both contain the length of the embedded track and that 
    // length can't be determined until the comments are stripped. So, we'll 
    // actually process the file now, then we'll process and write the header,
    // and then finally write the track. It's a bit confusing, but it works. 
    // We also have to save the starting offset for the lookup entry.
    size_t track_header_position = stream_offset;

    // Note, file should be accessible because the verifier stat-ed it.
    // Read, strip, and encode the actual track. 
    FILE *in_fp = fopen(infile.c_str(), "rb");
    if (in_fp == NULL) {
      cerr << "Error opening input file " << infile << ": "
	   << strerror(errno) << endl;
      return(2);
    }

#ifdef POSIX_FADVISE
    int in_fd = fileno(in_fp);
    //Advise the kernel not to cache this data, since we will only read it 
    //once. Since this is only a hint, we don't care if it succeeds or not.
    posix_fadvise(in_fd, 0, 0, POSIX_FADV_NOREUSE);
#endif

    // We'll let vcedit do the reading of the source file and then it'll
    // write out to our virtual file.
    virtual_file vf_buff(0);
    vcedit_state *state = vcedit_new_state();
    vorbis_comment *vc;
	
    if (vcedit_open_callbacks(state, in_fp, (vcedit_read_func)fread,
			      &vf_fwrite) < 0) {
      cerr << "Error initializing vorbiscomment edit library while "
	   << "stripping comments from " << infile << ": "
	   << vcedit_error(state) << endl;
      return(32);
    }

    vc = vcedit_comments(state);
    vorbis_comment_clear(vc);
    vorbis_comment_init(vc);
    if (vcedit_write(state, (void *)&vf_buff) < 0) {
      cerr << "Error writing stripped input file " << infile 
	   << " to memory buffer: " << vcedit_error(state) << endl;
      return(2);
    }
    vcedit_clear(state);
    fclose(in_fp);

    // If by some miracle the above actually worked, the virtual file now
    // contains the stripped Ogg Vorbis file with its size parameter set
    // properly. We'll now deal with the header creation and processing.
    track_header head;
    long beat_total = atol(import_ini.lookup(section.str() + ".beat_total", 
					    "0").c_str());
    // We will only lookup beat information up to beat_total; everything
    // after that will be padding with the default values.
    for (int i=0 ; i<TH_NUM_BEAT_ENTRIES ; i++) {
      string timing_str;
      string control_str;
      if (i < beat_total) {
	ostringstream key;
	key << section.str() << ".beat_" 
	    << setw(3) << setfill('0') << dec << i;
	timing_str = import_ini.lookup(key.str() + ".timing",
				       TH_BEAT_TIMING_DEFAULT_STR);
	control_str = import_ini.lookup(key.str() + ".control", 
					TH_BEAT_CONTROL_DEFAULT_STR);
      } else {
	timing_str = TH_BEAT_TIMING_DEFAULT_STR;
	control_str = TH_BEAT_CONTROL_DEFAULT_STR;
      }
      head.beats[i].timing = (int32_t)atol(timing_str.c_str());
      head.beats[i].control = (int32_t)atol(control_str.c_str());
    }
    int length_index = atoi(import_ini.lookup(section.str() + ".length_index", 
					      "0").c_str());
    if ( (length_index < 0) || (length_index >= TH_NUM_LENGTH_ENTRIES) ) {
      // more error handling needed
      length_index = 0;
    }
    for (int i=0 ; i<TH_NUM_LENGTH_ENTRIES ; i++) {
      if (i == length_index) {
	head.lengths[i].length = vf_buff.size;
	head.lengths[i].extra = 
	  (uint32_t)atol(import_ini.lookup(section.str() + ".length_extra", 
					   "0").c_str());
      } else {
	head.lengths[i].length = TH_LENGTH_DEFAULT;
	head.lengths[i].extra = TH_LENGTH_DEFAULT;
      }
    }
    string trailer_str = import_ini.lookup(section.str() + ".trailer", "");
    if (trailer_str == "") {
      head.trailer = TH_TRAILER_DEFAULT;
    }

    // Encode and write header data
    size_t head_size = sizeof(track_header);
    stream_encode((int8_t *)&head, head_size, encode_index);
    size_t written_size = 0;
    int result = 0;
    while (written_size < head_size) {
      result = write(out_fd, &head, head_size - written_size);
      if (result == -1 && errno == EINTR)
	continue;
      if (result == -1) {
	cerr << "Error writing to output file for track " 
	     << track << ": " << strerror(errno) << endl;
	return(4);
      }
      written_size += result;
    }
    stream_offset += head_size;

    // Encode and write the stripped Ogg Vorbis file
    stream_encode((int8_t *)vf_buff.data, vf_buff.size, encode_index);
    written_size = 0;
    while (written_size < vf_buff.size) {
      result = write(out_fd, vf_buff.data, vf_buff.size - written_size);
      if (result == -1 && errno == EINTR)
	continue;
      if (result == -1) {
	cerr << "Error writing to output file for track " 
	     << track << ": " << strerror(errno) << endl;
	return(4);
      }
      written_size += result;
    }
    stream_offset += written_size;

    // Finally, insert lookup entry with the saved position and written size
    lookup_entry entry(LT_TRAKLKUP, lookup_index, 
		       track_header_position, written_size);
    traklkup.insert(entry);
  } // for (track iteration)
  close(out_fd);
  cout << "done." << endl 
       << "Updating TrakLkup file " << arg_lookupfile << "...";
  // Write out updated lookup file
  ofstream lookup_out(arg_lookupfile, ios::out|ios::binary);
  if (!lookup_out) {
    cerr << "Error opening lookup file " << arg_lookupfile
	 << " for output" << endl;
    return(4);
  }
  lookup_out << traklkup;

  cout << "done." << endl << "Import completed successfully." << endl;
  return 0;
} // stream_import

// Read part of file and encode
ssize_t read_encoded_chunk(const int fd, void *buff, const size_t size, 
			   int &index) {
  int result = read_chunk(fd, buff, size);
  if (result <= 0) {
    // Something bad happened, abort
    return result;
  }
  // result must now be >0, so I can cast it to size_t w/o worry.
  if ((size_t)result != size) {
    // Something impossible happened.
    cerr << "Warning: size mismatch in read_encoded_chunk: "
	 << result << " != " << size << endl;
  }
  stream_encode((int8_t *)buff, result, index);
  return result;
}

// Encoder/Decoder
void stream_encode(int8_t *buff, const size_t size, int &index) {
  for (size_t i = 0; i < size; ++i) {
    buff[i] ^= encode_key[index];
    index = (index + 1) % 16;
  }
  return;
}

// Map filenames to md5 fingerprints for tagging in RFSA mode. See comments 
// in saat_stream.cpp for why md5 fingerprinting isn't being used. The only 
// reason for this is to maintain compatibility with the RFSA metadata.conf
string get_fingerprint(const string filename) {
  // Just to be extra safe, I will explicitly allcaps this here even though
  // my SAAT code will allcaps the name before calling this function.
  string ACfilename = allcaps(filename);
  if (ACfilename == "AA")
    return("8a388107cdf3934b3f7a3f1e2b33199c");
  if (ACfilename == "ADVERTS")
    return("9b7310981c3e88d2d9eda9b18b443f75");
  if (ACfilename == "AMBIENCE")
    return("8bbc634dc3495b74d9cc15ab179a836b");
  if (ACfilename == "BEATS")
    return("fa900b4aca54c5a6c419fda136374f6a");
  if (ACfilename == "CH")
    return("a88eb0130acb81b5956e412db06ea9ce");
  if (ACfilename == "CO")
    return("c2075553325e7fe8d3d50fefff857aa7");
  if (ACfilename == "CR")
    return("06a63d4222209a1d6e8ce2e7ace51956");
  if (ACfilename == "CUTSCENE")
    return("d4bcb8ae560e44080169e2b4a5ad03b8");
  if (ACfilename == "DS")
    return("bbf21458f45989c1edaab9a61b13878b");
  if (ACfilename == "HC")
    return("2c42bd257849aa8fddb23a70785b5e8c");
  if (ACfilename == "MH")
    return("fcc7b4550abb177ea52aee563e562be5");
  if (ACfilename == "MR")
    return("4fd83ffca7302ef1d952ba686c0cae32");
  if (ACfilename == "NJ")
    return("80b7cf701f555fc567e1254e7734cb0f");
  if (ACfilename == "RE")
    return("65e0bea63444d87f74f6ef25388310b7");
  if (ACfilename == "RG")
    return("743b480e6182f0a7795cddb2172cf45f");
  if (ACfilename == "TK")
    return("cda6d4299457eff44b33a7f1cb1d2a72");
  // default value; no match
  return "";
}

// Returns index from filename string; note that a truly robust modding
// tool would not assume these values but would instead read the strmpaks.dat
// file itself and use the values there. Because I can't imagine anyone
// actually modifiying that file in order to change the filenames, this will
// not be a truly robust implementation and will hardcode the indices.
// Note that there is no index #2; the filename entry is blank in the
// original game files and thus there is no corresponding file for it.
int get_lookup_index(const string filename) {
  // Just to be extra safe, I will explicitly allcaps this here even though
  // my SAAT code will allcaps the name before calling this function.
  string ACfilename = allcaps(filename);
  if (ACfilename == "AA")
    return(0);
  if (ACfilename == "ADVERTS")
    return(1);
  if (ACfilename == "AMBIENCE")
    return(3);
  if (ACfilename == "BEATS")
    return(4);
  if (ACfilename == "CH")
    return(5);
  if (ACfilename == "CO")
    return(6);
  if (ACfilename == "CR")
    return(7);
  if (ACfilename == "CUTSCENE")
    return(8);
  if (ACfilename == "DS")
    return(9);
  if (ACfilename == "HC")
    return(10);
  if (ACfilename == "MH")
    return(11);
  if (ACfilename == "MR")
    return(12);
  if (ACfilename == "NJ")
    return(13);
  if (ACfilename == "RE")
    return(14);
  if (ACfilename == "RG")
    return(15);
  if (ACfilename == "TK")
    return(16);
  // default value; no match
  return -1;
}

bool verify_stream_ini(config &import_ini, const string source_dir) {
  // Verifies INI is sane; warns and adjusts bad values if it can. If it's not
  // wise to guess an appropriate value, returns false and program will abort.
  // Note, by necessity there is some code duplication between this function
  // and the main import processor, but I wanted to do all the sanity checks 
  // as soon as possible and avoid having to abort the writing of the archive
  // halfway through simply due to an INI typo.
  int lookup_index = 
    atoi(import_ini.lookup("Stream.lookup_index", "-1").c_str());
  if ( (lookup_index < 0) || (lookup_index == 2) || (lookup_index > 16) ) {
    cerr << "Error: invalid or missing lookup_index (" << lookup_index << ")"
	 << endl 
	 << "[Stream] lookup_index must be an integer from [0-1] or [3-16]." 
	 << endl << "Please edit the import INI file and try again."
	 << endl;
    return(false);
  }

  long num_tracks = atol(import_ini.lookup("Stream.num_tracks", "0").c_str());
  if (num_tracks < 1) {
    cerr << "Error: number of tracks to import is less than 1 (" 
	 << num_tracks << ")" << endl
	 << "[Stream] num_tracks must be a positive integer." << endl 
	 << "Please edit the import INI file and try again." << endl;
    return(false);
  }

  for (int track=1; track<=num_tracks ; track++) {
    ostringstream section;
    section << "Track_" << setw(3) << setfill('0') << dec << track;
    string filename = import_ini.lookup(section.str() + ".filename", "");
    if (filename == "") {
      cerr << "Error: No filename entry found in INI for [" 
	   << section.str() << "]. Aborting import." << endl;
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
	   << " specified in input INI for [" << section.str() << "]: "
	   << strerror(errno) << ". Aborting import." << endl;
      return(false);
    }

    long beat_total = atol(import_ini.lookup(section.str() + ".beat_total", 
					    "0").c_str());
    int32_t last_timing = 0;
    int32_t last_control = TH_BEAT_CONTROL_END;
    int last_beat = -1;
    // Strict error-checking will make sure the timings are in order.
    for (int i=0 ; i<TH_NUM_BEAT_ENTRIES ; i++) {
      ostringstream key;
      key << section.str() << ".beat_" << setw(3) << setfill('0') << dec << i;
      string timing_str = import_ini.lookup(key.str() + ".timing",
					    TH_BEAT_TIMING_DEFAULT_STR);
      int32_t this_timing = (int32_t)atol(timing_str.c_str());
      string control_str = import_ini.lookup(key.str() + ".control", 
					     TH_BEAT_CONTROL_DEFAULT_STR);
      int32_t this_control = (int32_t)atol(control_str.c_str());

      if (this_timing == TH_BEAT_TIMING_DEFAULT) {
	if (last_control != TH_BEAT_CONTROL_END) {
	  // An empty timing was found but beats hadn't ended. End them now.
	  cerr << "Warning: In [" << section.str() << "] beat_"
	       << setw(3) << setfill('0') << dec << i << ".timing is "
	       << TH_BEAT_TIMING_DEFAULT 
	       << " but previous beat control was not the end token ("
	       << TH_BEAT_CONTROL_END << "). Forcing end token at beat_"
	       << setw(3) << setfill('0') << dec << i-1 << ".control" << endl;
	  ostringstream last_key;
	  last_key << section.str() << ".beat_" 
		   << setw(3) << setfill('0') << dec << i-1;
	  import_ini.assign(last_key.str() + ".control", TH_BEAT_CONTROL_END);
	  last_beat = i-1;
	  break;
	}
	// Beats ended properly, abandoning further checking.
	break;
      }

      if (this_timing <= last_timing) {
	// Error, going back in time. Make this timing be last + 1.
	cerr << "Warning: In [" << section.str() << "] beat_"
	     << setw(3) << setfill('0') << dec << i << ".timing ("
	     << this_timing << ") is less than or equal to the previous"
	     << " timing value ("
	     << last_timing << "). Forcing this timing to be 1ms later ("
	     << last_timing + 1 << ")." << endl;
	this_timing = last_timing + 1;
	import_ini.assign(key.str() + ".timing", this_timing);
      }

      // Make sure this_control is valid; we are more strict than the game.
      // Valid values are [0x01-0x04],[0x09-0x10],[0x21==TH_BEAT_CONTROL_END]
      if ( (this_control < 0x01) ||
	   ((this_control > 0x04) && (this_control < 0x09)) ||
	   ((this_control > 0x10) && (this_control < 0x21)) ||
	   (this_control > 0x21) ) {
	cerr << "Warning: In [" << section.str() << "] beat_"
	     << setw(3) << setfill('0') << dec << i << ".control ("
	     << this_control << ") is invalid. Valid controls are" << endl
	     << "1-4 inclusive for dances, 9-16 inclusive for low-riders, "
	     << "or 33 to mark the end of the beats." << endl
	     << "Forcing the end of the beats at this point." << endl;
	import_ini.assign(key.str() + ".control", TH_BEAT_CONTROL_END);
      }

      if (this_control == TH_BEAT_CONTROL_END) {
	last_beat = i;
	break;
      }

      last_timing = this_timing;
      last_control = this_control;
    }
    if ( beat_total < 0 ) {
      // Beat total is screwed up; set it to last_beat + 1;
      cerr << "Warning: In [" << section.str() << "] beat_total ("
	   << beat_total << ") is negative. Setting it to ("
	   << last_beat + 1 << ") based on the actual beat values." << endl;
      import_ini.assign(section.str() + ".beat_total", 
			int32_t(last_beat + 1));
    }
    if ( beat_total < (last_beat + 1) ){
      // The beats should cut off earlier; warn user about it, but there's
      // no need to adjust the ini's actual beat values since the import
      // processor will believe beat_total and ignore beats beyond it.
      cerr << "Warning: In [" << section.str() << "] beat_total is "
	   << beat_total << " but there are more than "
	   << beat_total << " beat values defined." << endl
	   << "Program will ignore these extra values and will "
	   << "only assign " << beat_total << " beats." << endl;
    }
    if ( beat_total > (last_beat + 1) ){
      // The beats should cut off earlier; change beat_total
      cerr << "Warning: In [" << section.str() << "] beat_total is " 
	   << beat_total << " but there is an end token before that at beat_"
	   << setw(3) << setfill('0') << dec << last_beat << ".control" 
	   << endl << "Setting beat_total to " << last_beat + 1
	   << " instead." << endl;
      import_ini.assign(section.str() + ".beat_total", 
			int32_t(last_beat + 1));
    }

    int length_index = atoi(import_ini.lookup(section.str() + ".length_index", 
					      "0").c_str());
    if ( (length_index < 0) || (length_index >= TH_NUM_LENGTH_ENTRIES) ) {
      // In the original streams, tracks with beat info have their length
      // listed at index 1 while those without beat info use index 0.
      cerr << "Warning: In [" << section.str() << "] length_index ("
	   << length_index << ") is invalid. "
	   << "This index must be in the range 0-"
	   << TH_NUM_LENGTH_ENTRIES << " inclusive." << endl;
      if (beat_total > 0) {
	length_index = 1;
      } else {
	length_index = 0;
      }
      cerr << "Program will set index to " << length_index << "." << endl;
      import_ini.assign(section.str() + "length_index", (int32_t)length_index);
    }
  }
  return(true);
}
