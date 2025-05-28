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


// Copyright (C) 2012-2014 Ryan Curtin
// Copyright (C) 2012-2014 Conrad Sanderson
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup spop_htrans
//! @{



template<typename T1>
arma_hot
inline
void
spop_htrans::apply(SpMat<typename T1::elem_type>& out, const SpOp<T1,spop_htrans>& in, const typename arma_not_cx<typename T1::elem_type>::result* junk)
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  spop_strans::apply(out, in);
  }



template<typename T1>
arma_hot
inline
void
spop_htrans::apply(SpMat<typename T1::elem_type>& out, const SpOp<T1,spop_htrans>& in, const typename arma_cx_only<typename T1::elem_type>::result* junk)
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename   T1::elem_type  eT;
  typedef typename umat::elem_type ueT;
  
  const SpProxy<T1> p(in.m);
  
  const uword N = p.get_n_nonzero();
  
  if(N == uword(0))
    {
    out.zeros(p.get_n_cols(), p.get_n_rows());
    return;
    }
  
  umat locs(2, N);
  
  Col<eT> vals(N);
  
  eT* vals_ptr = vals.memptr();
  
  typename SpProxy<T1>::const_iterator_type it = p.begin();
  
  for(uword count = 0; count < N; ++count)
    {
    ueT* locs_ptr = locs.colptr(count);
    
    locs_ptr[0] = it.col();
    locs_ptr[1] = it.row();
    
    vals_ptr[count] = std::conj(*it);
    
    ++it;
    }
  
  SpMat<eT> tmp(locs, vals, p.get_n_cols(), p.get_n_rows());
  
  out.steal_mem(tmp);
  }



//! @}
