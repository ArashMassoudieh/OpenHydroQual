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


// Copyright (C) 2008-2011 Conrad Sanderson
// Copyright (C) 2008-2011 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup mtGlueCube
//! @{



template<typename out_eT, typename T1, typename T2, typename glue_type>
class mtGlueCube : public BaseCube<out_eT, mtGlueCube<out_eT, T1, T2, glue_type> >
  {
  public:
  
  typedef          out_eT                       elem_type;
  typedef typename get_pod_type<out_eT>::result pod_type;
  
  arma_inline  mtGlueCube(const T1& in_A, const T2& in_B);
  arma_inline  mtGlueCube(const T1& in_A, const T2& in_B, const uword in_aux_uword);
  arma_inline ~mtGlueCube();
  
  arma_aligned const T1&   A;         //!< first operand
  arma_aligned const T2&   B;         //!< second operand
  arma_aligned       uword aux_uword; //!< storage of auxiliary data, uword format
  };



//! @}
