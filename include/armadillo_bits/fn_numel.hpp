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


//! \addtogroup fn_numel
//! @{



template<typename T1>
inline
typename enable_if2< is_arma_type<T1>::value, uword >::result
numel(const T1& X)
  {
  arma_extra_debug_sigprint();
  
  const Proxy<T1> P(X);
  
  return P.get_n_elem();
  }



template<typename T1>
inline
typename enable_if2< is_arma_cube_type<T1>::value, uword >::result
numel(const T1& X)
  {
  arma_extra_debug_sigprint();
  
  const ProxyCube<T1> P(X);
  
  return P.get_n_elem();
  }



template<typename T1>
inline
typename enable_if2< is_arma_sparse_type<T1>::value, uword >::result
numel(const T1& X)
  {
  arma_extra_debug_sigprint();
  
  const SpProxy<T1> P(X);
  
  return P.get_n_elem();
  }



template<typename oT>
inline
uword
numel(const field<oT>& X)
  {
  arma_extra_debug_sigprint();
  
  return X.n_elem;
  }



template<typename oT>
inline
uword
numel(const subview_field<oT>& X)
  {
  arma_extra_debug_sigprint();
  
  return X.n_elem;
  }



//! @}
