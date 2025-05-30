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


//! \addtogroup op_sort
//! @{



class op_sort
  {
  public:
  
  template<typename eT>
  inline static void copy_row(eT* X, const Mat<eT>& A, const uword row);
  
  template<typename eT>
  inline static void copy_row(Mat<eT>& A, const eT* X, const uword row);
  
  template<typename eT>
  inline static void direct_sort(eT* X, const uword N, const uword sort_type = 0);
  
  template<typename eT>
  inline static void direct_sort_ascending(eT* X, const uword N);
  
  template<typename eT>
  inline static void apply_noalias(Mat<eT>& out, const Mat<eT>& X, const uword sort_type, const uword dim);
  
  template<typename T1>
  inline static void apply(Mat<typename T1::elem_type>& out, const Op<T1,op_sort>& in);
  };



class op_sort_default
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<typename T1::elem_type>& out, const Op<T1,op_sort_default>& in);
  };



template<typename eT>
struct arma_ascend_sort_helper
  {
  arma_inline bool operator() (const eT a, const eT b) const { return (a < b); }
  };



template<typename eT>
struct arma_descend_sort_helper
  {
  arma_inline bool operator() (const eT a, const eT b) const { return (a > b); }
  };
  


template<typename T>
struct arma_ascend_sort_helper< std::complex<T> >
  {
  typedef typename std::complex<T> eT;
  
  inline bool operator() (const eT& a, const eT& b) const { return (std::abs(a) < std::abs(b)); }
  };



template<typename T>
struct arma_descend_sort_helper< std::complex<T> >
  {
  typedef typename std::complex<T> eT;
  
  inline bool operator() (const eT& a, const eT& b) const { return (std::abs(a) > std::abs(b)); }
  };



//! @}
