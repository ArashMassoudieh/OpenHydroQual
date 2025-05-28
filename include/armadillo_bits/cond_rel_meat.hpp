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



template<>
template<typename eT>
arma_inline
bool
cond_rel<true>::lt(const eT A, const eT B)
  {
  return (A < B);
  }
  


template<>
template<typename eT>
arma_inline
bool
cond_rel<false>::lt(const eT, const eT)
  {
  return false;
  }
  


template<>
template<typename eT>
arma_inline
bool
cond_rel<true>::gt(const eT A, const eT B)
  {
  return (A > B);
  }
  


template<>
template<typename eT>
arma_inline
bool
cond_rel<false>::gt(const eT, const eT)
  {
  return false;
  }
  


template<>
template<typename eT>
arma_inline
bool
cond_rel<true>::leq(const eT A, const eT B)
  {
  return (A <= B);
  }
  


template<>
template<typename eT>
arma_inline
bool
cond_rel<false>::leq(const eT, const eT)
  {
  return false;
  }
  


template<>
template<typename eT>
arma_inline
bool
cond_rel<true>::geq(const eT A, const eT B)
  {
  return (A >= B);
  }
  


template<>
template<typename eT>
arma_inline
bool
cond_rel<false>::geq(const eT, const eT)
  {
  return false;
  }



template<>
template<typename eT>
arma_inline
eT
cond_rel<true>::make_neg(const eT val)
  {
  return -val;
  }
  


template<>
template<typename eT>
arma_inline
eT
cond_rel<false>::make_neg(const eT)
  {
  return eT(0);
  }
  


//! @}
