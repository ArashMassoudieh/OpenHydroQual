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


// Copyright (C) 2013 Conrad Sanderson
// Copyright (C) 2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



//! \addtogroup op_vectorise
//! @{



class op_vectorise_col
  {
  public:
  
  template<typename T1> inline static void apply( Mat<typename T1::elem_type>& out, const Op<T1,op_vectorise_col>& in);
  
  template<typename T1> inline static void apply_proxy( Mat<typename T1::elem_type>& out, const Proxy<T1>& P);
  };



class op_vectorise_row
  {
  public:
  
  template<typename T1> inline static void apply( Mat<typename T1::elem_type>& out, const Op<T1,op_vectorise_row>& in);
  
  template<typename T1> inline static void apply_proxy( Mat<typename T1::elem_type>& out, const Proxy<T1>& P);
  };



class op_vectorise_all
  {
  public:
  
  template<typename T1> inline static void apply( Mat<typename T1::elem_type>& out, const Op<T1,op_vectorise_all>& in);
  };



//! @}
