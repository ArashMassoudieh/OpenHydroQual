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


// Copyright (C) 2012-2015 Conrad Sanderson
// Copyright (C) 2012-2015 NICTA (www.nicta.com.au)
// Copyright (C) 2012 Arnold Wiliem
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



//! \addtogroup op_unique
//! @{



class op_unique
  {
  public:
  
  template<typename T1>
  inline static bool apply_helper(Mat<typename T1::elem_type>& out, const Proxy<T1>& P);
  
  template<typename T1>
  inline static void apply(Mat<typename T1::elem_type>& out, const Op<T1,op_unique>& in);
  };



template<typename eT>
struct arma_unique_comparator
  {
  arma_inline
  bool
  operator() (const eT a, const eT b) const
    {
    return ( a < b );
    }
  };



template<typename T>
struct arma_unique_comparator< std::complex<T> >
  {
  arma_inline
  bool
  operator() (const std::complex<T>& a, const std::complex<T>& b) const
    {
    const T a_real = a.real();
    const T b_real = b.real();
    
    return (  (a_real < b_real) ? true : ((a_real == b_real) ? (a.imag() < b.imag()) : false)  );
    }
  };



//! @}
