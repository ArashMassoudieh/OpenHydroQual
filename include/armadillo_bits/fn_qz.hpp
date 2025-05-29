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
// Copyright (C) 2015 Keith O'Hara
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_qz
//! @{



//! QZ decomposition for pair of N-by-N general matrices (A,B)
template<typename T1, typename T2>
inline
typename
enable_if2
  <
  is_supported_blas_type<typename T1::elem_type>::value,
  bool
  >::result
qz
  (
         Mat<typename T1::elem_type>&    AA,
         Mat<typename T1::elem_type>&    BB,
         Mat<typename T1::elem_type>&    Q,
         Mat<typename T1::elem_type>&    Z,
  const Base<typename T1::elem_type,T1>& A,
  const Base<typename T1::elem_type,T2>& B
  )
  {
  arma_extra_debug_sigprint();
  
  const bool status = auxlib::qz(AA, BB, Q, Z, A, B, 'b');
  
  if(status == false)
    {
    AA.reset();
    BB.reset();
    Q.reset();
    Z.reset();
    arma_bad("qz(): failed to converge", false);
    }
  
  return status;
  }



//! @}
