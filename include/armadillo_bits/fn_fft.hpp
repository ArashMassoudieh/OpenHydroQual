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


// Copyright (C) 2013 Conrad Sanderson
// Copyright (C) 2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_fft
//! @{



// 1D FFT & 1D IFFT



template<typename T1>
inline
typename
enable_if2
  <
  (is_arma_type<T1>::value && is_real<typename T1::elem_type>::value),
  const mtOp<std::complex<typename T1::pod_type>, T1, op_fft_real>
  >::result
fft(const T1& A)
  {
  arma_extra_debug_sigprint();
  
  return mtOp<std::complex<typename T1::pod_type>, T1, op_fft_real>(A, uword(0), uword(1));
  }



template<typename T1>
inline
typename
enable_if2
  <
  (is_arma_type<T1>::value && is_real<typename T1::elem_type>::value),
  const mtOp<std::complex<typename T1::pod_type>, T1, op_fft_real>
  >::result
fft(const T1& A, const uword N)
  {
  arma_extra_debug_sigprint();
  
  return mtOp<std::complex<typename T1::pod_type>, T1, op_fft_real>(A, N, uword(0));
  }



template<typename T1>
inline
typename
enable_if2
  <
  (is_arma_type<T1>::value && is_complex_strict<typename T1::elem_type>::value),
  const Op<T1, op_fft_cx>
  >::result
fft(const T1& A)
  {
  arma_extra_debug_sigprint();
  
  return Op<T1, op_fft_cx>(A, uword(0), uword(1));
  }



template<typename T1>
inline
typename
enable_if2
  <
  (is_arma_type<T1>::value && is_complex_strict<typename T1::elem_type>::value),
  const Op<T1, op_fft_cx>
  >::result
fft(const T1& A, const uword N)
  {
  arma_extra_debug_sigprint();
  
  return Op<T1, op_fft_cx>(A, N, uword(0));
  }



template<typename T1>
inline
typename
enable_if2
  <
  (is_arma_type<T1>::value && is_complex_strict<typename T1::elem_type>::value),
  const Op<T1, op_ifft_cx>
  >::result
ifft(const T1& A)
  {
  arma_extra_debug_sigprint();
  
  return Op<T1, op_ifft_cx>(A, uword(0), uword(1));
  }



template<typename T1>
inline
typename
enable_if2
  <
  (is_arma_type<T1>::value && is_complex_strict<typename T1::elem_type>::value),
  const Op<T1, op_ifft_cx>
  >::result
ifft(const T1& A, const uword N)
  {
  arma_extra_debug_sigprint();
  
  return Op<T1, op_ifft_cx>(A, N, uword(0));
  }



//! @}
