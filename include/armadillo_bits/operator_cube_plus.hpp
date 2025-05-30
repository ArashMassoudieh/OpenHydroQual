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


//! \addtogroup operator_cube_plus
//! @{



//! unary plus operation (does nothing, but is required for completeness)
template<typename T1>
arma_inline
const BaseCube<typename T1::elem_type,T1>&
operator+
  (
  const BaseCube<typename T1::elem_type,T1>& X
  )
  {
  arma_extra_debug_sigprint();
  
  return X;
  }



//! BaseCube + scalar
template<typename T1>
arma_inline
const eOpCube<T1, eop_scalar_plus>
operator+
  (
  const BaseCube<typename T1::elem_type,T1>& X,
  const typename T1::elem_type               k
  )
  {
  arma_extra_debug_sigprint();
  
  return eOpCube<T1, eop_scalar_plus>(X.get_ref(), k);
  }



//! scalar + BaseCube
template<typename T1>
arma_inline
const eOpCube<T1, eop_scalar_plus>
operator+
  (
  const typename T1::elem_type               k,
  const BaseCube<typename T1::elem_type,T1>& X
  )
  {
  arma_extra_debug_sigprint();
  
  return eOpCube<T1, eop_scalar_plus>(X.get_ref(), k);
  }



//! non-complex BaseCube + complex scalar (experimental)
template<typename T1>
arma_inline
const mtOpCube<typename std::complex<typename T1::pod_type>, T1, op_cx_scalar_plus>
operator+
  (
  const BaseCube<typename T1::pod_type, T1>& X,
  const std::complex<typename T1::pod_type>& k
  )
  {
  arma_extra_debug_sigprint();
  
  return mtOpCube<typename std::complex<typename T1::pod_type>, T1, op_cx_scalar_plus>('j', X.get_ref(), k);
  }



//! complex scalar + non-complex BaseCube (experimental)
template<typename T1>
arma_inline
const mtOpCube<typename std::complex<typename T1::pod_type>, T1, op_cx_scalar_plus>
operator+
  (
  const std::complex<typename T1::pod_type>& k,
  const BaseCube<typename T1::pod_type, T1>& X
  )
  {
  arma_extra_debug_sigprint();
  
  return mtOpCube<typename std::complex<typename T1::pod_type>, T1, op_cx_scalar_plus>('j', X.get_ref(), k);  // NOTE: order is swapped
  }



//! addition of BaseCube objects with same element type
template<typename T1, typename T2>
arma_inline
const eGlueCube<T1, T2, eglue_plus>
operator+
  (
  const BaseCube<typename T1::elem_type,T1>& X,
  const BaseCube<typename T1::elem_type,T2>& Y
  )
  {
  arma_extra_debug_sigprint();
  
  return eGlueCube<T1, T2, eglue_plus>(X.get_ref(), Y.get_ref());
  }



//! addition of BaseCube objects with different element types
template<typename T1, typename T2>
inline
const mtGlueCube<typename promote_type<typename T1::elem_type, typename T2::elem_type>::result, T1, T2, glue_mixed_plus>
operator+
  (
  const BaseCube< typename force_different_type<typename T1::elem_type, typename T2::elem_type>::T1_result, T1>& X,
  const BaseCube< typename force_different_type<typename T1::elem_type, typename T2::elem_type>::T2_result, T2>& Y
  )
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT1;
  typedef typename T2::elem_type eT2;
  
  typedef typename promote_type<eT1,eT2>::result out_eT;
  
  promote_type<eT1,eT2>::check();
  
  return mtGlueCube<out_eT, T1, T2, glue_mixed_plus>( X.get_ref(), Y.get_ref() );
  }



template<typename eT, typename T2>
arma_inline
Cube<eT>
operator+
  (
  const subview_cube_each1<eT>& X,
  const Base<eT,T2>&            Y
  )
  {
  arma_extra_debug_sigprint();
  
  return subview_cube_each1_aux::operator_plus(X, Y.get_ref());
  }



template<typename T1, typename eT>
arma_inline
Cube<eT>
operator+
  (
  const Base<eT,T1>&            X,
  const subview_cube_each1<eT>& Y
  )
  {
  arma_extra_debug_sigprint();
  
  return subview_cube_each1_aux::operator_plus(Y, X.get_ref());  // NOTE: swapped order
  }



template<typename eT, typename TB, typename T2>
arma_inline
Cube<eT>
operator+
  (
  const subview_cube_each2<eT,TB>& X,
  const Base<eT,T2>&               Y
  )
  {
  arma_extra_debug_sigprint();
  
  return subview_cube_each2_aux::operator_plus(X, Y.get_ref());
  }



template<typename T1, typename eT, typename TB>
arma_inline
Cube<eT>
operator+
  (
  const Base<eT,T1>&               X,
  const subview_cube_each2<eT,TB>& Y
  )
  {
  arma_extra_debug_sigprint();
  
  return subview_cube_each2_aux::operator_plus(Y, X.get_ref());  // NOTE: swapped order
  }



//! @}
