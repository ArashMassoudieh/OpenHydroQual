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



//! \addtogroup distr_param
//! @{



class distr_param
  {
  public:
  
  uword state;
  
  union
    {
    int    a_int;
    double a_double;
    };
  
  union
    {
    int    b_int;
    double b_double;
    };
  
  
  inline distr_param()
    : state(0)
    {
    }
  
  
  inline explicit distr_param(const int a, const int b)
    : state(1)
    , a_int(a)
    , b_int(b)
    {
    }
  
  
  inline explicit distr_param(const double a, const double b)
    : state(2)
    , a_double(a)
    , b_double(b)
    {
    }
  };



//! @}
