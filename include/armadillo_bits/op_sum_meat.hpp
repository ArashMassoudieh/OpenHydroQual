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


// Copyright (C) 2008-2015 Conrad Sanderson
// Copyright (C) 2008-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_sum
//! @{



template<typename T1>
arma_hot
inline
void
op_sum::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_sum>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const uword dim = in.aux_uword_a;
  arma_debug_check( (dim > 1), "sum(): parameter 'dim' must be 0 or 1" );
  
  const Proxy<T1> P(in.m);
  
  if(P.is_alias(out) == false)
    {
    op_sum::apply_noalias(out, P, dim);
    }
  else
    {
    Mat<eT> tmp;
    
    op_sum::apply_noalias(tmp, P, dim);
    
    out.steal_mem(tmp);
    }
  }



template<typename T1>
arma_hot
inline
void
op_sum::apply_noalias(Mat<typename T1::elem_type>& out, const Proxy<T1>& P, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  if(is_Mat<typename Proxy<T1>::stored_type>::value)
    {
    op_sum::apply_noalias_unwrap(out, P, dim);
    }
  else
    {
    op_sum::apply_noalias_proxy(out, P, dim);
    }
  }



template<typename T1>
arma_hot
inline
void
op_sum::apply_noalias_unwrap(Mat<typename T1::elem_type>& out, const Proxy<T1>& P, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  typedef typename Proxy<T1>::stored_type P_stored_type;
  
  const unwrap<P_stored_type> tmp(P.Q);
  
  const typename unwrap<P_stored_type>::stored_type& X = tmp.M;
  
  const uword X_n_rows = X.n_rows;
  const uword X_n_cols = X.n_cols;
  
  if(dim == 0)
    {
    out.set_size(1, X_n_cols);
    
    eT* out_mem = out.memptr();
    
    for(uword col=0; col < X_n_cols; ++col)
      {
      out_mem[col] = arrayops::accumulate( X.colptr(col), X_n_rows );
      }
    }
  else
    {
    out.zeros(X_n_rows, 1);
    
    eT* out_mem = out.memptr();
    
    for(uword col=0; col < X_n_cols; ++col)
      {
      arrayops::inplace_plus( out_mem, X.colptr(col), X_n_rows );
      }
    }
  }



template<typename T1>
arma_hot
inline
void
op_sum::apply_noalias_proxy(Mat<typename T1::elem_type>& out, const Proxy<T1>& P, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const uword P_n_rows = P.get_n_rows();
  const uword P_n_cols = P.get_n_cols();
  
  if(dim == 0)
    {
    out.set_size(1, P_n_cols);
    
    eT* out_mem = out.memptr();
    
    for(uword col=0; col < P_n_cols; ++col)
      {
      eT val1 = eT(0);
      eT val2 = eT(0);
      
      uword i,j;
      for(i=0, j=1; j < P_n_rows; i+=2, j+=2)
        {
        val1 += P.at(i,col);
        val2 += P.at(j,col);
        }
      
      if(i < P_n_rows)
        {
        val1 += P.at(i,col);
        }
      
      out_mem[col] = (val1 + val2);
      }
    }
  else
    {
    out.zeros(P_n_rows, 1);
    
    eT* out_mem = out.memptr();
    
    for(uword col=0; col < P_n_cols; ++col)
    for(uword row=0; row < P_n_rows; ++row)
      {
      out_mem[row] += P.at(row,col);
      }
    }
  }



//! @}
