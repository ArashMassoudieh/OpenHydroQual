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


// Copyright (C) 2008-2011 Conrad Sanderson
// Copyright (C) 2008-2011 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup arma_static_check
//! @{



template<bool ERROR___INCORRECT_OR_UNSUPPORTED_TYPE>
struct arma_type_check_cxx1998
  {
  arma_inline
  static
  void
  apply()
    {
    static const char
    junk[ ERROR___INCORRECT_OR_UNSUPPORTED_TYPE ? -1 : +1 ];
    }
  };



template<>
struct arma_type_check_cxx1998<false>
  {
  arma_inline
  static
  void
  apply()
    {
    }
  };



#if defined(ARMA_USE_CXX11)
  
  #define arma_static_check(condition, message)  static_assert( !(condition), #message )
  
  #define arma_type_check(condition)  static_assert( !(condition), "error: incorrect or unsupported type" )
  
#else

  #define arma_static_check(condition, message)  static const char message[ (condition) ? -1 : +1 ]
  
  #define arma_type_check(condition)  arma_type_check_cxx1998<condition>::apply()

#endif



//! @}
