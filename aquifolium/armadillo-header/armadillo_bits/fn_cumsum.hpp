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


// Copyright (C) 2010-2012 Conrad Sanderson
// Copyright (C) 2010-2012 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_cumsum
//! @{



template<typename T1>
arma_inline
const Op<T1, op_cumsum_mat>
cumsum
  (
  const T1& X,
  const uword dim = 0,
  const typename enable_if< is_arma_type<T1>::value       == true  >::result* junk1 = 0,
  const typename enable_if< resolves_to_vector<T1>::value == false >::result* junk2 = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk1);
  arma_ignore(junk2);
  
  return Op<T1, op_cumsum_mat>(X, dim, 0);
  }



template<typename T1>
arma_inline
const Op<T1, op_cumsum_mat>
cumsum
  (
  const T1& X,
  const uword dim,
  const typename enable_if<resolves_to_vector<T1>::value == true>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  return Op<T1, op_cumsum_mat>(X, dim, 0);
  }



template<typename T1>
arma_inline
const Op<T1, op_cumsum_vec>
cumsum
  (
  const T1& X,
  const arma_empty_class junk1 = arma_empty_class(),
  const typename enable_if<resolves_to_vector<T1>::value == true>::result* junk2 = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk1);
  arma_ignore(junk2);
  
  return Op<T1, op_cumsum_vec>(X);
  }



template<typename T1>
arma_inline
const Op<Op<T1, op_cumsum_vec>, op_cumsum_mat>
cumsum
  (
  const Op<T1, op_cumsum_vec>& X,
  const uword dim,
  const typename enable_if<resolves_to_vector<T1>::value == true>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  return Op<Op<T1, op_cumsum_vec>, op_cumsum_mat>(X, dim, 0);
  }



template<typename T1>
arma_inline
const Op<Op<T1, op_cumsum_vec>, op_cumsum_vec>
cumsum
  (
  const Op<T1, op_cumsum_vec>& X,
  const arma_empty_class junk1 = arma_empty_class(),
  const typename enable_if<resolves_to_vector<T1>::value == true>::result* junk2 = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk1);
  arma_ignore(junk2);
  
  return Op<Op<T1, op_cumsum_vec>, op_cumsum_vec>(X);
  }



template<typename T>
arma_inline
arma_warn_unused
const typename arma_scalar_only<T>::result &
cumsum(const T& x)
  {
  return x;
  }



//! @}
