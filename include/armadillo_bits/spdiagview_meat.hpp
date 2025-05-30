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


// Copyright (C) 2015 Conrad Sanderson
// Copyright (C) 2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup spdiagview
//! @{


template<typename eT>
inline
spdiagview<eT>::~spdiagview()
  {
  arma_extra_debug_sigprint();
  }


template<typename eT>
arma_inline
spdiagview<eT>::spdiagview(const SpMat<eT>& in_m, const uword in_row_offset, const uword in_col_offset, const uword in_len)
  : m(in_m)
  , row_offset(in_row_offset)
  , col_offset(in_col_offset)
  , n_rows(in_len)
  , n_elem(in_len)
  {
  arma_extra_debug_sigprint();
  }



//! set a diagonal of our matrix using a diagonal from a foreign matrix
template<typename eT>
inline
void
spdiagview<eT>::operator= (const spdiagview<eT>& x)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  arma_debug_check( (d.n_elem != x.n_elem), "spdiagview: diagonals have incompatible lengths");
  
        SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  const SpMat<eT>& x_m = x.m;
  
  if(&d_m != &x_m)
    {
    const uword d_n_elem     = d.n_elem;
    const uword d_row_offset = d.row_offset;
    const uword d_col_offset = d.col_offset;
    
    const uword x_row_offset = x.row_offset;
    const uword x_col_offset = x.col_offset;
    
    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) = x_m.at(i + x_row_offset, i + x_col_offset);
      }
    }
  else
    {
    const Mat<eT> tmp = x;
    
    (*this).operator=(tmp);
    }
  }



template<typename eT>
inline
void
spdiagview<eT>::operator+=(const eT val)
  {
  arma_extra_debug_sigprint();
  
  SpMat<eT>& t_m = const_cast< SpMat<eT>& >(m);
  
  const uword t_n_elem     = n_elem;
  const uword t_row_offset = row_offset;
  const uword t_col_offset = col_offset;
  
  for(uword i=0; i < t_n_elem; ++i)
    {
    t_m.at(i + t_row_offset, i + t_col_offset) += val;
    }
  }



template<typename eT>
inline
void
spdiagview<eT>::operator-=(const eT val)
  {
  arma_extra_debug_sigprint();
  
  SpMat<eT>& t_m = const_cast< SpMat<eT>& >(m);
  
  const uword t_n_elem     = n_elem;
  const uword t_row_offset = row_offset;
  const uword t_col_offset = col_offset;
  
  for(uword i=0; i < t_n_elem; ++i)
    {
    t_m.at(i + t_row_offset, i + t_col_offset) -= val;
    }
  }



template<typename eT>
inline
void
spdiagview<eT>::operator*=(const eT val)
  {
  arma_extra_debug_sigprint();
  
  SpMat<eT>& t_m = const_cast< SpMat<eT>& >(m);
  
  const uword t_n_elem     = n_elem;
  const uword t_row_offset = row_offset;
  const uword t_col_offset = col_offset;
  
  for(uword i=0; i < t_n_elem; ++i)
    {
    t_m.at(i + t_row_offset, i + t_col_offset) *= val;
    }
  }



template<typename eT>
inline
void
spdiagview<eT>::operator/=(const eT val)
  {
  arma_extra_debug_sigprint();
  
  SpMat<eT>& t_m = const_cast< SpMat<eT>& >(m);
  
  const uword t_n_elem     = n_elem;
  const uword t_row_offset = row_offset;
  const uword t_col_offset = col_offset;
  
  for(uword i=0; i < t_n_elem; ++i)
    {
    t_m.at(i + t_row_offset, i + t_col_offset) /= val;
    }
  }



//! set a diagonal of our matrix using data from a foreign object
template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator= (const Base<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
    
  const Proxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( (is_Mat<typename Proxy<T1>::stored_type>::value) || (Proxy<T1>::prefer_at_accessor) )
    {
    const unwrap<typename Proxy<T1>::stored_type> tmp(P.Q);
    const Mat<eT>& x = tmp.M;
    
    const eT* x_mem = x.memptr();

    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) = x_mem[i];
      }
    }
  else
    {
    typename Proxy<T1>::ea_type Pea = P.get_ea();
      
    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) = Pea[i];
      }
    }
  }



template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator+=(const Base<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
    
  const Proxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( (is_Mat<typename Proxy<T1>::stored_type>::value) || (Proxy<T1>::prefer_at_accessor) )
    {
    const unwrap<typename Proxy<T1>::stored_type> tmp(P.Q);
    const Mat<eT>& x = tmp.M;
    
    const eT* x_mem = x.memptr();

    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) += x_mem[i];
      }
    }
  else
    {
    typename Proxy<T1>::ea_type Pea = P.get_ea();
      
    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) += Pea[i];
      }
    }
  }



template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator-=(const Base<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
    
  const Proxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( (is_Mat<typename Proxy<T1>::stored_type>::value) || (Proxy<T1>::prefer_at_accessor) )
    {
    const unwrap<typename Proxy<T1>::stored_type> tmp(P.Q);
    const Mat<eT>& x = tmp.M;
    
    const eT* x_mem = x.memptr();

    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) -= x_mem[i];
      }
    }
  else
    {
    typename Proxy<T1>::ea_type Pea = P.get_ea();
      
    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) -= Pea[i];
      }
    }
  }



