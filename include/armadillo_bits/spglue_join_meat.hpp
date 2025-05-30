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
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup spglue_join
//! @{



template<typename T1, typename T2>
inline
void
spglue_join_cols::apply(SpMat<typename T1::elem_type>& out, const SpGlue<T1,T2,spglue_join_cols>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const unwrap_spmat<T1> A_tmp(X.A);
  const unwrap_spmat<T2> B_tmp(X.B);
  
  const SpMat<eT>& A = A_tmp.M;
  const SpMat<eT>& B = B_tmp.M;
  
  if( (&out != &A) && (&out != &B) )
    {
    spglue_join_cols::apply_noalias(out, A, B);
    }
  else
    {
    SpMat<eT> tmp;
    
    spglue_join_cols::apply_noalias(tmp, A, B);
    
    out.steal_mem(tmp);
    }
  }



template<typename eT>
inline
void
spglue_join_cols::apply_noalias(SpMat<eT>& out, const SpMat<eT>& A, const SpMat<eT>& B)
  {
  arma_extra_debug_sigprint();
  
  const uword A_n_rows = A.n_rows;
  const uword A_n_cols = A.n_cols;
  
  const uword B_n_rows = B.n_rows;
  const uword B_n_cols = B.n_cols;
  
  arma_debug_check
    (
    ( (A_n_cols != B_n_cols) && ( (A_n_rows > 0) || (A_n_cols > 0) ) && ( (B_n_rows > 0) || (B_n_cols > 0) ) ),
    "join_cols() / join_vert(): number of columns must be the same"
    );
  
  out.set_size( A_n_rows + B_n_rows, (std::max)(A_n_cols, B_n_cols) );
  
  if( out.n_elem > 0 )
    {
    if(A.is_empty() == false)
      { 
      out.submat(0,        0,   A_n_rows-1, out.n_cols-1) = A;
      }
    
    if(B.is_empty() == false)
      {
      out.submat(A_n_rows, 0, out.n_rows-1, out.n_cols-1) = B;
      }
    }
  }



template<typename T1, typename T2>
inline
void
spglue_join_rows::apply(SpMat<typename T1::elem_type>& out, const SpGlue<T1,T2,spglue_join_rows>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const unwrap_spmat<T1> A_tmp(X.A);
  const unwrap_spmat<T2> B_tmp(X.B);
  
  const SpMat<eT>& A = A_tmp.M;
  const SpMat<eT>& B = B_tmp.M;
  
  if( (&out != &A) && (&out != &B) )
    {
    spglue_join_rows::apply_noalias(out, A, B);
    }
  else
    {
    SpMat<eT> tmp;
    
    spglue_join_rows::apply_noalias(tmp, A, B);
    
    out.steal_mem(tmp);
    }
  }



template<typename eT>
inline
void
spglue_join_rows::apply_noalias(SpMat<eT>& out, const SpMat<eT>& A, const SpMat<eT>& B)
  {
  arma_extra_debug_sigprint();
  
  const uword A_n_rows = A.n_rows;
  const uword A_n_cols = A.n_cols;
  
  const uword B_n_rows = B.n_rows;
  const uword B_n_cols = B.n_cols;
  
  arma_debug_check
    (
    ( (A_n_rows != B.n_rows) && ( (A_n_rows > 0) || (A_n_cols > 0) ) && ( (B_n_rows > 0) || (B_n_cols > 0) ) ),
    "join_rows() / join_horiz(): number of rows must be the same"
    );
  
  out.set_size( (std::max)(A_n_rows, B_n_rows), A_n_cols + B_n_cols );
  
  if( out.n_elem > 0 )
    {
    if(A.is_empty() == false)
      {
      out.submat(0, 0,        out.n_rows-1,   A.n_cols-1) = A;
      }
    
    if(B.is_empty() == false)
      {
      out.submat(0, A_n_cols, out.n_rows-1, out.n_cols-1) = B;
      }
    }
  }



//! @}
