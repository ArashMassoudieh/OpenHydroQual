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


//! \addtogroup typedef_mat
//! @{


typedef Mat <unsigned char> uchar_mat;
typedef Col <unsigned char> uchar_vec;
typedef Col <unsigned char> uchar_colvec;
typedef Row <unsigned char> uchar_rowvec;
typedef Cube<unsigned char> uchar_cube;

typedef Mat <u32> u32_mat;
typedef Col <u32> u32_vec;
typedef Col <u32> u32_colvec;
typedef Row <u32> u32_rowvec;
typedef Cube<u32> u32_cube;

typedef Mat <s32> s32_mat;
typedef Col <s32> s32_vec;
typedef Col <s32> s32_colvec;
typedef Row <s32> s32_rowvec;
typedef Cube<s32> s32_cube;

#if defined(ARMA_USE_U64S64)
  typedef Mat <u64> u64_mat;
  typedef Col <u64> u64_vec;
  typedef Col <u64> u64_colvec;
  typedef Row <u64> u64_rowvec;
  typedef Cube<u64> u64_cube;

  typedef Mat <s64> s64_mat;
  typedef Col <s64> s64_vec;
  typedef Col <s64> s64_colvec;
  typedef Row <s64> s64_rowvec;
  typedef Cube<s64> s64_cube;
#endif

typedef Mat <uword> umat;
typedef Col <uword> uvec;
typedef Col <uword> ucolvec;
typedef Row <uword> urowvec;
typedef Cube<uword> ucube;

typedef Mat <sword> imat;
typedef Col <sword> ivec;
typedef Col <sword> icolvec;
typedef Row <sword> irowvec;
typedef Cube<sword> icube;

typedef Mat <float> fmat;
typedef Col <float> fvec;
typedef Col <float> fcolvec;
typedef Row <float> frowvec;
typedef Cube<float> fcube;

typedef Mat <double> mat;
typedef Col <double> vec;
typedef Col <double> colvec;
typedef Row <double> rowvec;
typedef Cube<double> cube;

typedef Mat <cx_float> cx_fmat;
typedef Col <cx_float> cx_fvec;
typedef Col <cx_float> cx_fcolvec;
typedef Row <cx_float> cx_frowvec;
typedef Cube<cx_float> cx_fcube;

typedef Mat <cx_double> cx_mat;
typedef Col <cx_double> cx_vec;
typedef Col <cx_double> cx_colvec;
typedef Row <cx_double> cx_rowvec;
typedef Cube<cx_double> cx_cube;



typedef SpMat <uword> sp_umat;
typedef SpCol <uword> sp_uvec;
typedef SpCol <uword> sp_ucolvec;
typedef SpRow <uword> sp_urowvec;

typedef SpMat <sword> sp_imat;
typedef SpCol <sword> sp_ivec;
typedef SpCol <sword> sp_icolvec;
typedef SpRow <sword> sp_irowvec;

typedef SpMat <float> sp_fmat;
typedef SpCol <float> sp_fvec;
typedef SpCol <float> sp_fcolvec;
typedef SpRow <float> sp_frowvec;

typedef SpMat <double> sp_mat;
typedef SpCol <double> sp_vec;
typedef SpCol <double> sp_colvec;
typedef SpRow <double> sp_rowvec;

typedef SpMat <cx_float> sp_cx_fmat;
typedef SpCol <cx_float> sp_cx_fvec;
typedef SpCol <cx_float> sp_cx_fcolvec;
typedef SpRow <cx_float> sp_cx_frowvec;

typedef SpMat <cx_double> sp_cx_mat;
typedef SpCol <cx_double> sp_cx_vec;
typedef SpCol <cx_double> sp_cx_colvec;
typedef SpRow <cx_double> sp_cx_rowvec;


//! @}
