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


// Copyright (C) 2010-2012 Conrad Sanderson
// Copyright (C) 2010-2012 NICTA (www.nicta.com.au)
// Copyright (C) 2011 Ryan Curtin
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_trimat
//! @{



class op_trimat
  {
  public:
  
  template<typename eT>
  inline static void fill_zeros(Mat<eT>& A, const bool upper);
  
  //
  
  template<typename T1>
  inline static void apply(Mat<typename T1::elem_type>& out, const Op<T1,op_trimat>& in);
  
  template<typename T1>
  inline static void apply(Mat<typename T1::elem_type>& out, const Op<Op<T1,op_htrans>, op_trimat>& in);
  
  //
  
  template<typename eT>
  inline static void apply_htrans(Mat<eT>& out, const Mat<eT>& A, const bool upper, const typename arma_not_cx<eT>::result* junk = 0);
  
  template<typename eT>
  inline static void apply_htrans(Mat<eT>& out, const Mat<eT>& A, const bool upper, const typename arma_cx_only<eT>::result* junk = 0);
  };



//! @}
