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


//! \addtogroup op_relational
//! @{



class op_rel_lt_pre
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_lt_pre>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_lt_pre>& X);
  };



class op_rel_lt_post
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_lt_post>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_lt_post>& X);
  };



class op_rel_gt_pre
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_gt_pre>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_gt_pre>& X);
  };



class op_rel_gt_post
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_gt_post>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_gt_post>& X);
  };



class op_rel_lteq_pre
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_lteq_pre>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_lteq_pre>& X);
  };



class op_rel_lteq_post
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_lteq_post>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_lteq_post>& X);
  };



class op_rel_gteq_pre
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_gteq_pre>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_gteq_pre>& X);
  };



class op_rel_gteq_post
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_gteq_post>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_gteq_post>& X);
  };



class op_rel_eq
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_eq>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_eq>& X);
  };



class op_rel_noteq
  {
  public:
  
  template<typename T1>
  inline static void apply(Mat<uword>& out, const mtOp<uword, T1, op_rel_noteq>& X);
  
  template<typename T1>
  inline static void apply(Cube<uword>& out, const mtOpCube<uword, T1, op_rel_noteq>& X);
  };



//! @}
