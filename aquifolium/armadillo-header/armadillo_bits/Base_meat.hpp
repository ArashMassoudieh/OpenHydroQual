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


// Copyright (C) 2008-2013 Conrad Sanderson
// Copyright (C) 2008-2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup Base
//! @{



template<typename elem_type, typename derived>
arma_inline
const derived&
Base<elem_type,derived>::get_ref() const
  {
  return static_cast<const derived&>(*this);
  }



template<typename elem_type, typename derived>
arma_inline
const Op<derived,op_htrans>
Base<elem_type,derived>::t() const
  {
  return Op<derived,op_htrans>( (*this).get_ref() );
  }



template<typename elem_type, typename derived>
arma_inline
const Op<derived,op_htrans>
Base<elem_type,derived>::ht() const
  {
  return Op<derived,op_htrans>( (*this).get_ref() );
  }



template<typename elem_type, typename derived>
arma_inline
const Op<derived,op_strans>
Base<elem_type,derived>::st() const
  {
  return Op<derived,op_strans>( (*this).get_ref() );
  }




template<typename elem_type, typename derived>
inline
void
Base<elem_type,derived>::print(const std::string extra_text) const
  {
  const Proxy<derived> P( (*this).get_ref() );
  
  const quasi_unwrap< typename Proxy<derived>::stored_type > tmp(P.Q);
  
  tmp.M.impl_print(extra_text);
  }



template<typename elem_type, typename derived>
inline
void
Base<elem_type,derived>::print(std::ostream& user_stream, const std::string extra_text) const
  {
  const Proxy<derived> P( (*this).get_ref() );
  
  const quasi_unwrap< typename Proxy<derived>::stored_type > tmp(P.Q);
  
  tmp.M.impl_print(user_stream, extra_text);
  }
  


template<typename elem_type, typename derived>
inline
void
Base<elem_type,derived>::raw_print(const std::string extra_text) const
  {
  const Proxy<derived> P( (*this).get_ref() );
  
  const quasi_unwrap< typename Proxy<derived>::stored_type > tmp(P.Q);
  
  tmp.M.impl_raw_print(extra_text);
  }



template<typename elem_type, typename derived>
inline
void
Base<elem_type,derived>::raw_print(std::ostream& user_stream, const std::string extra_text) const
  {
  const Proxy<derived> P( (*this).get_ref() );
  
  const quasi_unwrap< typename Proxy<derived>::stored_type > tmp(P.Q);
  
  tmp.M.impl_raw_print(user_stream, extra_text);
  }



//
// extra functions defined in Base_blas_elem_type

template<typename derived>
arma_inline
const Op<derived,op_inv>
Base_blas_elem_type<derived>::i(const bool slow) const
  {
  return Op<derived,op_inv>( static_cast<const derived&>(*this), ((slow == false) ? 0 : 1), 0 );
  }



template<typename derived>
arma_inline
const Op<derived,op_inv>
Base_blas_elem_type<derived>::i(const char* method) const
  {
  const char sig = method[0];
  
  arma_debug_check( ((sig != 's') && (sig != 'f')), "Base::i(): unknown method specified" );
  
  return Op<derived,op_inv>( static_cast<const derived&>(*this), ((sig == 'f') ? 0 : 1), 0 );
  }



//
// extra functions defined in Base_eval_Mat

template<typename elem_type, typename derived>
arma_inline
const derived&
Base_eval_Mat<elem_type, derived>::eval() const
  {
  arma_extra_debug_sigprint();
  
  return static_cast<const derived&>(*this);
  }



//
// extra functions defined in Base_eval_expr

template<typename elem_type, typename derived>
arma_inline
Mat<elem_type>
Base_eval_expr<elem_type, derived>::eval() const
  {
  arma_extra_debug_sigprint();
  
  return Mat<elem_type>( static_cast<const derived&>(*this) );
  }



//! @}
