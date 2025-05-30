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


//! \addtogroup fn_inplace_trans
//! @{



template<typename eT>
inline
typename
enable_if2
  <
  is_cx<eT>::no,
  void
  >::result
inplace_htrans
  (
        Mat<eT>& X,
  const char*    method = "std"
  )
  {
  arma_extra_debug_sigprint();
  
  inplace_strans(X, method);
  }



template<typename eT>
inline
typename
enable_if2
  <
  is_cx<eT>::yes,
  void
  >::result
inplace_htrans
  (
        Mat<eT>& X,
  const char*    method = "std"
  )
  {
  arma_extra_debug_sigprint();
  
  const char sig = method[0];
  
  arma_debug_check( ((sig != 's') && (sig != 'l')), "inplace_htrans(): unknown method specified" );
  
  const bool low_memory = (sig == 'l');
  
  if( (low_memory == false) || (X.n_rows == X.n_cols) )
    {
    op_htrans::apply_mat_inplace(X);
    }
  else
    {
    inplace_strans(X, method);
    
    X = conj(X);
    }
  }



template<typename eT>
inline
typename
enable_if2
  <
  is_cx<eT>::no,
  void
  >::result
inplace_trans
  (
        Mat<eT>& X,
  const char*    method = "std"
  )
  {
  arma_extra_debug_sigprint();
  
  const char sig = method[0];
  
  arma_debug_check( ((sig != 's') && (sig != 'l')), "inplace_trans(): unknown method specified" );
  
  inplace_strans(X, method);
  }



template<typename eT>
inline
typename
enable_if2
  <
  is_cx<eT>::yes,
  void
  >::result
inplace_trans
  (
        Mat<eT>& X,
  const char*    method = "std"
  )
  {
  arma_extra_debug_sigprint();
  
  const char sig = method[0];
  
  arma_debug_check( ((sig != 's') && (sig != 'l')), "inplace_trans(): unknown method specified" );
  
  inplace_htrans(X, method);
  }



//! @}
