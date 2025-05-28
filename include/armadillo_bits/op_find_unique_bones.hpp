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


//! \addtogroup op_find_unique
//! @{



class op_find_unique
  {
  public:
  
  template<typename T1>
  static inline bool apply_helper(Mat<uword>& out, const Proxy<T1>& P, const bool ascending_indices);
  
  template<typename T1>
  static inline void apply(Mat<uword>& out, const mtOp<uword,T1,op_find_unique>& in);
  };



template<typename eT>
struct arma_find_unique_packet
  {
  eT    val;
  uword index;
  };



template<typename eT>
struct arma_find_unique_comparator
  {
  arma_inline
  bool
  operator() (const arma_find_unique_packet<eT>& A, const arma_find_unique_packet<eT>& B) const
    {
    return (A.val < B.val);
    }
  };



template<typename T>
struct arma_find_unique_comparator< std::complex<T> >
  {
  arma_inline
  bool
  operator() (const arma_find_unique_packet< std::complex<T> >& A, const arma_find_unique_packet< std::complex<T> >& B) const
    {
    const T A_real = A.val.real();
    const T B_real = B.val.real();
    
    return (  (A_real < B_real) ? true : ((A_real == B_real) ? (A.val.imag() < B.val.imag()) : false)  );
    }
  };



//! @}
