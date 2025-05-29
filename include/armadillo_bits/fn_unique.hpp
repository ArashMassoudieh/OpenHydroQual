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


// Copyright (C) 2012-2015 Conrad Sanderson
// Copyright (C) 2012-2015 NICTA (www.nicta.com.au)
// Copyright (C) 2012 Arnold Wiliem
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



template<typename T1>
inline
typename
enable_if2
  <
  is_arma_type<T1>::value,
  const Op<T1,op_unique>
  >::result
unique(const T1& A)
  {
  arma_extra_debug_sigprint();
  
  return Op<T1,op_unique>(A);
  }
