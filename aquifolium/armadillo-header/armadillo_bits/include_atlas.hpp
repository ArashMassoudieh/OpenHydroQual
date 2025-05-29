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


#if defined(ARMA_USE_ATLAS)
  #if !defined(ARMA_ATLAS_INCLUDE_DIR)
    extern "C"
      {
      #include <cblas.h>
      #include <clapack.h>
      }
  #else
    #define ARMA_STR1(x) x
    #define ARMA_STR2(x) ARMA_STR1(x)
    
    #define ARMA_CBLAS   ARMA_STR2(ARMA_ATLAS_INCLUDE_DIR)ARMA_STR2(cblas.h)
    #define ARMA_CLAPACK ARMA_STR2(ARMA_ATLAS_INCLUDE_DIR)ARMA_STR2(clapack.h)
    
    extern "C"
      {
      #include ARMA_INCFILE_WRAP(ARMA_CBLAS)
      #include ARMA_INCFILE_WRAP(ARMA_CLAPACK)
      }
    
    #undef ARMA_STR1
    #undef ARMA_STR2
    #undef ARMA_CBLAS
    #undef ARMA_CLAPACK
  #endif
#endif
