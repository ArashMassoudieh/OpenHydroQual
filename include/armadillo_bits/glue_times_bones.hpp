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


// Copyright (C) 2008-2013 Conrad Sanderson
// Copyright (C) 2008-2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup glue_times
//! @{



//! \brief
//! Template metaprogram depth_lhs
//! calculates the number of Glue<Tx,Ty, glue_type> instances on the left hand side argument of Glue<Tx,Ty, glue_type>
//! i.e. it recursively expands each Tx, until the type of Tx is not "Glue<..,.., glue_type>"  (i.e the "glue_type" changes)

template<typename glue_type, typename T1>
struct depth_lhs
  {
  static const uword num = 0;
  };

template<typename glue_type, typename T1, typename T2>
struct depth_lhs< glue_type, Glue<T1,T2,glue_type> >
  {
  static const uword num = 1 + depth_lhs<glue_type, T1>::num;
  };



template<bool do_inv_detect>
struct glue_times_redirect2_helper
  {
  template<typename T1, typename T2>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue<T1,T2,glue_times>& X);
  };


template<>
struct glue_times_redirect2_helper<true>
  {
  template<typename T1, typename T2>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue<T1,T2,glue_times>& X);
  };



template<bool do_inv_detect>
struct glue_times_redirect3_helper
  {
  template<typename T1, typename T2, typename T3>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue< Glue<T1,T2,glue_times>,T3,glue_times>& X);
  };


template<>
struct glue_times_redirect3_helper<true>
  {
  template<typename T1, typename T2, typename T3>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue< Glue<T1,T2,glue_times>,T3,glue_times>& X);
  };



template<uword N>
struct glue_times_redirect
  {
  template<typename T1, typename T2>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue<T1,T2,glue_times>& X);
  };


template<>
struct glue_times_redirect<2>
  {
  template<typename T1, typename T2>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue<T1,T2,glue_times>& X);
  };


template<>
struct glue_times_redirect<3>
  {
  template<typename T1, typename T2, typename T3>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue< Glue<T1,T2,glue_times>,T3,glue_times>& X);
  };


template<>
struct glue_times_redirect<4>
  {
  template<typename T1, typename T2, typename T3, typename T4>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue< Glue< Glue<T1,T2,glue_times>, T3, glue_times>, T4, glue_times>& X);
  };



//! Class which implements the immediate multiplication of two or more matrices
class glue_times
  {
  public:
  
  
  template<typename T1, typename T2>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue<T1,T2,glue_times>& X);
  
  
  template<typename T1>
  arma_hot inline static void apply_inplace(Mat<typename T1::elem_type>& out, const T1& X);
  
  template<typename T1, typename T2>
  arma_hot inline static void apply_inplace_plus(Mat<typename T1::elem_type>& out, const Glue<T1, T2, glue_times>& X, const sword sign);
  
  //
  
  template<typename eT, const bool do_trans_A, const bool do_trans_B, typename TA, typename TB>
  arma_inline static uword mul_storage_cost(const TA& A, const TB& B);
  
  template<typename eT, const bool do_trans_A, const bool do_trans_B, const bool do_scalar_times, typename TA, typename TB>
  arma_hot inline static void apply(Mat<eT>& out, const TA& A, const TB& B, const eT val);
  
  template<typename eT, const bool do_trans_A, const bool do_trans_B, const bool do_trans_C, const bool do_scalar_times, typename TA, typename TB, typename TC>
  arma_hot inline static void apply(Mat<eT>& out, const TA& A, const TB& B, const TC& C, const eT val);
  
  template<typename eT, const bool do_trans_A, const bool do_trans_B, const bool do_trans_C, const bool do_trans_D, const bool do_scalar_times, typename TA, typename TB, typename TC, typename TD>
  arma_hot inline static void apply(Mat<eT>& out, const TA& A, const TB& B, const TC& C, const TD& D, const eT val);
  };



class glue_times_diag
  {
  public:
  
  template<typename T1, typename T2>
  arma_hot inline static void apply(Mat<typename T1::elem_type>& out, const Glue<T1, T2, glue_times_diag>& X);
  
  };



//! @}

