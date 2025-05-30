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


// Copyright (C) 2011-2012 Conrad Sanderson
// Copyright (C) 2011-2012 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_symmat
//! @{



template<typename T1>
inline
void
op_symmat::apply
  (
        Mat<typename T1::elem_type>& out,
  const Op<T1,op_symmat>&            in,
  const typename arma_not_cx<typename T1::elem_type>::result* junk
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::elem_type eT;
  
  const unwrap<T1>   tmp(in.m);
  const Mat<eT>& A = tmp.M;
  
  arma_debug_check( (A.is_square() == false), "symmatu()/symmatl(): given matrix must be square" );
  
  const uword N     = A.n_rows;
  const bool  upper = (in.aux_uword_a == 0);
  
  if(&out != &A)
    {
    out.copy_size(A);
    
    if(upper)
      {
      // upper triangular: copy the diagonal and the elements above the diagonal
      
      for(uword i=0; i<N; ++i)
        {
        const eT* A_data   = A.colptr(i);
              eT* out_data = out.colptr(i);
        
        arrayops::copy( out_data, A_data, i+1 );
        }
      }
    else
      {
      // lower triangular: copy the diagonal and the elements below the diagonal
      
      for(uword i=0; i<N; ++i)
        {
        const eT* A_data   = A.colptr(i);
              eT* out_data = out.colptr(i);
        
        arrayops::copy( &out_data[i], &A_data[i], N-i );
        }
      }
    }
  
  
  if(upper)
    {
    // reflect elements across the diagonal from upper triangle to lower triangle
    
    for(uword col=1; col < N; ++col)
      {
      const eT* coldata = out.colptr(col);
      
      for(uword row=0; row < col; ++row)
        {
        out.at(col,row) = coldata[row];
        }
      }
    }
  else
    {
    // reflect elements across the diagonal from lower triangle to upper triangle
    
    for(uword col=0; col < N; ++col)
      {
      const eT* coldata = out.colptr(col);
      
      for(uword row=(col+1); row < N; ++row)
        {
        out.at(col,row) = coldata[row];
        }
      }
    }
  }



template<typename T1>
inline
void
op_symmat::apply
  (
        Mat<typename T1::elem_type>& out,
  const Op<T1,op_symmat>&            in,
  const typename arma_cx_only<typename T1::elem_type>::result* junk
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::elem_type eT;
  
  const unwrap<T1>   tmp(in.m);
  const Mat<eT>& A = tmp.M;
  
  arma_debug_check( (A.is_square() == false), "symmatu()/symmatl(): given matrix must be square" );
  
  const uword N     = A.n_rows;
  const bool  upper = (in.aux_uword_a == 0);
  
  if(&out != &A)
    {
    out.copy_size(A);
    
    if(upper)
      {
      // upper triangular: copy the diagonal and the elements above the diagonal
      
      for(uword i=0; i<N; ++i)
        {
        const eT* A_data   = A.colptr(i);
              eT* out_data = out.colptr(i);
        
        arrayops::copy( out_data, A_data, i+1 );
        }
      }
    else
      {
      // lower triangular: copy the diagonal and the elements below the diagonal
      
      for(uword i=0; i<N; ++i)
        {
        const eT* A_data   = A.colptr(i);
              eT* out_data = out.colptr(i);
        
        arrayops::copy( &out_data[i], &A_data[i], N-i );
        }
      }
    }
  
  
  if(upper)
    {
    // reflect elements across the diagonal from upper triangle to lower triangle
    
    for(uword col=1; col < N; ++col)
      {
      const eT* coldata = out.colptr(col);
      
      for(uword row=0; row < col; ++row)
        {
        out.at(col,row) = std::conj(coldata[row]);
        }
      }
    }
  else
    {
    // reflect elements across the diagonal from lower triangle to upper triangle
    
    for(uword col=0; col < N; ++col)
      {
      const eT* coldata = out.colptr(col);
      
      for(uword row=(col+1); row < N; ++row)
        {
        out.at(col,row) = std::conj(coldata[row]);
        }
      }
    }
  }



//! @}
