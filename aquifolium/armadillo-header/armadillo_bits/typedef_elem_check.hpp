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


//! \addtogroup typedef_elem
//! @{


namespace junk
  {
  struct arma_elem_size_test
    {
    
    arma_static_check( (sizeof(u8) != 1), ERROR___TYPE_U8_HAS_UNSUPPORTED_SIZE );
    arma_static_check( (sizeof(s8) != 1), ERROR___TYPE_S8_HAS_UNSUPPORTED_SIZE );
    
    arma_static_check( (sizeof(u16) != 2), ERROR___TYPE_U16_HAS_UNSUPPORTED_SIZE );
    arma_static_check( (sizeof(s16) != 2), ERROR___TYPE_S16_HAS_UNSUPPORTED_SIZE );
    
    arma_static_check( (sizeof(u32) != 4), ERROR___TYPE_U32_HAS_UNSUPPORTED_SIZE );
    arma_static_check( (sizeof(s32) != 4), ERROR___TYPE_S32_HAS_UNSUPPORTED_SIZE );
    
    #if defined(ARMA_USE_U64S64)
      arma_static_check( (sizeof(u64) != 8), ERROR___TYPE_U64_HAS_UNSUPPORTED_SIZE );
      arma_static_check( (sizeof(s64) != 8), ERROR___TYPE_S64_HAS_UNSUPPORTED_SIZE );
    #endif
    
    arma_static_check( (sizeof(float)  != 4), ERROR___TYPE_FLOAT_HAS_UNSUPPORTED_SIZE );
    arma_static_check( (sizeof(double) != 8), ERROR___TYPE_DOUBLE_HAS_UNSUPPORTED_SIZE );
    
    arma_static_check( (sizeof(std::complex<float>)  != 8),  ERROR___TYPE_COMPLEX_FLOAT_HAS_UNSUPPORTED_SIZE );
    arma_static_check( (sizeof(std::complex<double>) != 16), ERROR___TYPE_COMPLEX_DOUBLE_HAS_UNSUPPORTED_SIZE );
    
    };
  }


//! @}
