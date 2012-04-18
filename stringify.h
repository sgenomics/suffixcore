/* -
 * Copyright (c) 2012 Nava Whiteford <new@sgenomics.org>
 * suffixcore - core suffixtree algorithms
 *
 * A license to use this software is granted to users given access to the git repository at: https://github.com/sgenomics/suffixcore
 * A complete copy of this license is located in the license.txt file of this package.
 *
 * In Summary this software:
 *
 * Can be used for creating unlimited applications.
 * Can be distributed in binary or object form only.
 * Commercial use is allowed.
 * Can modify source-code but cannot distribute modifications (derivative works).
 */

#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>

#ifndef STRINGIFY_H
#define STRINGIFY_H

class BadConversion : public std::runtime_error {
public:
 BadConversion(const std::string& s)
      : std::runtime_error(s) {}
};

template<class _type>
inline std::string stringify(_type x)
{
  std::ostringstream o;
  if (!(o << std::fixed << x))
    throw BadConversion("stringify()");
  return o.str();
}

template<class _type>
inline _type convertTo(const std::string& s)
{
  std::istringstream i(s);
  _type x;
  if (!(i >> x))
    throw BadConversion("convertTo(\"" + s + "\")");
  return x;
}

#endif
