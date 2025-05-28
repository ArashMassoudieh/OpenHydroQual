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


// Copyright (C) 2008-2012 Conrad Sanderson
// Copyright (C) 2008-2012 NICTA (www.nicta.com.au)
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
  
  template<typename T1>
  inline static void apply(Mat<typename T1::elem_type>& out, const Op<T1,op_sort>& in);
  };



//! @}
