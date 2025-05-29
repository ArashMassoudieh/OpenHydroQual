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


// Copyright (C) 2008-2014 Conrad Sanderson
// Copyright (C) 2008-2014 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup BaseCube
//! @{



template<typename elem_type, typename derived>
struct BaseCube_eval_Cube
  {
  arma_inline const derived& eval() const;
  };


template<typename elem_type, typename derived>
struct BaseCube_eval_expr
  {
  arma_inline Cube<elem_type> eval() const;   //!< force the immediate evaluation of a delayed expression
  };


template<typename elem_type, typename derived, bool condition>
struct BaseCube_eval {};

template<typename elem_type, typename derived>
struct BaseCube_eval<elem_type, derived, true>  { typedef BaseCube_eval_Cube<elem_type, derived>  result; };

template<typename elem_type, typename derived>
struct BaseCube_eval<elem_type, derived, false> { typedef BaseCube_eval_expr<elem_type, derived> result; };



//! Analog of the Base class, intended for cubes
template<typename elem_type, typename derived>
struct BaseCube
  : public BaseCube_eval<elem_type, derived, is_Cube<derived>::value>::result
  {
  arma_inline const derived& get_ref() const;
  
  inline void print(                           const std::string extra_text = "") const;
  inline void print(std::ostream& user_stream, const std::string extra_text = "") const;
  
  inline void raw_print(                           const std::string extra_text = "") const;
  inline void raw_print(std::ostream& user_stream, const std::string extra_text = "") const;
  };



//! @}
