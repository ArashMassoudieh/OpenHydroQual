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


// Copyright (C) 2008-2012 Conrad Sanderson
// Copyright (C) 2008-2012 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_min
//! @{



//! \brief
//! For each row or for each column, find the minimum value.
//! The result is stored in a dense matrix that has either one column or one row.
//! The dimension, for which the minima are found, is set via the min() function.
template<typename T1>
inline void op_min::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_min>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const unwrap_check<T1> tmp(in.m, out);
  const Mat<eT>&     X = tmp.M;
  
  const uword dim = in.aux_uword_a;
  arma_debug_check( (dim > 1), "min(): incorrect usage. dim must be 0 or 1");
  
  const uword X_n_rows = X.n_rows;
  const uword X_n_cols = X.n_cols;
  
  if(dim == 0)  // min in each column
    {
    arma_extra_debug_print("op_min::apply(), dim = 0");
    
    arma_debug_check( (X_n_rows == 0), "min(): given object has zero rows" );

    out.set_size(1, X_n_cols);
    
    eT* out_mem = out.memptr();
    
    for(uword col=0; col<X_n_cols; ++col)
      {
      out_mem[col] = op_min::direct_min( X.colptr(col), X_n_rows );
      }
    }
  else
  if(dim == 1)  // min in each row
    {
    arma_extra_debug_print("op_min::apply(), dim = 1");
    
    arma_debug_check( (X_n_cols == 0), "min(): given object has zero columns" );

    out.set_size(X_n_rows, 1);
    
    eT* out_mem = out.memptr();
    
    for(uword row=0; row<X_n_rows; ++row)
      {
      out_mem[row] = op_min::direct_min( X, row );
      }
    }
  }



template<typename eT>
arma_pure
inline 
eT
op_min::direct_min(const eT* const X, const uword n_elem)
  {
  arma_extra_debug_sigprint();
  
  eT min_val = priv::most_pos<eT>();
  
  uword i,j;
  for(i=0, j=1; j<n_elem; i+=2, j+=2)
    {
    const eT X_i = X[i];
    const eT X_j = X[j];
    
    if(X_i < min_val) { min_val = X_i; }
    if(X_j < min_val) { min_val = X_j; }
    }
  
  if(i < n_elem)
    {
    const eT X_i = X[i];
    
    if(X_i < min_val) { min_val = X_i; }
    }
  
  return min_val;
  }



template<typename eT>
inline 
eT
op_min::direct_min(const eT* const X, const uword n_elem, uword& index_of_min_val)
  {
  arma_extra_debug_sigprint();
  
  eT min_val = priv::most_pos<eT>();
  
  uword best_index = 0;
  
  uword i,j;
  for(i=0, j=1; j<n_elem; i+=2, j+=2)
    {
    const eT X_i = X[i];
    const eT X_j = X[j];
    
    if(X_i < min_val)
      {
      min_val    = X_i;
      best_index = i;
      }
    
    if(X_j < min_val)
      {
      min_val    = X_j;
      best_index = j;
      }
    }
  
  if(i < n_elem)
    {
    const eT X_i = X[i];
    
    if(X_i < min_val)
      {
      min_val    = X_i;
      best_index = i;
      }
    }
  
  index_of_min_val = best_index;
  
  return min_val;
  }  



template<typename eT>
inline 
eT
op_min::direct_min(const Mat<eT>& X, const uword row)
  {
  arma_extra_debug_sigprint();
  
  const uword X_n_cols = X.n_cols;
  
  eT min_val = priv::most_pos<eT>();
  
  uword i,j;
  for(i=0, j=1; j < X_n_cols; i+=2, j+=2)
    {
    const eT tmp_i = X.at(row,i);
    const eT tmp_j = X.at(row,j);
    
    if(tmp_i < min_val) { min_val = tmp_i; }
    if(tmp_j < min_val) { min_val = tmp_j; }
    }
  
  if(i < X_n_cols)
    {
    const eT tmp_i = X.at(row,i);
    
    if(tmp_i < min_val) { min_val = tmp_i; }
    }
  
  return min_val;
  }



