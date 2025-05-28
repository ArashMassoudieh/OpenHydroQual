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
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup xvec_htrans
//! @{


template<typename eT>
class xvec_htrans : public Base<eT, xvec_htrans<eT> >
  {
  public:
  
  typedef eT                                       elem_type;
  typedef typename get_pod_type<elem_type>::result pod_type;
  
  static const bool is_row = false;
  static const bool is_col = false;
  
  arma_aligned const eT* const mem;
  
  const uword n_rows;
  const uword n_cols;
  const uword n_elem;
  
  
  inline explicit xvec_htrans(const eT* const in_mem, const uword in_n_rows, const uword in_n_cols);
  
  inline void extract(Mat<eT>& out) const;
  
  inline eT operator[](const uword ii) const;
  inline eT at_alt    (const uword ii) const;
  
  inline eT at        (const uword in_row, const uword in_col) const;
  };



//! @}
