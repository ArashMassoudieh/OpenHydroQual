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


// Copyright (C) 2014 Conrad Sanderson
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup xtrans_mat
//! @{


template<typename eT, bool do_conj>
class xtrans_mat : public Base<eT, xtrans_mat<eT, do_conj> >
  {
  public:
  
  typedef eT                                       elem_type;
  typedef typename get_pod_type<elem_type>::result pod_type;
  
  static const bool is_row = false;
  static const bool is_col = false;
  
  arma_aligned const   Mat<eT>& X;
  arma_aligned mutable Mat<eT>  Y;
  
  arma_aligned const uword n_rows;
  arma_aligned const uword n_cols;
  arma_aligned const uword n_elem;
  
  inline explicit xtrans_mat(const Mat<eT>& in_X);
  
  inline void extract(Mat<eT>& out) const;
  
  inline eT operator[](const uword ii) const;
  inline eT at_alt    (const uword ii) const;
  
  arma_inline eT at(const uword in_row, const uword in_col) const;
  };



//! @}
