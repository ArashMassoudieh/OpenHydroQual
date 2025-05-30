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


//! \addtogroup op_sort
//! @{



template<typename eT>
inline 
void
op_sort::direct_sort(eT* X, const uword n_elem, const uword sort_type)
  {
  arma_extra_debug_sigprint();
  
  if(sort_type == 0)
    {
    arma_ascend_sort_helper<eT> comparator;
    
    std::sort(&X[0], &X[n_elem], comparator);
    }
  else
    {
    arma_descend_sort_helper<eT> comparator;
    
    std::sort(&X[0], &X[n_elem], comparator);
    }
  }



template<typename eT>
inline 
void
op_sort::direct_sort_ascending(eT* X, const uword n_elem)
  {
  arma_extra_debug_sigprint();
  
  arma_ascend_sort_helper<eT> comparator;
    
  std::sort(&X[0], &X[n_elem], comparator);
  }



template<typename eT>
inline 
void
op_sort::copy_row(eT* X, const Mat<eT>& A, const uword row)
  {
  const uword N = A.n_cols;
  
  uword i,j;
  
  for(i=0, j=1; j<N; i+=2, j+=2)
    {
    X[i] = A.at(row,i);
    X[j] = A.at(row,j);
    }
  
  if(i < N)
    {
    X[i] = A.at(row,i);
    }
  }



template<typename eT>
inline 
void
op_sort::copy_row(Mat<eT>& A, const eT* X, const uword row)
  {
  const uword N = A.n_cols;
  
  uword i,j;
  
  for(i=0, j=1; j<N; i+=2, j+=2)
    {
    A.at(row,i) = X[i]; 
    A.at(row,j) = X[j];
    }
  
  if(i < N)
    {
    A.at(row,i) = X[i];
    }
  }



template<typename eT>
inline
void
op_sort::apply_noalias(Mat<eT>& out, const Mat<eT>& X, const uword sort_type, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  if( (X.n_rows * X.n_cols) <= 1 )
    {
    out = X;
    return;
    }
  
  arma_debug_check( (sort_type > 1), "sort(): parameter 'sort_type' must be 0 or 1" );
  arma_debug_check( (X.has_nan()),   "sort(): detected NaN"                         );
  
  if(dim == 0)  // sort the contents of each column
    {
    arma_extra_debug_print("op_sort::apply(): dim = 0");
    
    out = X;
    
    const uword n_rows = out.n_rows;
    const uword n_cols = out.n_cols;
      
    for(uword col=0; col < n_cols; ++col)
      {
      op_sort::direct_sort( out.colptr(col), n_rows, sort_type );
      }
    }
  else
  if(dim == 1)  // sort the contents of each row
    {
    if(X.n_rows == 1)  // a row vector
      {
      arma_extra_debug_print("op_sort::apply(): dim = 1, vector specific");
      
      out = X;
      op_sort::direct_sort(out.memptr(), out.n_elem, sort_type);
      }
    else  // not a row vector
      {
      arma_extra_debug_print("op_sort::apply(): dim = 1, generic");
      
      out.copy_size(X);
      
      const uword n_rows = out.n_rows;
      const uword n_cols = out.n_cols;
      
      podarray<eT> tmp_array(n_cols);
      
      for(uword row=0; row < n_rows; ++row)
        {
        op_sort::copy_row(tmp_array.memptr(), X, row);
        
        op_sort::direct_sort( tmp_array.memptr(), n_cols, sort_type );
        
        op_sort::copy_row(out, tmp_array.memptr(), row);
        }
      }
    }
  }



template<typename T1>
inline
void
op_sort::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_sort>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const quasi_unwrap<T1> U(in.m);
  
  const Mat<eT>& X = U.M;
  
  const uword sort_type = in.aux_uword_a;
  const uword dim       = in.aux_uword_b;
  
  if(U.is_alias(out))
    {
    Mat<eT> tmp;
    
    op_sort::apply_noalias(tmp, X, sort_type, dim);
    
    out.steal_mem(tmp);
    }
  else
    {
    op_sort::apply_noalias(out, X, sort_type, dim);
    }
  }



template<typename T1>
inline
void
op_sort_default::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_sort_default>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const quasi_unwrap<T1> U(in.m);
  
  const Mat<eT>& X = U.M;
  
  const uword sort_type = in.aux_uword_a;
  const uword dim       = (T1::is_row) ? 1 : 0;
  
  if(U.is_alias(out))
    {
    Mat<eT> tmp;
    
    op_sort::apply_noalias(tmp, X, sort_type, dim);
    
    out.steal_mem(tmp);
    }
  else
    {
    op_sort::apply_noalias(out, X, sort_type, dim);
    }
  }



//! @}
