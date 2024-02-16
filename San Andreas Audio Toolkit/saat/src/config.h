/* The San Andreas Audio Toolkit (SAAT): config.h
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
 * INI/Configuration file function header file adapted from RFSA
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <map>
#include <stdexcept>

// This is thrown, with the failed directive as the message, when a
// directive lookup fails.
class directive_not_found : public std::runtime_error {
 public:
  explicit directive_not_found(const std::string&  __arg) :
    runtime_error(__arg)
    {
    }
};

// An instance of this class holds all configuration options read from
// a configuration file.
class config {
 public:

  /* Read a config file into this config object from an istream.
   *
   * Arguments:
   *  is    The istream to read from.
   *  conf  The config object to populate.
   *
   * Returns:
   *  The istream.
   */
  friend std::istream &operator<<(config &conf, std::istream &is);
  friend std::istream &operator>>(std::istream &is, config &conf);
  
  /* Write a config file from this config object to an ostream.
   *
   * Arguments:
   *  is    The ostream to write to.
   *  conf  The config object to use.
   *
   * Returns:
   *  The ostream.
   */
  friend std::ostream &operator<<(std::ostream &os, config &conf);
  friend std::ostream &operator>>(config &conf, std::ostream &os);
  
  /* Gets the setting(s) of a key.
   *
   * Arguments:
   *  key  The key of the setting to lookup; of the form 'section.key'.
   *
   * Returns:
   *  The value of the setting.
   *
   * Exceptions:
   *  directive_not_found The key was not found.
   */
  std::string operator[](const std::string &key) const;
  
  /* Fetches the value for a key, or a default value if the key does
   * not exist.
   *
   * Arguments:
   *  key  The key of the setting to lookup; of the form 'section.key'.
   *  def  The default value. Returned if the key was not found.
   *
   * Returns:
   *  The key's value or the default value.
   */
  std::string lookup(const std::string &key, const std::string &def) const;
  
  /* Assigns the value for a key, creating the key if necessary.
   *
   * Arguments:
   *  key  The key of the setting to change; of the form 'section.key'.
   *  val  The new value in a variety of forms which all will become strings.
   *
   * Returns:
   *  The key's value in string form.
   */
  std::string assign(const std::string &key, const std::string &val);
  std::string assign(const std::string &key, const uint32_t val);
  std::string assign(const std::string &key, const int32_t val);
  std::string assign(const std::string &key, const uint16_t val);
  std::string assign(const std::string &key, const uint8_t val);

 private:

  // The settings mapping from 'section.key' to 'value'.
  std::map<std::string, std::string> settings;
};

#endif
