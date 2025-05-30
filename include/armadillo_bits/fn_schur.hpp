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


// Copyright (C) 2015 Conrad Sanderson
// Copyright (C) 2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_schur
//! @{


template<typename T1>
inline
bool
schur
  (
         Mat<typename T1::elem_type>&    S,
  const Base<typename T1::elem_type,T1>& X,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::elem_type eT;
  
  Mat<eT> U;
  
  const bool status = auxlib::schur(U, S, X.get_ref(), false);
  
  if(status == false)
    {
    S.reset();
    arma_bad("schur(): failed to converge", false);
    }
  
  return status;
  }



template<typename T1>
inline
Mat<typename T1::elem_type>
schur
  (
  const Base<typename T1::elem_type,T1>& X,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::elem_type eT;
  
  Mat<eT> S;
  Mat<eT> U;
  
  const bool status = auxlib::schur(U, S, X.get_ref(), false);
  
  if(status == false)
    {
    S.reset();
    arma_bad("schur(): failed to converge");
    }
  
  return S;
  }



template<typename T1> 
inline
bool
schur
  (
         Mat<typename T1::elem_type>&    U,
         Mat<typename T1::elem_type>&    S,
  const Base<typename T1::elem_type,T1>& X,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  arma_debug_check( void_ptr(&U) == void_ptr(&S), "schur(): 'U' is an alias of 'S'" );
  
  const bool status = auxlib::schur(U, S, X.get_ref(), true);
  
  if(status == false)
    {
    U.reset();
    S.reset();
    arma_bad("schur(): failed to converge", false);
    }
  
  return status;
  }



//! @}
