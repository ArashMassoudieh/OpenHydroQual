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
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



//! \addtogroup op_hist
//! @{



template<typename eT>
inline
void
op_hist::apply_noalias(Mat<uword>& out, const Mat<eT>& A, const uword n_bins, const bool A_is_row)
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( ((A.is_vec() == false) && (A.is_empty() == false)), "hist(): only vectors are supported when automatically determining bin centers" );
  
  if(n_bins == 0)  { out.reset(); return; }
  
        uword A_n_elem = A.n_elem;
  const eT*   A_mem    = A.memptr();
  
  eT min_val = priv::most_pos<eT>();
  eT max_val = priv::most_neg<eT>();
  
  uword i,j;
  for(i=0, j=1; j < A_n_elem; i+=2, j+=2)
    {
    const eT val_i = A_mem[i];
    const eT val_j = A_mem[j];
    
    if(min_val > val_i) { min_val = val_i; }
    if(min_val > val_j) { min_val = val_j; }
      
    if(max_val < val_i) { max_val = val_i; }
    if(max_val < val_j) { max_val = val_j; }
    }
  
  if(i < A_n_elem)
    {
    const eT val_i = A_mem[i];
    
    if(min_val > val_i) { min_val = val_i; }
    if(max_val < val_i) { max_val = val_i; }
    }
  
  if(arma_isfinite(min_val) == false) { min_val = priv::most_neg<eT>(); }
  if(arma_isfinite(max_val) == false) { max_val = priv::most_pos<eT>(); }
  
  Col<eT> c(n_bins);
  eT* c_mem = c.memptr();
  
  for(uword ii=0; ii < n_bins; ++ii)
    {
    c_mem[ii] = (0.5 + ii) / double(n_bins);   // TODO: may need to be modified for integer matrices
    }
  
  c = ((max_val - min_val) * c) + min_val;
  
  const uword dim = (A_is_row) ? 1 : 0;
  
  glue_hist::apply_noalias(out, A, c, dim);
  }



template<typename T1>
inline
void
op_hist::apply(Mat<uword>& out, const mtOp<uword, T1, op_hist>& X)
  {
  arma_extra_debug_sigprint();
  
  const uword n_bins = X.aux_uword_a;
  
  const quasi_unwrap<T1> U(X.m);
  
  if(U.is_alias(out))
    {
    Mat<uword> tmp;
    
    op_hist::apply_noalias(tmp, U.M, n_bins, (T1::is_row));
    
    out.steal_mem(tmp);
    }
  else
    {
    op_hist::apply_noalias(out, U.M, n_bins, (T1::is_row));
    }
  }



//! @}
