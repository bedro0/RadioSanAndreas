
                      The San Andreas Audio Toolkit (SAAT)

Current Version: 1.10

   The San Andreas Audio Toolkit (SAAT) is a set of commandine tools
   useful for modding the PC version of the video game Grand Theft Auto:
   San Andreas. SAAT allows the importing of music files into SA audio
   streams and the importing of WAVe files into SA sfx archives. It will
   also necessarily allow exporting from each type of archive into
   individual files.

   SAAT was written by Dave Schmitt (AKA "P.D. Escobar") in C++; it is an
   open-source program released under the [1]GNU General Public License.
   It was based on the open-source program [2]Radio Free San Andreas and
   uses several open-source libraries: [3]libogg and libvorbis for Ogg
   Vorbis processing and [4]libsndfile for other sound processing.

Resources

  Download Locations for Version 1.10

   SAAT is distributed in two primary locations.
     * From the official website:
       [5]http://pdescobar.home.comcast.net/gta/saat/SAAT_release_1_10.zip
     * From GTA Garage: [6]http://www.gtagarage.com/mods/show.php?id=1186

  Official Website

   The official SAAT website is
   [7]http://pdescobar.home.comcast.net/gta/saat/. It contains the most
   recent version of this README file, other documentation, and the
   distribution downloads.

  Feedback

   The main place for discussion and feedback on SAAT is its GTA Forums
   thread: [8]http://www.gtaforums.com/index.php?showtopic=225049. You may
   also contact the author directly via email at <pdescobar@comcast.net>.

Documentation

   Here's a very brief overview of SAAT. For more detailed documentation
   with some usage examples, see [9]MANUAL.txt; for a list of changes in
   this version, see [10]CHANGES.txt.

  Installation (Windows Binaries):

    1. Unzip the two executables and all ini files into a useful location
       such as the GTASA main directory.
    2. Open a command prompt window and change to the install directory.

  Usage (stream tool):

saat_stream.exe -e <stream_file(s)> <target_dir>
  Simple export mode; exports all tracks from <stream_file(s)>, storing the Ogg
  Vorbis files in an appropriately named subdirectory of <target_dir>.

saat_stream.exe -r <stream_file(s)> <target_dir> <metadata_file>
  RFSA export mode; like -e but uses contents of <metadata_file> for additional
  descriptive information to tag and name the exported files.
  Note: running RFSA mode on modified streams could lead to confusion.

saat_stream.exe -i <target_stream> <import_ini> <lookup_file>
  Import mode; creates <target_stream> based on the information and filename
  references in <import_ini>. Also updates <lookup_file> to reflect the changes

  Usage (sfx tool):

saat_sfx.exe -e <archive_file(s)> <target_dir> <lookup_file>
  Export mode; exports all sounds from <archive_file(s)>, storing the WAVe
  files in an appropriately named subdirectory of <target_dir>.
  Uses <lookup_file> to help locate sounds within the archive.

saat_sfx.exe -i <target_archive> <import_ini> <lookup_file>
  Import mode; creates <target_archive> based on the information and filename
  references in <import_ini>. Also updates <lookup_file> to reflect the changes

Acknowledgements

   The following people and projects have made major contributions to
   SAAT, whether they know it or not:
     * SAAT is based on the stream export program [11]Radio Free San
       Andreas (RFSA) which was released under the GNU GPL. The primary
       author of RFSA is Karl-Johan Karlsson; additionally, code written
       for RFSA by Marcus Eriksson was adapted for use with SAAT.
     * SAAT incorporates several of the changes to RFSA made by Ondra
       Hosek in the [12]RFSA for 32-bit Windows release.
     * SAAT reuses RFSA's Ogg Vorbis commenting routines which originated
       with the vorbiscomment program from the [13]vorbis-tools
       distribution; the authors of these routines are Michael Smith and
       Ralph Giles.
     * SAAT uses some of the example code from the [14]libsndfile
       distribution. Both that code and the library itself are written by
       Eric de Castro Lopo.
     * Since SAAT must both decode and encode streams, it builds on the
       work of Simon Elén who, according to RFSA documentation, was the
       first to break Rockstar's stream encoding.
     * SAAT has modified and supplemented the stream metadata supplied
       with RFSA; much of this metadata originated with [15]San Andreas
       Radio by Eddie Edwards.
     * SAAT's sfx processing was aided by the posting of the sfx bank
       header format to [16]GTA Forums by [17]Steve M.

Licensing

   (Please see [18]COPYING.txt for full details.)

   The San Andreas Audio Toolkit is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The San Andreas Audio Toolkit is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the San Andreas Audio Toolkit; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
   USA

References

   1. http://www.gnu.org/licenses/licenses.html#GPL
   2. http://www.lysator.liu.se/~creideiki/radio-free-san-andreas/
   3. http://www.xiph.org/downloads/
   4. http://www.mega-nerd.com/libsndfile/
   5. http://pdescobar.home.comcast.net/gta/saat/SAAT_release_1_10.zip
   6. http://www.gtagarage.com/mods/show.php?id=1186
   7. http://pdescobar.home.comcast.net/gta/saat/
   8. http://www.gtaforums.com/index.php?showtopic=225049
   9. file://./MANUAL.txt
  10. file://./CHANGES.txt
  11. http://www.lysator.liu.se/~creideiki/radio-free-san-andreas/
  12. http://ondrasplayground.on.funpic.de/
  13. http://www.xiph.org/downloads/
  14. http://www.mega-nerd.com/libsndfile/
  15. http://www.tinyted.net/eddie/SanAndreasRadio/
  16. http://www.gtaforums.com/
  17. http://www.steve-m.com/
  18. file://./COPYING.txt
