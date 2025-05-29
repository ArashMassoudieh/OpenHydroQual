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
// Copyright (C) 2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_find_unique
//! @{



template<typename T1>
inline
typename
enable_if2
  <
  is_arma_type<T1>::value,
  const mtOp<uword,T1,op_find_unique>
  >::result
find_unique
  (
  const T1&  X,
  const bool ascending_indices = true
  )
  {
  arma_extra_debug_sigprint();
  
  return mtOp<uword,T1,op_find_unique>(X, ((ascending_indices) ? uword(1) : uword(0)), uword(0));
  }



template<typename T1>
inline
uvec
find_unique
  (
  const BaseCube<typename T1::elem_type,T1>& X,
  const bool ascending_indices = true
  )
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const unwrap_cube<T1> tmp(X.get_ref());
  
  const Mat<eT> R( const_cast< eT* >(tmp.M.memptr()), tmp.M.n_elem, 1, false );
  
  return find_unique(R,ascending_indices);
  }



//! @}
