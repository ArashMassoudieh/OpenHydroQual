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


// Copyright (C) 2009-2013 Conrad Sanderson
// Copyright (C) 2009-2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_solve
//! @{



//! Solve a system of linear equations, i.e., A*X = B, where X is unknown.
//! For a square matrix A, this function is conceptually the same as X = inv(A)*B,
//! but is done more efficiently.
//! The number of rows in A and B must be the same.
//! B can be either a column vector or a matrix.
//! This function will also try to provide approximate solutions
//! to under-determined as well as over-determined systems (non-square A matrices).

template<typename T1, typename T2>
inline
const Glue<T1, T2, glue_solve>
solve
  (
  const Base<typename T1::elem_type,T1>& A,
  const Base<typename T1::elem_type,T2>& B,
  const bool slow = false,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  return Glue<T1, T2, glue_solve>(A.get_ref(), B.get_ref(), ((slow == false) ? 0 : 1) );
  }



template<typename T1, typename T2>
inline
const Glue<T1, T2, glue_solve>
solve
  (
  const Base<typename T1::elem_type,T1>& A,
  const Base<typename T1::elem_type,T2>& B,
  const char* method,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  const char sig = (method != NULL) ? method[0] : char(0);
  
  arma_debug_check( ((sig != 's') && (sig != 'f')), "solve(): unknown method specified" );
  
  return Glue<T1, T2, glue_solve>( A.get_ref(), B.get_ref(), ((sig == 'f') ? 0 : 1) );
  }



template<typename T1, typename T2>
inline
const Glue<T1, T2, glue_solve_tr>
solve
  (
  const Op<T1, op_trimat>& A,
  const Base<typename T1::elem_type,T2>& B,
  const bool slow = false,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(slow);
  arma_ignore(junk);
  
  return Glue<T1, T2, glue_solve_tr>(A.m, B.get_ref(), A.aux_uword_a);
  }



template<typename T1, typename T2>
inline
const Glue<T1, T2, glue_solve_tr>
solve
  (
  const Op<T1, op_trimat>& A,
  const Base<typename T1::elem_type,T2>& B,
  const char* method,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  const char sig = (method != NULL) ? method[0] : char(0);
  
  arma_debug_check( ((sig != 's') && (sig != 'f')), "solve(): unknown method specified" );
  
  return Glue<T1, T2, glue_solve_tr>(A.m, B.get_ref(), A.aux_uword_a);
  }



template<typename T1, typename T2>
inline
bool
solve
  (
         Mat<typename T1::elem_type>&    out,
  const Base<typename T1::elem_type,T1>& A,
  const Base<typename T1::elem_type,T2>& B,
  const bool slow = false,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  try
    {
    out = solve( A.get_ref(), B.get_ref(), slow );
    }
  catch(std::runtime_error&)
    {
    return false;
    }
  
  return true;
  }



template<typename T1, typename T2>
inline
bool
solve
  (
         Mat<typename T1::elem_type>&    out,
  const Base<typename T1::elem_type,T1>& A,
  const Base<typename T1::elem_type,T2>& B,
  const char* method,
  const typename arma_blas_type_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  try
    {
    out = solve( A.get_ref(), B.get_ref(), method );
    }
  catch(std::runtime_error&)
    {
    return false;
    }
  
  return true;
  }



//! @}
