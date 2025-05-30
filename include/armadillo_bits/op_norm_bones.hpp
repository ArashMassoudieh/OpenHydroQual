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


//! \addtogroup op_norm
//! @{


class op_norm
  {
  public:
  
  // norms for dense vectors and matrices
  
  template<typename T1> arma_hot inline static typename T1::pod_type vec_norm_1(const Proxy<T1>& P, const typename  arma_not_cx<typename T1::elem_type>::result* junk = 0);
  template<typename T1> arma_hot inline static typename T1::pod_type vec_norm_1(const Proxy<T1>& P, const typename arma_cx_only<typename T1::elem_type>::result* junk = 0);
  template<typename eT> arma_hot inline static eT                    vec_norm_1_direct_std(const Mat<eT>& X);
  template<typename eT> arma_hot inline static eT                    vec_norm_1_direct_mem(const uword N, const eT* A);
  
  template<typename T1> arma_hot inline static typename T1::pod_type vec_norm_2(const Proxy<T1>& P, const typename  arma_not_cx<typename T1::elem_type>::result* junk = 0);
  template<typename T1> arma_hot inline static typename T1::pod_type vec_norm_2(const Proxy<T1>& P, const typename arma_cx_only<typename T1::elem_type>::result* junk = 0);
  template<typename eT> arma_hot inline static eT                    vec_norm_2_direct_std(const Mat<eT>& X);
  template<typename eT> arma_hot inline static eT                    vec_norm_2_direct_mem(const uword N, const eT* A);
  template<typename eT> arma_hot inline static eT                    vec_norm_2_direct_robust(const Mat<eT>& X);
  
  template<typename T1> arma_hot inline static typename T1::pod_type vec_norm_k(const Proxy<T1>& P, const int k);
  
  template<typename T1> arma_hot inline static typename T1::pod_type vec_norm_max(const Proxy<T1>& P);
  template<typename T1> arma_hot inline static typename T1::pod_type vec_norm_min(const Proxy<T1>& P);
  
  template<typename T1> inline static typename T1::pod_type mat_norm_1(const Proxy<T1>& P);
  template<typename T1> inline static typename T1::pod_type mat_norm_2(const Proxy<T1>& P);
  
  template<typename T1> inline static typename T1::pod_type mat_norm_inf(const Proxy<T1>& P);
  
  
  // norms for sparse matrices
  
  template<typename T1> inline static typename T1::pod_type mat_norm_1(const SpProxy<T1>& P);

  template<typename T1> inline static typename T1::pod_type mat_norm_2(const SpProxy<T1>& P, const typename arma_real_only<typename T1::elem_type>::result* junk = 0);
  template<typename T1> inline static typename T1::pod_type mat_norm_2(const SpProxy<T1>& P, const typename   arma_cx_only<typename T1::elem_type>::result* junk = 0);

  template<typename T1> inline static typename T1::pod_type mat_norm_inf(const SpProxy<T1>& P);
  };



//! @}