template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator%=(const Base<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
    
  const Proxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( (is_Mat<typename Proxy<T1>::stored_type>::value) || (Proxy<T1>::prefer_at_accessor) )
    {
    const unwrap<typename Proxy<T1>::stored_type> tmp(P.Q);
    const Mat<eT>& x = tmp.M;
    
    const eT* x_mem = x.memptr();

    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) *= x_mem[i];
      }
    }
  else
    {
    typename Proxy<T1>::ea_type Pea = P.get_ea();
      
    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) *= Pea[i];
      }
    }
  }



template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator/=(const Base<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
    
  const Proxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( (is_Mat<typename Proxy<T1>::stored_type>::value) || (Proxy<T1>::prefer_at_accessor) )
    {
    const unwrap<typename Proxy<T1>::stored_type> tmp(P.Q);
    const Mat<eT>& x = tmp.M;
    
    const eT* x_mem = x.memptr();

    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) /= x_mem[i];
      }
    }
  else
    {
    typename Proxy<T1>::ea_type Pea = P.get_ea();
      
    for(uword i=0; i < d_n_elem; ++i)
      {
      d_m.at(i + d_row_offset, i + d_col_offset) /= Pea[i];
      }
    }
  }



//! set a diagonal of our matrix using data from a foreign object
template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator= (const SpBase<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
  
  const SpProxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( SpProxy<T1>::must_use_iterator || P.is_alias(d_m) )
    {
    const SpMat<eT> tmp(P.Q);
    
    if(tmp.n_cols == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) = tmp.at(i,0); }
      }
    else
    if(tmp.n_rows == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) = tmp.at(0,i); }
      }
    }
  else
    {
    if(P.get_n_cols() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) = P.at(i,0); }
      }
    else
    if(P.get_n_rows() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) = P.at(0,i); }
      }
    }
  }



template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator+=(const SpBase<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
  
  const SpProxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( SpProxy<T1>::must_use_iterator || P.is_alias(d_m) )
    {
    const SpMat<eT> tmp(P.Q);
    
    if(tmp.n_cols == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) += tmp.at(i,0); }
      }
    else
    if(tmp.n_rows == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) += tmp.at(0,i); }
      }
    }
  else
    {
    if(P.get_n_cols() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) += P.at(i,0); }
      }
    else
    if(P.get_n_rows() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) += P.at(0,i); }
      }
    }
  }



template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator-=(const SpBase<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
  
  const SpProxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( SpProxy<T1>::must_use_iterator || P.is_alias(d_m) )
    {
    const SpMat<eT> tmp(P.Q);
    
    if(tmp.n_cols == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) -= tmp.at(i,0); }
      }
    else
    if(tmp.n_rows == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) -= tmp.at(0,i); }
      }
    }
  else
    {
    if(P.get_n_cols() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) -= P.at(i,0); }
      }
    else
    if(P.get_n_rows() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) -= P.at(0,i); }
      }
    }
  }



template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator%=(const SpBase<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
  
  const SpProxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( SpProxy<T1>::must_use_iterator || P.is_alias(d_m) )
    {
    const SpMat<eT> tmp(P.Q);
    
    if(tmp.n_cols == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) *= tmp.at(i,0); }
      }
    else
    if(tmp.n_rows == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) *= tmp.at(0,i); }
      }
    }
  else
    {
    if(P.get_n_cols() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) *= P.at(i,0); }
      }
    else
    if(P.get_n_rows() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) *= P.at(0,i); }
      }
    }
  }



template<typename eT>
template<typename T1>
inline
void
spdiagview<eT>::operator/=(const SpBase<eT,T1>& o)
  {
  arma_extra_debug_sigprint();
  
  spdiagview<eT>& d = *this;
  
  SpMat<eT>& d_m = const_cast< SpMat<eT>& >(d.m);
  
  const uword d_n_elem     = d.n_elem;
  const uword d_row_offset = d.row_offset;
  const uword d_col_offset = d.col_offset;
  
  const SpProxy<T1> P( o.get_ref() );
  
  arma_debug_check
    (
    ( (d_n_elem != P.get_n_elem()) || ((P.get_n_rows() != 1) && (P.get_n_cols() != 1)) ),
    "spdiagview: given object has incompatible size"
    );
  
  if( SpProxy<T1>::must_use_iterator || P.is_alias(d_m) )
    {
    const SpMat<eT> tmp(P.Q);
    
    if(tmp.n_cols == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) /= tmp.at(i,0); }
      }
    else
    if(tmp.n_rows == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) /= tmp.at(0,i); }
      }
    }
  else
    {
    if(P.get_n_cols() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) /= P.at(i,0); }
      }
    else
    if(P.get_n_rows() == 1)
      {
      for(uword i=0; i < d_n_elem; ++i)  { d_m.at(i + d_row_offset, i + d_col_offset) /= P.at(0,i); }
      }
    }
  }



