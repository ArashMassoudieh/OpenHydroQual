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


// Copyright (C) 2013-2015 Conrad Sanderson
// Copyright (C) 2013-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_size
//! @{



inline
const SizeMat
size(const uword n_rows, const uword n_cols)
  {
  arma_extra_debug_sigprint();
  
  return SizeMat(n_rows, n_cols);
  }



template<typename T1>
inline
typename enable_if2< is_arma_type<T1>::value, const SizeMat >::result
size(const T1& X)
  {
  arma_extra_debug_sigprint();
  
  const Proxy<T1> P(X);
  
  return SizeMat( P.get_n_rows(), P.get_n_cols() );
  }



template<typename T1>
inline
typename enable_if2< is_arma_type<T1>::value, uword >::result
size(const T1& X, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  const Proxy<T1> P(X);
  
  return SizeMat( P.get_n_rows(), P.get_n_cols() )( dim );
  }



inline
const SizeCube
size(const uword n_rows, const uword n_cols, const uword n_slices)
  {
  arma_extra_debug_sigprint();
  
  return SizeCube(n_rows, n_cols, n_slices);
  }



template<typename T1>
inline
typename enable_if2< is_arma_cube_type<T1>::value, const SizeCube >::result
size(const T1& X)
  {
  arma_extra_debug_sigprint();
  
  const ProxyCube<T1> P(X);
  
  return SizeCube( P.get_n_rows(), P.get_n_cols(), P.get_n_slices() );
  }



template<typename T1>
inline
typename enable_if2< is_arma_cube_type<T1>::value, uword >::result
size(const T1& X, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  const ProxyCube<T1> P(X);
  
  return SizeCube( P.get_n_rows(), P.get_n_cols(), P.get_n_slices() )( dim );
  }



template<typename T1>
inline
typename enable_if2< is_arma_sparse_type<T1>::value, const SizeMat >::result
size(const T1& X)
  {
  arma_extra_debug_sigprint();
  
  const SpProxy<T1> P(X);
  
  return SizeMat( P.get_n_rows(), P.get_n_cols() );
  }



template<typename T1>
inline
typename enable_if2< is_arma_sparse_type<T1>::value, uword >::result
size(const T1& X, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  const SpProxy<T1> P(X);
  
  return SizeMat( P.get_n_rows(), P.get_n_cols() )( dim );
  }




template<typename oT>
inline
const SizeCube
size(const field<oT>& X)
  {
  arma_extra_debug_sigprint();
  
  return SizeCube( X.n_rows, X.n_cols, X.n_slices );
  }



template<typename oT>
inline
uword
size(const field<oT>& X, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  return SizeCube( X.n_rows, X.n_cols, X.n_slices )( dim );
  }



template<typename oT>
inline
const SizeCube
size(const subview_field<oT>& X)
  {
  arma_extra_debug_sigprint();
  
  return SizeCube( X.n_rows, X.n_cols, X.n_slices );
  }



template<typename oT>
inline
uword
size(const subview_field<oT>& X, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  return SizeCube( X.n_rows, X.n_cols, X.n_slices )( dim );
  }



//! @}
