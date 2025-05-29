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


// Copyright (C) 2012 Ryan Curtin
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup mtSpOp
//! @{

// Class for delayed multi-type sparse operations.  These are operations where
// the resulting type is different than the stored type.



template<typename out_eT, typename T1, typename op_type>
class mtSpOp : public SpBase<out_eT, mtSpOp<out_eT, T1, op_type> >
  {
  public:

  typedef          out_eT                       elem_type;
  typedef typename get_pod_type<out_eT>::result pod_type;

  typedef typename T1::elem_type                in_eT;

  static const bool is_row = false;
  static const bool is_col = false;

  inline explicit mtSpOp(const T1& in_m);
  inline          mtSpOp(const T1& in_m, const uword aux_uword_a, const uword aux_uword_b);

  inline          ~mtSpOp();

  arma_aligned const T1&    m;
  arma_aligned       uword  aux_uword_a;
  arma_aligned       uword  aux_uword_b;
  };



//! @}
