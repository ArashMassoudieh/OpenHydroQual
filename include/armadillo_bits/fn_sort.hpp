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


//! \addtogroup fn_sort
//! @{


template<typename T1>
arma_inline
typename
enable_if2
  <
  (is_arma_type<T1>::value),
  const Op<T1, op_sort_default>
  >::result
sort
  (
  const T1&   X,
  const uword sort_type = 0
  )
  {
  arma_extra_debug_sigprint();
  
  return Op<T1, op_sort_default>(X, sort_type, 0);
  }



//! kept for compatibility with old code
template<typename T1>
arma_inline
typename
enable_if2
  <
  (is_arma_type<T1>::value),
  const Op<T1, op_sort>
  >::result
sort
  (
  const T1&   X,
  const uword sort_type,
  const uword dim
  )
  {
  arma_extra_debug_sigprint();
  
  return Op<T1, op_sort>(X, sort_type, dim);
  }



template<typename T1, typename T2>
arma_inline
typename
enable_if2
  <
  ( (is_arma_type<T1>::value) && (is_same_type<T2, char>::value) ),
  const Op<T1, op_sort_default>
  >::result
sort
  (
  const T1&   X,
  const T2*   sort_direction
  )
  {
  arma_extra_debug_sigprint();
  
  const char sig = (sort_direction != NULL) ? sort_direction[0] : char(0);
  
  arma_debug_check( (sig != 'a') && (sig != 'd'), "sort(): unknown sort direction");
  
  const uword sort_type = (sig == 'a') ? 0 : 1;
  
  return Op<T1, op_sort_default>(X, sort_type, 0);
  }



template<typename T1, typename T2>
arma_inline
typename
enable_if2
  <
  ( (is_arma_type<T1>::value) && (is_same_type<T2, char>::value) ),
  const Op<T1, op_sort>
  >::result
sort
  (
  const T1&   X,
  const T2*   sort_direction,
  const uword dim
  )
  {
  arma_extra_debug_sigprint();
  
  const char sig = (sort_direction != NULL) ? sort_direction[0] : char(0);
  
  arma_debug_check( (sig != 'a') && (sig != 'd'), "sort(): unknown sort direction");
  
  const uword sort_type = (sig == 'a') ? 0 : 1;
  
  return Op<T1, op_sort>(X, sort_type, dim);
  }



//! @}
