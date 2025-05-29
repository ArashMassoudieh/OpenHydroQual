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


//! \addtogroup SizeMat
//! @{



class SizeMat
  {
  public:
  
  const uword n_rows;
  const uword n_cols;
  
  inline SizeMat(const uword in_n_rows = 0, const uword in_n_cols = 0);
  
  // inline operator SizeCube () const;
  
  inline bool operator==(const SizeMat& s) const;
  inline bool operator!=(const SizeMat& s) const;
  
  inline bool operator==(const SizeCube& s) const;
  inline bool operator!=(const SizeCube& s) const;
  };



//! @}
