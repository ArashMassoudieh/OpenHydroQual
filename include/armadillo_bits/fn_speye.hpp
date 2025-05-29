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


// Copyright (C) 2012-2015 Conrad Sanderson
// Copyright (C) 2012 Ryan Curtin
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_speye
//! @{



//! Generate a sparse matrix with the values along the main diagonal set to one
template<typename obj_type>
inline
obj_type
speye(const uword n_rows, const uword n_cols, const typename arma_SpMat_SpCol_SpRow_only<obj_type>::result* junk = NULL)
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  if(is_SpCol<obj_type>::value == true)
    {
    arma_debug_check( (n_cols != 1), "speye(): incompatible size" );
    }
  else
  if(is_SpRow<obj_type>::value == true)
    {
    arma_debug_check( (n_rows != 1), "speye(): incompatible size" );
    }
  
  obj_type out;
  
  out.eye(n_rows, n_cols);
  
  return out;
  }



template<typename obj_type>
inline
obj_type
speye(const SizeMat& s, const typename arma_SpMat_SpCol_SpRow_only<obj_type>::result* junk = NULL)
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  return speye<obj_type>(s.n_rows, s.n_cols);
  }



// Convenience shortcut method (no template parameter necessary)
inline
sp_mat
speye(const uword n_rows, const uword n_cols)
  {
  arma_extra_debug_sigprint();
  
  sp_mat out;
  
  out.eye(n_rows, n_cols);
  
  return out;
  }



inline
sp_mat
speye(const SizeMat& s)
  {
  arma_extra_debug_sigprint();
  
  sp_mat out;
  
  out.eye(s.n_rows, s.n_cols);
  
  return out;
  }



//! @}
