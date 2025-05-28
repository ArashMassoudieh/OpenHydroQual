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


// Copyright (C) 2008-2013 Conrad Sanderson
// Copyright (C) 2008-2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup operator_ostream
//! @{



template<typename eT, typename T1>
inline
std::ostream&
operator<< (std::ostream& o, const Base<eT,T1>& X)
  {
  arma_extra_debug_sigprint();
  
  const unwrap<T1> tmp(X.get_ref());
  
  arma_ostream::print(o, tmp.M, true);
  
  return o;
  }



template<typename eT, typename T1>
inline
std::ostream&
operator<< (std::ostream& o, const SpBase<eT,T1>& X)
  {
  arma_extra_debug_sigprint();
  
  const unwrap_spmat<T1> tmp(X.get_ref());
  
  arma_ostream::print(o, tmp.M, true);
  
  return o;
  }



template<typename T1>
inline
std::ostream&
operator<< (std::ostream& o, const SpValProxy<T1>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  o << eT(X);
  
  return o;
  }



template<typename T1>
inline
std::ostream&
operator<< (std::ostream& o, const BaseCube<typename T1::elem_type,T1>& X)
  {
  arma_extra_debug_sigprint();
  
  const unwrap_cube<T1> tmp(X.get_ref());
  
  arma_ostream::print(o, tmp.M, true);
  
  return o;
  }



//! Print the contents of a field to the specified stream.
template<typename T1>
inline
std::ostream&
operator<< (std::ostream& o, const field<T1>& X)
  {
  arma_extra_debug_sigprint();
  
  arma_ostream::print(o, X);
  
  return o;
  }



//! Print the contents of a subfield to the specified stream
template<typename T1>
inline
std::ostream&
operator<< (std::ostream& o, const subview_field<T1>& X)
  {
  arma_extra_debug_sigprint();
  
  arma_ostream::print(o, X);

  return o;
  }



//! @}
