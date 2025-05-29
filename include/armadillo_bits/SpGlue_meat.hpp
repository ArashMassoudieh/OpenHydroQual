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


// Copyright (C) 2012 Conrad Sanderson
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup SpGlue
//! @{



template<typename T1, typename T2, typename spglue_type>
inline
SpGlue<T1,T2,spglue_type>::SpGlue(const T1& in_A, const T2& in_B)
  : A(in_A)
  , B(in_B)
  {
  arma_extra_debug_sigprint();
  }



template<typename T1, typename T2, typename spglue_type>
inline
SpGlue<T1,T2,spglue_type>::SpGlue(const T1& in_A, const T2& in_B, const typename T1::elem_type in_aux)
  : A(in_A)
  , B(in_B)
  , aux(in_aux)
  {
  arma_extra_debug_sigprint();
  }



template<typename T1, typename T2, typename spglue_type>
inline
SpGlue<T1,T2,spglue_type>::~SpGlue()
  {
  arma_extra_debug_sigprint();
  }



//! @}
