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


// Copyright (C) 2009-2012 Conrad Sanderson
// Copyright (C) 2009-2012 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_mean
//! @{


//! Class for finding mean values of a matrix
class op_mean
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<typename T1::elem_type>& out, const Op<T1,op_mean>& in);
  
  template<typename T1>
  inline static void apply_noalias(Mat<typename T1::elem_type>& out, const Proxy<T1>& P, const uword dim);
  
  template<typename T1>
  inline static void apply_noalias_unwrap(Mat<typename T1::elem_type>& out, const Proxy<T1>& P, const uword dim);
  
  template<typename T1>
  inline static void apply_noalias_proxy(Mat<typename T1::elem_type>& out, const Proxy<T1>& P, const uword dim);
  
  //
  
  template<typename eT>
  inline static eT direct_mean(const eT* const X, const uword N);
  
  template<typename eT>
  inline static eT direct_mean_robust(const eT* const X, const uword N);
  

  //
  
  template<typename eT>
  inline static eT direct_mean(const Mat<eT>& X, const uword row);
  
  template<typename eT>
  inline static eT direct_mean_robust(const Mat<eT>& X, const uword row);
  

  //
  
  template<typename eT>
  inline static eT mean_all(const subview<eT>& X);
  
  template<typename eT>
  inline static eT mean_all_robust(const subview<eT>& X);
  
  
  //
  
  template<typename eT>
  inline static eT mean_all(const diagview<eT>& X);
  
  template<typename eT>
  inline static eT mean_all_robust(const diagview<eT>& X);
  
  
  //
  
  template<typename T1>
  inline static typename T1::elem_type mean_all(const Base<typename T1::elem_type, T1>& X);
  
  
  //
  
  template<typename eT>
  arma_inline static eT robust_mean(const eT A, const eT B);
  
  template<typename T>
  arma_inline static std::complex<T> robust_mean(const std::complex<T>& A, const std::complex<T>& B);
  };



//! @}
