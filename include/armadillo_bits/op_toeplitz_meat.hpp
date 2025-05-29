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


// Copyright (C) 2013 Conrad Sanderson
// Copyright (C) 2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_toeplitz
//! @{



template<typename T1>
inline
void
op_toeplitz::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_toeplitz>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const unwrap_check<T1>  tmp(in.m, out);
  const Mat<eT>& X      = tmp.M;
  
  arma_debug_check( ((X.is_vec() == false) && (X.is_empty() == false)), "toeplitz(): given object is not a vector" );
  
  const uword N     = X.n_elem;
  const eT*   X_mem = X.memptr();
  
  out.set_size(N,N);
  
  for(uword col=0; col < N; ++col)
    {
    eT* col_mem = out.colptr(col);
    
    uword i;
    
    i = col;
    for(uword row=0; row < col; ++row, --i) { col_mem[row] = X_mem[i]; }
    
    i = 0;
    for(uword row=col; row < N; ++row, ++i) { col_mem[row] = X_mem[i]; }      
    }
  }



template<typename T1>
inline
void
op_toeplitz_c::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_toeplitz_c>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const unwrap_check<T1>  tmp(in.m, out);
  const Mat<eT>& X      = tmp.M;
  
  arma_debug_check( ((X.is_vec() == false) && (X.is_empty() == false)), "circ_toeplitz(): given object is not a vector" );
  
  const uword N     = X.n_elem;
  const eT*   X_mem = X.memptr();
  
  out.set_size(N,N);
  
  if(X.is_rowvec() == true)
    {
    for(uword row=0; row < N; ++row)
      {
      uword i;
      
      i = row;
      for(uword col=0; col < row; ++col, --i)  { out.at(row,col) = X_mem[N-i]; }
      
      i = 0;
      for(uword col=row; col < N; ++col, ++i)  { out.at(row,col) = X_mem[i];   }
      }
    }
  else
    {
    for(uword col=0; col < N; ++col)
      {
      eT* col_mem = out.colptr(col);
      
      uword i;
      
      i = col;
      for(uword row=0; row < col; ++row, --i)  { col_mem[row] = X_mem[N-i]; }
      
      i = 0;
      for(uword row=col; row < N; ++row, ++i)  { col_mem[row] = X_mem[i];   }
      }
    }
  }



//! @}
