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


// Copyright (C) 2009-2010 Conrad Sanderson
// Copyright (C) 2009-2010 NICTA (www.nicta.com.au)
// Copyright (C) 2009-2010 Dimitrios Bouzas
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_cor
//! @{



template<typename T1>
inline
const Op<T1, op_cor>
cor(const Base<typename T1::elem_type,T1>& X, const uword norm_type = 0)
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( (norm_type > 1), "cor(): parameter 'norm_type' must be 0 or 1" );
  
  return Op<T1, op_cor>(X.get_ref(), norm_type, 0);
  }



template<typename T1, typename T2>
inline
const Glue<T1,T2,glue_cor>
cor(const Base<typename T1::elem_type,T1>& A, const Base<typename T1::elem_type,T2>& B, const uword norm_type = 0)
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( (norm_type > 1), "cor(): parameter 'norm_type' must be 0 or 1" );
  
  return Glue<T1, T2, glue_cor>(A.get_ref(), B.get_ref(), norm_type);
  }



//! @}
