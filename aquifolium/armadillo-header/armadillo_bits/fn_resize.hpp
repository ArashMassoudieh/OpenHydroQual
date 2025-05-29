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


// Copyright (C) 2011 Conrad Sanderson
// Copyright (C) 2011 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_resize
//! @{



template<typename T1>
inline
const Op<T1, op_resize>
resize(const Base<typename T1::elem_type,T1>& X, const uword in_n_rows, const uword in_n_cols)
  {
  arma_extra_debug_sigprint();
  
  return Op<T1, op_resize>(X.get_ref(), in_n_rows, in_n_cols);
  }



template<typename T1>
inline
const OpCube<T1, op_resize>
resize(const BaseCube<typename T1::elem_type,T1>& X, const uword in_n_rows, const uword in_n_cols, const uword in_n_slices)
  {
  arma_extra_debug_sigprint();
  
  return OpCube<T1, op_resize>(X.get_ref(), in_n_rows, in_n_cols, in_n_slices);
  }



//! @}