//! extract a diagonal and store it as a dense column vector
template<typename eT>
inline
void
spdiagview<eT>::extract(Mat<eT>& out, const spdiagview<eT>& in)
  {
  arma_extra_debug_sigprint();
  
  // NOTE: we're assuming that the 'out' matrix has already been set to the correct size;
  // size setting is done by either the Mat contructor or Mat::operator=()
  
  const SpMat<eT>& in_m = in.m;
  
  const uword in_n_elem     = in.n_elem;
  const uword in_row_offset = in.row_offset;
  const uword in_col_offset = in.col_offset;
  
  eT* out_mem = out.memptr();
  
  for(uword i=0; i < in_n_elem; ++i)
    {
    out_mem[i] = in_m.at(i + in_row_offset, i + in_col_offset );
    }
  }



template<typename eT>
inline
eT
spdiagview<eT>::at_alt(const uword i) const
  {
  return m.at(i+row_offset, i+col_offset);
  }



template<typename eT>
inline
SpValProxy< SpMat<eT> >
spdiagview<eT>::operator[](const uword i)
  {
  return (const_cast< SpMat<eT>& >(m)).at(i+row_offset, i+col_offset);
  }



template<typename eT>
inline
eT
spdiagview<eT>::operator[](const uword i) const
  {
  return m.at(i+row_offset, i+col_offset);
  }



template<typename eT>
inline
SpValProxy< SpMat<eT> >
spdiagview<eT>::at(const uword i)
  {
  return (const_cast< SpMat<eT>& >(m)).at(i+row_offset, i+col_offset);
  }



template<typename eT>
inline
eT
spdiagview<eT>::at(const uword i) const
  {
  return m.at(i+row_offset, i+col_offset);
  }



template<typename eT>
inline
SpValProxy< SpMat<eT> >
spdiagview<eT>::operator()(const uword i)
  {
  arma_debug_check( (i >= n_elem), "spdiagview::operator(): out of bounds" );
  
  return (const_cast< SpMat<eT>& >(m)).at(i+row_offset, i+col_offset);
  }



template<typename eT>
inline
eT
spdiagview<eT>::operator()(const uword i) const
  {
  arma_debug_check( (i >= n_elem), "spdiagview::operator(): out of bounds" );
  
  return m.at(i+row_offset, i+col_offset);
  }



template<typename eT>
inline
SpValProxy< SpMat<eT> >
spdiagview<eT>::at(const uword row, const uword)
  {
  return (const_cast< SpMat<eT>& >(m)).at(row+row_offset, row+col_offset);
  }



template<typename eT>
inline
eT
spdiagview<eT>::at(const uword row, const uword) const
  {
  return m.at(row+row_offset, row+col_offset);
  }



template<typename eT>
inline
SpValProxy< SpMat<eT> >
spdiagview<eT>::operator()(const uword row, const uword col)
  {
  arma_debug_check( ((row >= n_elem) || (col > 0)), "spdiagview::operator(): out of bounds" );
  
  return (const_cast< SpMat<eT>& >(m)).at(row+row_offset, row+col_offset);
  }



template<typename eT>
inline
eT
spdiagview<eT>::operator()(const uword row, const uword col) const
  {
  arma_debug_check( ((row >= n_elem) || (col > 0)), "spdiagview::operator(): out of bounds" );
  
  return m.at(row+row_offset, row+col_offset);
  }



template<typename eT>
inline
void
spdiagview<eT>::fill(const eT val)
  {
  arma_extra_debug_sigprint();
  
  SpMat<eT>& x = const_cast< SpMat<eT>& >(m);
  
  const uword local_n_elem = n_elem;
  
  for(uword i=0; i < local_n_elem; ++i)
    {
    x.at(i+row_offset, i+col_offset) = val;
    }
  }



template<typename eT>
inline
void
spdiagview<eT>::zeros()
  {
  arma_extra_debug_sigprint();
  
  (*this).fill(eT(0));
  }



template<typename eT>
inline
void
spdiagview<eT>::ones()
  {
  arma_extra_debug_sigprint();
  
  (*this).fill(eT(1));
  }



template<typename eT>
inline
void
spdiagview<eT>::randu()
  {
  arma_extra_debug_sigprint();
  
  SpMat<eT>& x = const_cast< SpMat<eT>& >(m);
  
  const uword local_n_elem = n_elem;
  
  for(uword i=0; i < local_n_elem; ++i)
    {
    x.at(i+row_offset, i+col_offset) = eT(arma_rng::randu<eT>());
    }
  }



template<typename eT>
inline
void
spdiagview<eT>::randn()
  {
  arma_extra_debug_sigprint();
  
  SpMat<eT>& x = const_cast< SpMat<eT>& >(m);
  
  const uword local_n_elem = n_elem;
  
  for(uword i=0; i < local_n_elem; ++i)
    {
    x.at(i+row_offset, i+col_offset) = eT(arma_rng::randn<eT>());
    }
  }



//! @}