template<typename eT>
inline
eT
op_min::min(const subview<eT>& X)
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( (X.n_elem == 0), "min(): given object has no elements" );
  
  const uword X_n_rows = X.n_rows;
  const uword X_n_cols = X.n_cols;
  
  eT min_val = priv::most_pos<eT>();
  
  if(X_n_rows == 1)
    {
    const Mat<eT>& A = X.m;
    
    const uword start_row = X.aux_row1;
    const uword start_col = X.aux_col1;
    
    const uword end_col_p1 = start_col + X_n_cols;
  
    uword i,j;
    for(i=start_col, j=start_col+1; j < end_col_p1; i+=2, j+=2)
      {
      const eT tmp_i = A.at(start_row, i);
      const eT tmp_j = A.at(start_row, j);
      
      if(tmp_i < min_val) { min_val = tmp_i; }
      if(tmp_j < min_val) { min_val = tmp_j; }
      }
    
    if(i < end_col_p1)
      {
      const eT tmp_i = A.at(start_row, i);
      
      if(tmp_i < min_val) { min_val = tmp_i; }
      }
    }
  else
    {
    for(uword col=0; col < X_n_cols; ++col)
      {
      eT tmp_val = op_min::direct_min(X.colptr(col), X_n_rows);
      
      if(tmp_val < min_val) { min_val = tmp_val; }
      }
    }
  
  return min_val;
  }



template<typename T1>
inline
typename arma_not_cx<typename T1::elem_type>::result
op_min::min(const Base<typename T1::elem_type,T1>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const Proxy<T1> P(X.get_ref());
  
  const uword n_elem = P.get_n_elem();
  
  arma_debug_check( (n_elem == 0), "min(): given object has no elements" );
  
  eT min_val = priv::most_pos<eT>();
  
  if(Proxy<T1>::prefer_at_accessor == false)
    {
    typedef typename Proxy<T1>::ea_type ea_type;
    
    ea_type A = P.get_ea();
    
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      const eT tmp_i = A[i];
      const eT tmp_j = A[j];
      
      if(tmp_i < min_val) { min_val = tmp_i; }
      if(tmp_j < min_val) { min_val = tmp_j; }
      }
    
    if(i < n_elem)
      {
      const eT tmp_i = A[i];
      
      if(tmp_i < min_val) { min_val = tmp_i; }
      }
    }
  else
    {
    const uword n_rows = P.get_n_rows();
    const uword n_cols = P.get_n_cols();
    
    if(n_rows == 1)
      {
      uword i,j;
      for(i=0, j=1; j < n_cols; i+=2, j+=2)
        {
        const eT tmp_i = P.at(0,i);
        const eT tmp_j = P.at(0,j);
        
        if(tmp_i < min_val) { min_val = tmp_i; }
        if(tmp_j < min_val) { min_val = tmp_j; }
        }
      
      if(i < n_cols)
        {
        const eT tmp_i = P.at(0,i);
        
        if(tmp_i < min_val) { min_val = tmp_i; }
        }
      }
    else
      {
      for(uword col=0; col < n_cols; ++col)
        {
        uword i,j;
        for(i=0, j=1; j < n_rows; i+=2, j+=2)
          {
          const eT tmp_i = P.at(i,col);
          const eT tmp_j = P.at(j,col);
          
          if(tmp_i < min_val) { min_val = tmp_i; }
          if(tmp_j < min_val) { min_val = tmp_j; }
          }
          
        if(i < n_rows)
          {
          const eT tmp_i = P.at(i,col);
          
          if(tmp_i < min_val) { min_val = tmp_i; }
          }
        }
      }
    }
  
  return min_val;
  }



template<typename T>
inline 
std::complex<T>
op_min::direct_min(const std::complex<T>* const X, const uword n_elem)
  {
  arma_extra_debug_sigprint();
  
  uword index   = 0;
  T     min_val = priv::most_pos<T>();
  
  for(uword i=0; i<n_elem; ++i)
    {
    const T tmp_val = std::abs(X[i]);
    
    if(tmp_val < min_val)
      {
      min_val = tmp_val;
      index   = i;
      }
    }
  
  return X[index];
  }



