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


// Copyright (C) 2008-2014 Conrad Sanderson
// Copyright (C) 2008-2014 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup subview_field
//! @{


template<typename oT>
inline
subview_field<oT>::~subview_field()
  {
  arma_extra_debug_sigprint();
  }



template<typename oT>
arma_inline
subview_field<oT>::subview_field
  (
  const field<oT>& in_f,
  const uword      in_row1,
  const uword      in_col1,
  const uword      in_n_rows,
  const uword      in_n_cols
  )
  : f(in_f)
  , aux_row1(in_row1)
  , aux_col1(in_col1)
  , aux_slice1(0)
  , n_rows(in_n_rows)
  , n_cols(in_n_cols)
  , n_slices(1)
  , n_elem(in_n_rows*in_n_cols)
  {
  arma_extra_debug_sigprint();
  }



template<typename oT>
arma_inline
subview_field<oT>::subview_field
  (
  const field<oT>& in_f,
  const uword      in_row1,
  const uword      in_col1,
  const uword      in_slice1,
  const uword      in_n_rows,
  const uword      in_n_cols,
  const uword      in_n_slices
  )
  : f(in_f)
  , aux_row1(in_row1)
  , aux_col1(in_col1)
  , aux_slice1(in_slice1)
  , n_rows(in_n_rows)
  , n_cols(in_n_cols)
  , n_slices(in_n_slices)
  , n_elem(in_n_rows*in_n_cols*in_n_slices)
  {
  arma_extra_debug_sigprint();
  }



template<typename oT>
inline
void
subview_field<oT>::operator= (const field<oT>& x)
  {
  arma_extra_debug_sigprint();
  
  subview_field<oT>& t = *this;
  
  arma_debug_check( (t.n_rows != x.n_rows) || (t.n_cols != x.n_cols) || (t.n_slices != x.n_slices), "incompatible field dimensions");
  
  if(t.n_slices == 1)
    {
    for(uword col=0; col < t.n_cols; ++col)
    for(uword row=0; row < t.n_rows; ++row)
      {
      t.at(row,col) = x.at(row,col);
      }
    }
  else
    {
    for(uword slice=0; slice < t.n_slices; ++slice)
    for(uword col=0;   col   < t.n_cols;   ++col  )
    for(uword row=0;   row   < t.n_rows;   ++row  )
      {
      t.at(row,col,slice) = x.at(row,col,slice);
      }
    }
  }



//! x.subfield(...) = y.subfield(...)
template<typename oT>
inline
void
subview_field<oT>::operator= (const subview_field<oT>& x)
  {
  arma_extra_debug_sigprint();
  
  if(check_overlap(x))
    {
    const field<oT> tmp(x);
    
    (*this).operator=(tmp);
    
    return;
    }
  
  subview_field<oT>& t = *this;
  
  arma_debug_check( (t.n_rows != x.n_rows) || (t.n_cols != x.n_cols) || (t.n_slices != x.n_slices), "incompatible field dimensions");
  
  if(t.n_slices == 1)
    {
    for(uword col=0; col < t.n_cols; ++col)
    for(uword row=0; row < t.n_rows; ++row)
      {
      t.at(row,col) = x.at(row,col);
      }
    }
  else
    {
    for(uword slice=0; slice < t.n_slices; ++slice)
    for(uword col=0;   col   < t.n_cols;   ++col  )
    for(uword row=0;   row   < t.n_rows;   ++row  )
      {
      t.at(row,col,slice) = x.at(row,col,slice);
      }
    }
  }



