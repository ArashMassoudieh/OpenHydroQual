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


// Copyright (C) 2008-2010 Conrad Sanderson
// Copyright (C) 2008-2010 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_dotext
//! @{



template<typename eT>
inline
eT
op_dotext::direct_rowvec_mat_colvec
  (
  const eT*      A_mem,
  const Mat<eT>& B,
  const eT*      C_mem
  )
  {
  arma_extra_debug_sigprint();
  
  const uword cost_AB = B.n_cols;
  const uword cost_BC = B.n_rows;
  
  if(cost_AB <= cost_BC)
    {
    podarray<eT> tmp(B.n_cols);
    
    for(uword col=0; col<B.n_cols; ++col)
      {
      const eT* B_coldata = B.colptr(col);
      
      eT val = eT(0);
      for(uword i=0; i<B.n_rows; ++i)
        {
        val += A_mem[i] * B_coldata[i];
        }
        
      tmp[col] = val;
      }
    
    return op_dot::direct_dot(B.n_cols, tmp.mem, C_mem);
    }
  else
    {
    podarray<eT> tmp(B.n_rows);
    
    for(uword row=0; row<B.n_rows; ++row)
      {
      eT val = eT(0);
      for(uword col=0; col<B.n_cols; ++col)
        {
        val += B.at(row,col) * C_mem[col];
        }
      
      tmp[row] = val;
      }
    
    return op_dot::direct_dot(B.n_rows, A_mem, tmp.mem);
    }
  
  
  }



template<typename eT>
inline
eT
op_dotext::direct_rowvec_transmat_colvec
  (
  const eT*      A_mem,
  const Mat<eT>& B,
  const eT*      C_mem
  )
  {
  arma_extra_debug_sigprint();
  
  const uword cost_AB = B.n_rows;
  const uword cost_BC = B.n_cols;
  
  if(cost_AB <= cost_BC)
    {
    podarray<eT> tmp(B.n_rows);
    
    for(uword row=0; row<B.n_rows; ++row)
      {
      eT val = eT(0);
      
      for(uword i=0; i<B.n_cols; ++i)
        {
        val += A_mem[i] * B.at(row,i);
        }
        
      tmp[row] = val;
      }
    
    return op_dot::direct_dot(B.n_rows, tmp.mem, C_mem);
    }
  else
    {
    podarray<eT> tmp(B.n_cols);
    
    for(uword col=0; col<B.n_cols; ++col)
      {
      const eT* B_coldata = B.colptr(col);
      
      eT val = eT(0);
      
      for(uword i=0; i<B.n_rows; ++i)
        {
        val += B_coldata[i] * C_mem[i];
        }
      
      tmp[col] = val;
      }
    
    return op_dot::direct_dot(B.n_cols, A_mem, tmp.mem);
    }
  
  
  }



template<typename eT>
inline
eT
op_dotext::direct_rowvec_diagmat_colvec
  (
  const eT*      A_mem,
  const Mat<eT>& B,
  const eT*      C_mem
  )
  {
  arma_extra_debug_sigprint();
  
  eT val = eT(0);

  for(uword i=0; i<B.n_rows; ++i)
    {
    val += A_mem[i] * B.at(i,i) * C_mem[i];
    }

  return val;
  }



template<typename eT>
inline
eT
op_dotext::direct_rowvec_invdiagmat_colvec
  (
  const eT*      A_mem,
  const Mat<eT>& B,
  const eT*      C_mem
  )
  {
  arma_extra_debug_sigprint();
  
  eT val = eT(0);

  for(uword i=0; i<B.n_rows; ++i)
    {
    val += (A_mem[i] * C_mem[i]) / B.at(i,i);
    }

  return val;
  }



template<typename eT>
inline
eT
op_dotext::direct_rowvec_invdiagvec_colvec
  (
  const eT*      A_mem,
  const Mat<eT>& B,
  const eT*      C_mem
  )
  {
  arma_extra_debug_sigprint();
  
  const eT* B_mem = B.mem;
  
  eT val = eT(0);

  for(uword i=0; i<B.n_elem; ++i)
    {
    val += (A_mem[i] * C_mem[i]) / B_mem[i];
    }

  return val;
  }



//! @}