template<typename T>
inline 
std::complex<T>
op_min::direct_min(const std::complex<T>* const X, const uword n_elem, uword& index_of_min_val)
  {
  arma_extra_debug_sigprint();
  
  uword index   = 0;
  T     min_val = priv::most_pos<T>();
  
  for(uword i=0; i<n_elem; ++i)
    {
    const T tmp_val = std::abs(X[i]);
    
    if(tmp_val < min_val)
      {
      min_val = tmp_val;
      index   = i;
      }
    }
  
  index_of_min_val = index;
  
  return X[index];
  }



template<typename T>
inline 
std::complex<T>
op_min::direct_min(const Mat< std::complex<T> >& X, const uword row)
  {
  arma_extra_debug_sigprint();
  
  const uword X_n_cols = X.n_cols;
  
  uword index   = 0;
  T   min_val = priv::most_pos<T>();
  
  for(uword col=0; col<X_n_cols; ++col)
    {
    const T tmp_val = std::abs(X.at(row,col));
    
    if(tmp_val < min_val)
      {
      min_val = tmp_val;
      index   = col;
      }
    }
  
  return X.at(row,index);
  }



template<typename T>
inline
std::complex<T>
op_min::min(const subview< std::complex<T> >& X)
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( (X.n_elem == 0), "min(): given object has no elements" );
  
  const Mat< std::complex<T> >& A = X.m;
  
  const uword X_n_rows = X.n_rows;
  const uword X_n_cols = X.n_cols;
  
  const uword start_row = X.aux_row1;
  const uword start_col = X.aux_col1;
  
  const uword end_row_p1 = start_row + X_n_rows;
  const uword end_col_p1 = start_col + X_n_cols;
  
  T min_val = priv::most_pos<T>();
  
  uword best_row = 0;
  uword best_col = 0;
    
  if(X_n_rows == 1)
    {
    best_col = 0;
    
    for(uword col=start_col; col < end_col_p1; ++col)
      {
      const T tmp_val = std::abs( A.at(start_row, col) );
      
      if(tmp_val < min_val)
        {
        min_val  = tmp_val;
        best_col = col;
        }
      }
    
    best_row = start_row;
    }
  else
    {
    for(uword col=start_col; col < end_col_p1; ++col)
    for(uword row=start_row; row < end_row_p1; ++row)
      {
      const T tmp_val = std::abs( A.at(row, col) );
      
      if(tmp_val < min_val)
        {
        min_val  = tmp_val;
        best_row = row;
        best_col = col;
        }
      }
    }
  
  return A.at(best_row, best_col);
  }



template<typename T1>
inline
typename arma_cx_only<typename T1::elem_type>::result
op_min::min(const Base<typename T1::elem_type,T1>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type            eT;
  typedef typename get_pod_type<eT>::result T;
  
  const Proxy<T1> P(X.get_ref());
  
  const uword n_elem = P.get_n_elem();
  
  arma_debug_check( (n_elem == 0), "min(): given object has no elements" );
  
  T min_val = priv::most_pos<T>();
  
  if(Proxy<T1>::prefer_at_accessor == false)
    {
    typedef typename Proxy<T1>::ea_type ea_type;
    
    ea_type A = P.get_ea();
    
    uword index = 0;
    
    for(uword i=0; i<n_elem; ++i)
      {
      const T tmp = std::abs(A[i]);
      
      if(tmp < min_val)
        {
        min_val = tmp;
        index   = i;
        }
      }
    
    return( A[index] );
    }
  else
    {
    const uword n_rows = P.get_n_rows();
    const uword n_cols = P.get_n_cols();
    
    uword best_row = 0;
    uword best_col = 0;
    
    if(n_rows == 1)
      {
      for(uword col=0; col < n_cols; ++col)
        {
        const T tmp = std::abs(P.at(0,col));
        
        if(tmp < min_val)
          {
          min_val  = tmp;
          best_col = col;
          }
        }
      }
    else
      {
      for(uword col=0; col < n_cols; ++col)
      for(uword row=0; row < n_rows; ++row)
        {
        const T tmp = std::abs(P.at(row,col));
        
        if(tmp < min_val)
          {
          min_val = tmp;
          
          best_row = row;
          best_col = col;
          }
        }
      }
    
    return P.at(best_row, best_col);
    }
  }



//! @}
