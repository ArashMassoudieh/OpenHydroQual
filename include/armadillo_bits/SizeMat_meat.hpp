/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


// Copyright (C) 2013-2015 Conrad Sanderson
// Copyright (C) 2013-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup SizeMat
//! @{



inline
SizeMat::SizeMat(const uword in_n_rows, const uword in_n_cols)
  : n_rows(in_n_rows)
  , n_cols(in_n_cols)
  {
  arma_extra_debug_sigprint();
  }



inline
uword
SizeMat::operator[](const uword dim) const
  {
  if(dim == 0)  { return n_rows; }
  if(dim == 1)  { return n_cols; }
  
  return uword(1);
  }



inline
uword
SizeMat::operator()(const uword dim) const
  {
  if(dim == 0)  { return n_rows; }
  if(dim == 1)  { return n_cols; }
  
  arma_debug_check(true, "size(): index out of bounds");
  
  return uword(1);
  }



inline
bool
SizeMat::operator==(const SizeMat& s) const
  {
  if(n_rows != s.n_rows)  { return false; }
  
  if(n_cols != s.n_cols)  { return false; }
  
  return true;
  }



inline
bool
SizeMat::operator!=(const SizeMat& s) const
  {
  if(n_rows != s.n_rows)  { return true; }
  
  if(n_cols != s.n_cols)  { return true; }
  
  return false;
  }



inline
SizeMat
SizeMat::operator+(const SizeMat& s) const
  {
  return SizeMat( (n_rows + s.n_rows), (n_cols + s.n_cols) );
  }



inline
SizeMat
SizeMat::operator-(const SizeMat& s) const
  {
  const uword out_n_rows = (n_rows > s.n_rows) ? (n_rows - s.n_rows) : uword(0);
  const uword out_n_cols = (n_cols > s.n_cols) ? (n_cols - s.n_cols) : uword(0);
  
  return SizeMat(out_n_rows, out_n_cols);
  }



inline
SizeMat
SizeMat::operator+(const uword val) const
  {
  return SizeMat( (n_rows + val), (n_cols + val) );
  }



inline
SizeMat
SizeMat::operator-(const uword val) const
  {
  const uword out_n_rows = (n_rows > val) ? (n_rows - val) : uword(0);
  const uword out_n_cols = (n_cols > val) ? (n_cols - val) : uword(0);
  
  return SizeMat(out_n_rows, out_n_cols);
  }



inline
SizeMat
SizeMat::operator*(const uword val) const
  {
  return SizeMat( (n_rows * val), (n_cols * val) );
  }



inline
SizeMat
SizeMat::operator/(const uword val) const
  {
  return SizeMat( (n_rows / val), (n_cols / val) );
  }



//! @}
