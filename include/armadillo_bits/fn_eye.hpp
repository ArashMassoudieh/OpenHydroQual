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


// Copyright (C) 2008-2015 Conrad Sanderson
// Copyright (C) 2008-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_eye
//! @{



arma_inline
const Gen<mat, gen_eye>
eye(const uword n_rows, const uword n_cols)
  {
  arma_extra_debug_sigprint();
  
  return Gen<mat, gen_eye>(n_rows, n_cols);
  }



arma_inline
const Gen<mat, gen_eye>
eye(const SizeMat& s)
  {
  arma_extra_debug_sigprint();
  
  return Gen<mat, gen_eye>(s.n_rows, s.n_cols);
  }



template<typename obj_type>
arma_inline
const Gen<obj_type, gen_eye>
eye(const uword n_rows, const uword n_cols, const typename arma_Mat_Col_Row_only<obj_type>::result* junk = 0)
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  if(is_Col<obj_type>::value)
    {
    arma_debug_check( (n_cols != 1), "eye(): incompatible size" );
    }
  else
  if(is_Row<obj_type>::value)
    {
    arma_debug_check( (n_rows != 1), "eye(): incompatible size" );
    }
  
  return Gen<obj_type, gen_eye>(n_rows, n_cols);
  }



template<typename obj_type>
arma_inline
const Gen<obj_type, gen_eye>
eye(const SizeMat& s, const typename arma_Mat_Col_Row_only<obj_type>::result* junk = 0)
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  return eye<obj_type>(s.n_rows, s.n_cols);
  }



template<typename obj_type>
inline
obj_type
eye(const uword n_rows, const uword n_cols, const typename arma_SpMat_SpCol_SpRow_only<obj_type>::result* junk = NULL)
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  if(is_SpCol<obj_type>::value == true)
    {
    arma_debug_check( (n_cols != 1), "eye(): incompatible size" );
    }
  else
  if(is_SpRow<obj_type>::value == true)
    {
    arma_debug_check( (n_rows != 1), "eye(): incompatible size" );
    }
  
  obj_type out;
  
  out.eye(n_rows, n_cols);
  
  return out;
  }



template<typename obj_type>
inline
obj_type
eye(const SizeMat& s, const typename arma_SpMat_SpCol_SpRow_only<obj_type>::result* junk = NULL)
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  return eye<obj_type>(s.n_rows, s.n_cols);
  }



//! @}
