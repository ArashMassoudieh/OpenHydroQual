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


// Copyright (C) 2012 Conrad Sanderson
// Copyright (C) 2012 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup SpBase
//! @{



template<typename elem_type, typename derived>
arma_inline
const derived&
SpBase<elem_type,derived>::get_ref() const
  {
  return static_cast<const derived&>(*this);
  }



template<typename elem_type, typename derived>
inline
const SpOp<derived, spop_htrans>
SpBase<elem_type,derived>::t() const
  {
  return SpOp<derived,spop_htrans>( (*this).get_ref() );
  }


template<typename elem_type, typename derived>
inline
const SpOp<derived, spop_htrans>
SpBase<elem_type,derived>::ht() const
  {
  return SpOp<derived, spop_htrans>( (*this).get_ref() );
  }



template<typename elem_type, typename derived>
inline
const SpOp<derived, spop_strans>
SpBase<elem_type,derived>::st() const
  {
  return SpOp<derived, spop_strans>( (*this).get_ref() );
  }



template<typename elem_type, typename derived>
inline
void
SpBase<elem_type,derived>::print(const std::string extra_text) const
  {
  const unwrap_spmat<derived> tmp( (*this).get_ref() );
  
  tmp.M.impl_print(extra_text);
  }



template<typename elem_type, typename derived>
inline
void
SpBase<elem_type,derived>::print(std::ostream& user_stream, const std::string extra_text) const
  {
  const unwrap_spmat<derived> tmp( (*this).get_ref() );
  
  tmp.M.impl_print(user_stream, extra_text);
  }
  


template<typename elem_type, typename derived>
inline
void
SpBase<elem_type,derived>::raw_print(const std::string extra_text) const
  {
  const unwrap_spmat<derived> tmp( (*this).get_ref() );
  
  tmp.M.impl_raw_print(extra_text);
  }



template<typename elem_type, typename derived>
inline
void
SpBase<elem_type,derived>::raw_print(std::ostream& user_stream, const std::string extra_text) const
  {
  const unwrap_spmat<derived> tmp( (*this).get_ref() );
  
  tmp.M.impl_raw_print(user_stream, extra_text);
  }



template<typename elem_type, typename derived>
inline
void
SpBase<elem_type, derived>::print_dense(const std::string extra_text) const
  {
  const unwrap_spmat<derived> tmp( (*this).get_ref() );

  tmp.M.impl_print_dense(extra_text);
  }



template<typename elem_type, typename derived>
inline
void
SpBase<elem_type, derived>::print_dense(std::ostream& user_stream, const std::string extra_text) const
  {
  const unwrap_spmat<derived> tmp( (*this).get_ref() );

  tmp.M.impl_print_dense(user_stream, extra_text);
  }



template<typename elem_type, typename derived>
inline
void
SpBase<elem_type, derived>::raw_print_dense(const std::string extra_text) const
  {
  const unwrap_spmat<derived> tmp( (*this).get_ref() );

  tmp.M.impl_raw_print_dense(extra_text);
  }



template<typename elem_type, typename derived>
inline
void
SpBase<elem_type, derived>::raw_print_dense(std::ostream& user_stream, const std::string extra_text) const
  {
  const unwrap_spmat<derived> tmp( (*this).get_ref() );

  tmp.M.impl_raw_print_dense(user_stream, extra_text);
  }



//! @}
