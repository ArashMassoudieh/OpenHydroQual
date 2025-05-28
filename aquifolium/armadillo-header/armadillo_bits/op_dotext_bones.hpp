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


// Copyright (C) 2008-2010 Conrad Sanderson
// Copyright (C) 2008-2010 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_dotext
//! @{



class op_dotext
  {
  public:
  
  
  template<typename eT>
  inline static eT direct_rowvec_mat_colvec       (const eT* A_mem, const Mat<eT>& B, const eT* C_mem);
  
  template<typename eT>
  inline static eT direct_rowvec_transmat_colvec  (const eT* A_mem, const Mat<eT>& B, const eT* C_mem);
  
  template<typename eT>
  inline static eT direct_rowvec_diagmat_colvec   (const eT* A_mem, const Mat<eT>& B, const eT* C_mem);
  
  template<typename eT>
  inline static eT direct_rowvec_invdiagmat_colvec(const eT* A_mem, const Mat<eT>& B, const eT* C_mem);
  
  template<typename eT>
  inline static eT direct_rowvec_invdiagvec_colvec(const eT* A_mem, const Mat<eT>& B, const eT* C_mem);
  
  };



//! @}

