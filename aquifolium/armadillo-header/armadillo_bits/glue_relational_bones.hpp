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


// Copyright (C) 2009-2010 Conrad Sanderson
// Copyright (C) 2009-2010 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup glue_relational
//! @{



class glue_rel_lt
  {
  public:
  
  template<typename T1, typename T2>
  inline static void apply(Mat <uword>& out, const mtGlue<uword, T1, T2, glue_rel_lt>& X);
  
  template<typename T1, typename T2>
  inline static void apply(Cube <uword>& out, const mtGlueCube<uword, T1, T2, glue_rel_lt>& X);
  };



class glue_rel_gt
  {
  public:
  
  template<typename T1, typename T2>
  inline static void apply(Mat <uword>& out, const mtGlue<uword, T1, T2, glue_rel_gt>& X);
  
  template<typename T1, typename T2>
  inline static void apply(Cube <uword>& out, const mtGlueCube<uword, T1, T2, glue_rel_gt>& X);
  };



class glue_rel_lteq
  {
  public:
  
  template<typename T1, typename T2>
  inline static void apply(Mat <uword>& out, const mtGlue<uword, T1, T2, glue_rel_lteq>& X);
  
  template<typename T1, typename T2>
  inline static void apply(Cube <uword>& out, const mtGlueCube<uword, T1, T2, glue_rel_lteq>& X);
  };



class glue_rel_gteq
  {
  public:
  
  template<typename T1, typename T2>
  inline static void apply(Mat <uword>& out, const mtGlue<uword, T1, T2, glue_rel_gteq>& X);
  
  template<typename T1, typename T2>
  inline static void apply(Cube <uword>& out, const mtGlueCube<uword, T1, T2, glue_rel_gteq>& X);
  };



class glue_rel_eq
  {
  public:
  
  template<typename T1, typename T2>
  inline static void apply(Mat <uword>& out, const mtGlue<uword, T1, T2, glue_rel_eq>& X);
  
  template<typename T1, typename T2>
  inline static void apply(Cube <uword>& out, const mtGlueCube<uword, T1, T2, glue_rel_eq>& X);
  };



class glue_rel_noteq
  {
  public:
  
  template<typename T1, typename T2>
  inline static void apply(Mat <uword>& out, const mtGlue<uword, T1, T2, glue_rel_noteq>& X);
  
  template<typename T1, typename T2>
  inline static void apply(Cube <uword>& out, const mtGlueCube<uword, T1, T2, glue_rel_noteq>& X);
  };



//! @}
