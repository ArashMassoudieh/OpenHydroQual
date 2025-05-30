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


//! \addtogroup op_min
//! @{


class op_min
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<typename T1::elem_type>& out, const Op<T1,op_min>& in);
  
  template<typename eT>
  inline static void apply_noalias(Mat<eT>& out, const Mat<eT>& X, const uword dim, const typename arma_not_cx<eT>::result* junk = 0);
  
  template<typename eT>
  inline static void apply_noalias(Mat<eT>& out, const Mat<eT>& X, const uword dim, const typename arma_cx_only<eT>::result* junk = 0);
  
  
  //
  // for non-complex numbers
  
  template<typename eT>
  inline static eT direct_min(const eT* const X, const uword N);
  
  template<typename eT>
  inline static eT direct_min(const eT* const X, const uword N, uword& index_of_min_val);
  
  template<typename eT>
  inline static eT direct_min(const Mat<eT>& X, const uword row);
  
  template<typename eT>
  inline static eT min(const subview<eT>& X);
  
  template<typename T1>
  inline static typename arma_not_cx<typename T1::elem_type>::result min(const Base<typename T1::elem_type, T1>& X);
  
  template<typename T1>
  inline static typename arma_not_cx<typename T1::elem_type>::result min_with_index(const Proxy<T1>& P, uword& index_of_min_val);
  
  
  //
  // for complex numbers
  
  template<typename T>
  inline static std::complex<T> direct_min(const std::complex<T>* const X, const uword n_elem);
  
  template<typename T>
  inline static std::complex<T> direct_min(const std::complex<T>* const X, const uword n_elem, uword& index_of_min_val);
  
  template<typename T>
  inline static std::complex<T> direct_min(const Mat< std::complex<T> >& X, const uword row);
  
  template<typename T>
  inline static std::complex<T> min(const subview< std::complex<T> >&X);
  
  template<typename T1>
  inline static typename arma_cx_only<typename T1::elem_type>::result min(const Base<typename T1::elem_type, T1>& X);
  
  template<typename T1>
  inline static typename arma_cx_only<typename T1::elem_type>::result min_with_index(const Proxy<T1>& P, uword& index_of_min_val);
  };



//! @}
