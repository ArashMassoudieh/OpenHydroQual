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


// Copyright (C) 2014-2015 Conrad Sanderson
// Copyright (C) 2014-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



//! \addtogroup op_normalise
//! @{



template<typename T1>
inline
void
op_normalise_vec::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_normalise_vec>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::pod_type T;
  
  const uword p = in.aux_uword_a;
  
  arma_debug_check( (p == 0), "normalise(): parameter 'p' must be greater than zero" );
  
  const quasi_unwrap<T1> tmp(in.m);
  
  const T norm_val_a = norm(tmp.M, p);
  const T norm_val_b = (norm_val_a != T(0)) ? norm_val_a : T(1);
  
  out = tmp.M / norm_val_b;
  }



template<typename T1>
inline
void
op_normalise_mat::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_normalise_mat>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const uword p   = in.aux_uword_a;
  const uword dim = in.aux_uword_b;
  
  arma_debug_check( (p   == 0), "normalise(): parameter 'p' must be greater than zero" );
  arma_debug_check( (dim >  1), "normalise(): parameter 'dim' must be 0 or 1"          );
  
  const unwrap<T1>   tmp(in.m);
  const Mat<eT>& A = tmp.M;
  
  const bool alias = ( (&out) == (&A) );
  
  if(alias)
    {
    Mat<eT> out2;
    
    op_normalise_mat::apply(out2, A, p, dim);
    
    out.steal_mem(out2);
    }
  else
    {
    op_normalise_mat::apply(out, A, p, dim);
    }
  }



template<typename eT>
inline
void
op_normalise_mat::apply(Mat<eT>& out, const Mat<eT>& A, const uword p, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  typedef typename get_pod_type<eT>::result T;
  
  out.copy_size(A);
  
  if(A.n_elem == 0)  { return; }
  
  if(dim == 0)
    {
    const uword n_cols = A.n_cols;
    
    for(uword i=0; i<n_cols; ++i)
      {
      const T norm_val_a = norm(A.col(i), p);
      const T norm_val_b = (norm_val_a != T(0)) ? norm_val_a : T(1);
      
      out.col(i) = A.col(i) / norm_val_b;
      }
    }
  else
    {
    // better-than-nothing implementation
    
    const uword n_rows = A.n_rows;
    
    for(uword i=0; i<n_rows; ++i)
      {
      const T norm_val_a = norm(A.row(i), p);
      const T norm_val_b = (norm_val_a != T(0)) ? norm_val_a : T(1);
      
      out.row(i) = A.row(i) / norm_val_b;
      }
    }
  }



//! @}
