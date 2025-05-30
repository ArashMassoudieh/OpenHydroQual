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


// Copyright (C) 2012-2015 Conrad Sanderson
// Copyright (C) 2012-2015 NICTA (www.nicta.com.au)
// Copyright (C) 2012 Arnold Wiliem
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



//! \addtogroup op_unique
//! @{



template<typename T1>
inline
bool
op_unique::apply_helper(Mat<typename T1::elem_type>& out, const Proxy<T1>& P)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const uword n_rows = P.get_n_rows();
  const uword n_cols = P.get_n_cols();
  const uword n_elem = P.get_n_elem();
  
  if(n_elem == 0)  { out.set_size(n_rows, n_cols); return true; }
  
  if(n_elem == 1)
    {
    const eT tmp = Proxy<T1>::prefer_at_accessor ? P.at(0,0) : P[0];
    
    out.set_size(n_rows, n_cols);
    
    out[0] = tmp;
    
    return true;
    }
  
  Mat<eT> X(n_elem,1);
  
  eT* X_mem = X.memptr();
  
  if(Proxy<T1>::prefer_at_accessor == false)
    {
    typename Proxy<T1>::ea_type Pea = P.get_ea();
    
    for(uword i=0; i<n_elem; ++i)
      {
      const eT val = Pea[i];
      
      if(arma_isnan(val))  { out.reset(); return false; }
      
      X_mem[i] = val;
      }
    }
  else
    {
    for(uword col=0; col < n_cols; ++col)
    for(uword row=0; row < n_rows; ++row)
      {
      const eT val = P.at(row,col);
      
      if(arma_isnan(val))  { out.reset(); return false; }
      
      (*X_mem) = val;  X_mem++;
      }
    
    X_mem = X.memptr();
    }
  
  arma_unique_comparator<eT> comparator;
  
  std::sort( X.begin(), X.end(), comparator );
  
  uword N_unique = 1;
  
  for(uword i=1; i < n_elem; ++i)
    {
    const eT a = X_mem[i-1];
    const eT b = X_mem[i  ];
    
    const eT diff = a - b;
    
    if(diff != eT(0)) { ++N_unique; }
    }
  
  uword out_n_rows;
  uword out_n_cols;
  
  if( (n_rows == 1) || (n_cols == 1) )
    {
    if(n_rows == 1)
      {
      out_n_rows = 1;
      out_n_cols = N_unique;
      }
    else
      {
      out_n_rows = N_unique;
      out_n_cols = 1;
      }
    }
  else
    {
    out_n_rows = N_unique;
    out_n_cols = 1;
    }
  
  out.set_size(out_n_rows, out_n_cols);
  
  eT* out_mem = out.memptr();
  
  if(n_elem > 0)  { (*out_mem) = X_mem[0];  out_mem++; }
  
  for(uword i=1; i < n_elem; ++i)
    {
    const eT a = X_mem[i-1];
    const eT b = X_mem[i  ];
    
    const eT diff = a - b;
    
    if(diff != eT(0))  { (*out_mem) = b;  out_mem++; }
    }
  
  return true;
  }



template<typename T1>
inline
void
op_unique::apply(Mat<typename T1::elem_type>& out, const Op<T1, op_unique>& in)
  {
  arma_extra_debug_sigprint();
  
  const Proxy<T1> P(in.m);
  
  const bool all_non_nan = op_unique::apply_helper(out, P);
  
  arma_debug_check( (all_non_nan == false), "unique(): detected NaN" );
  }



//! @}