template<typename oT>
arma_inline
oT&
subview_field<oT>::operator[](const uword i)
  {
  uword index;
  
  if(n_slices == 1)
    {
    const uword in_col = i / n_rows;
    const uword in_row = i % n_rows;
      
    index = (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
    }
  else
    {
    const uword n_elem_slice = n_rows*n_cols;
    
    const uword in_slice = i / n_elem_slice;
    const uword offset   = in_slice * n_elem_slice;
    const uword j        = i - offset;
    
    const uword in_col   = j / n_rows;
    const uword in_row   = j % n_rows;
    
    index = (in_slice + aux_slice1)*(f.n_rows*f.n_cols) + (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
    }
  
  return *((const_cast< field<oT>& >(f)).mem[index]);
  }



template<typename oT>
arma_inline
const oT&
subview_field<oT>::operator[](const uword i) const
  {
  uword index;
  
  if(n_slices == 1)
    {
    const uword in_col = i / n_rows;
    const uword in_row = i % n_rows;
      
    index = (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
    }
  else
    {
    const uword n_elem_slice = n_rows*n_cols;
    
    const uword in_slice = i / n_elem_slice;
    const uword offset   = in_slice * n_elem_slice;
    const uword j        = i - offset;
    
    const uword in_col   = j / n_rows;
    const uword in_row   = j % n_rows;
    
    index = (in_slice + aux_slice1)*(f.n_rows*f.n_cols) + (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
    }
  
  return *(f.mem[index]);
  }



template<typename oT>
arma_inline
oT&
subview_field<oT>::operator()(const uword i)
  {
  arma_debug_check( (i >= n_elem), "subview_field::operator(): index out of bounds");
  
  return operator[](i);
  }



template<typename oT>
arma_inline
const oT&
subview_field<oT>::operator()(const uword i) const
  {
  arma_debug_check( (i >= n_elem), "subview_field::operator(): index out of bounds");
  
  return operator[](i);
  }



template<typename oT>
arma_inline
oT&
subview_field<oT>::operator()(const uword in_row, const uword in_col)
  {
  arma_debug_check( ((in_row >= n_rows) || (in_col >= n_cols)), "subview_field::operator(): index out of bounds");
  
  const uword index = (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
  
  return *((const_cast< field<oT>& >(f)).mem[index]);
  }



template<typename oT>
arma_inline
const oT&
subview_field<oT>::operator()(const uword in_row, const uword in_col) const
  {
  arma_debug_check( ((in_row >= n_rows) || (in_col >= n_cols)), "subview_field::operator(): index out of bounds");
  
  const uword index = (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
  
  return *(f.mem[index]);
  }



template<typename oT>
arma_inline
oT&
subview_field<oT>::operator()(const uword in_row, const uword in_col, const uword in_slice)
  {
  arma_debug_check( ((in_row >= n_rows) || (in_col >= n_cols) || (in_slice >= n_slices)), "subview_field::operator(): index out of bounds");
  
  const uword index = (in_slice + aux_slice1)*(f.n_rows*f.n_cols) + (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
  
  return *((const_cast< field<oT>& >(f)).mem[index]);
  }



template<typename oT>
arma_inline
const oT&
subview_field<oT>::operator()(const uword in_row, const uword in_col, const uword in_slice) const
  {
  arma_debug_check( ((in_row >= n_rows) || (in_col >= n_cols) || (in_slice >= n_slices)), "subview_field::operator(): index out of bounds");
  
  const uword index = (in_slice + aux_slice1)*(f.n_rows*f.n_cols) + (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
  
  return *(f.mem[index]);
  }



template<typename oT>
arma_inline
oT&
subview_field<oT>::at(const uword in_row, const uword in_col)
  {
  const uword index = (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
  
  return *((const_cast< field<oT>& >(f)).mem[index]);
  }



template<typename oT>
arma_inline
const oT&
subview_field<oT>::at(const uword in_row, const uword in_col) const
  {
  const uword index = (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
  
  return *(f.mem[index]);
  }



template<typename oT>
arma_inline
oT&
subview_field<oT>::at(const uword in_row, const uword in_col, const uword in_slice)
  {
  const uword index = (in_slice + aux_slice1)*(f.n_rows*f.n_cols) + (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
  
  return *((const_cast< field<oT>& >(f)).mem[index]);
  }



template<typename oT>
arma_inline
const oT&
subview_field<oT>::at(const uword in_row, const uword in_col, const uword in_slice) const
  {
  const uword index = (in_slice + aux_slice1)*(f.n_rows*f.n_cols) + (in_col + aux_col1)*f.n_rows + aux_row1 + in_row;
  
  return *(f.mem[index]);
  }



template<typename oT>
inline
bool
subview_field<oT>::check_overlap(const subview_field<oT>& x) const
  {
  const subview_field<oT>& t = *this;
  
  if(&t.f != &x.f)
    {
    return false;
    }
  else
    {
    if( (t.n_elem == 0) || (x.n_elem == 0) )
      {
      return false;
      }
    else
      {
      const uword t_row_start    = t.aux_row1;
      const uword t_row_end_p1   = t_row_start + t.n_rows;
      
      const uword t_col_start    = t.aux_col1;
      const uword t_col_end_p1   = t_col_start + t.n_cols;
      
      const uword t_slice_start  = t.aux_slice1;
      const uword t_slice_end_p1 = t_slice_start + t.n_slices;
      
      const uword x_row_start    = x.aux_row1;
      const uword x_row_end_p1   = x_row_start + x.n_rows;
      
      const uword x_col_start    = x.aux_col1;
      const uword x_col_end_p1   = x_col_start + x.n_cols;
      
      const uword x_slice_start  = x.aux_slice1;
      const uword x_slice_end_p1 = x_slice_start + x.n_slices;
      
      const bool outside_rows   = ( (x_row_start   >= t_row_end_p1  ) || (t_row_start   >= x_row_end_p1  ) );
      const bool outside_cols   = ( (x_col_start   >= t_col_end_p1  ) || (t_col_start   >= x_col_end_p1  ) );
      const bool outside_slices = ( (x_slice_start >= t_slice_end_p1) || (t_slice_start >= x_slice_end_p1) );
      
      return ( (outside_rows == false) && (outside_cols == false) && (outside_slices == false) );
      }
    }
  }



template<typename oT>
inline
void
subview_field<oT>::print(const std::string extra_text) const
  {
  arma_extra_debug_sigprint();
  
  if(extra_text.length() != 0)
    {
    const std::streamsize orig_width = ARMA_DEFAULT_OSTREAM.width();
    
    ARMA_DEFAULT_OSTREAM << extra_text << '\n';
  
    ARMA_DEFAULT_OSTREAM.width(orig_width);
    }
  
  arma_ostream::print(ARMA_DEFAULT_OSTREAM, *this);
  }



template<typename oT>
inline
void
subview_field<oT>::print(std::ostream& user_stream, const std::string extra_text) const
  {
  arma_extra_debug_sigprint();
  
  if(extra_text.length() != 0)
    {
    const std::streamsize orig_width = user_stream.width();
    
    user_stream << extra_text << '\n';
  
    user_stream.width(orig_width);
    }
  
  arma_ostream::print(user_stream, *this);
  }



//! X = Y.subfield(...)
template<typename oT>
inline
void
subview_field<oT>::extract(field<oT>& actual_out, const subview_field<oT>& in)
  {
  arma_extra_debug_sigprint();
  
  //
  const bool alias = (&actual_out == &in.f);
  
  field<oT>* tmp = (alias) ? new field<oT> : 0;
  field<oT>& out = (alias) ? (*tmp)        : actual_out;
  
  //
  
  const uword n_rows   = in.n_rows;
  const uword n_cols   = in.n_cols;
  const uword n_slices = in.n_slices;
  
  out.set_size(n_rows, n_cols, n_slices);
  
  arma_extra_debug_print(arma_boost::format("out.n_rows = %d   out.n_cols = %d   out.n_slices = %d    in.m.n_rows = %d  in.m.n_cols = %d  in.m.n_slices = %d") % out.n_rows % out.n_cols % out.n_slices % in.f.n_rows % in.f.n_cols % in.f.n_slices);
  
  if(n_slices == 1)
    {
    for(uword col = 0; col < n_cols; ++col)
    for(uword row = 0; row < n_rows; ++row)
      {
      out.at(row,col) = in.at(row,col);
      }
    }
  else
    {
    for(uword slice = 0; slice < n_slices; ++slice)
    for(uword col   = 0; col   < n_cols;   ++col  )
    for(uword row   = 0; row   < n_rows;   ++row  )
      {
      out.at(row,col,slice) = in.at(row,col,slice);
      }
    }
  
  if(alias)
    {
    actual_out = out;
    delete tmp;
    }
  
  }



//! @}
