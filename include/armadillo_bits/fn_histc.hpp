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



template<typename T1, typename T2>
arma_inline
typename
enable_if2
  <
  (is_arma_type<T1>::value) && (is_arma_type<T2>::value) && (is_not_complex<typename T1::elem_type>::value) && (is_same_type<typename T1::elem_type, typename T2::elem_type>::value),
  const mtGlue<uword,T1,T2,glue_histc_default>
  >::result
histc(const T1& X, const T2& Y)
  {
  arma_extra_debug_sigprint();
  
  return mtGlue<uword,T1,T2,glue_histc_default>(X, Y);
  }



template<typename T1, typename T2>
arma_inline
typename
enable_if2
  <
  (is_arma_type<T1>::value) && (is_arma_type<T2>::value) && (is_not_complex<typename T1::elem_type>::value) && (is_same_type<typename T1::elem_type, typename T2::elem_type>::value),
  const mtGlue<uword,T1,T2,glue_histc>
  >::result
histc(const T1& X, const T2& Y, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  return mtGlue<uword,T1,T2,glue_histc>(X, Y, dim);
  }
