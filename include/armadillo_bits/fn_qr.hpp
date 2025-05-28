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


// Copyright (C) 2009-2012 Conrad Sanderson
// Copyright (C) 2009-2012 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_qr
//! @{



//! QR decomposition
template<typename T1>
inline
bool
qr
  (
         Mat<typename T1::elem_type>&    Q,
         Mat<typename T1::elem_type>&    R,
  const Base<typename T1::elem_type,T1>& X,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  arma_debug_check( (&Q == &R), "qr(): Q and R are the same object");
  
  const bool status = auxlib::qr(Q, R, X);
  
  if(status == false)
    {
    Q.reset();
    R.reset();
    arma_bad("qr(): failed to converge", false);
    }
  
  return status;
  }



//! economical QR decomposition
template<typename T1>
inline
bool
qr_econ
  (
         Mat<typename T1::elem_type>&    Q,
         Mat<typename T1::elem_type>&    R,
  const Base<typename T1::elem_type,T1>& X,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  arma_debug_check( (&Q == &R), "qr_econ(): Q and R are the same object");
  
  const bool status = auxlib::qr_econ(Q, R, X);
  
  if(status == false)
    {
    Q.reset();
    R.reset();
    arma_bad("qr_econ(): failed to converge", false);
    }
  
  return status;
  }



//! @}
