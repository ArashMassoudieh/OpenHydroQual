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


// Copyright (C) 2012 NICTA (www.nicta.com.au)
// Copyright (C) 2012 Conrad Sanderson
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup cond_rel
//! @{


//
// for preventing pedantic compiler warnings

template<const bool do_eval>
class cond_rel
  {
  public:
  
  template<typename eT> arma_inline static bool lt(const eT A, const eT B);
  template<typename eT> arma_inline static bool gt(const eT A, const eT B);

  template<typename eT> arma_inline static bool leq(const eT A, const eT B);
  template<typename eT> arma_inline static bool geq(const eT A, const eT B);
  
  template<typename eT> arma_inline static eT make_neg(const eT val);
  };



//! @}
