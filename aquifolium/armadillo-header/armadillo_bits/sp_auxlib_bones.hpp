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


// Copyright (C) 2013 Ryan Curtin
// Copyright (C) 2013 Conrad Sanderson
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup sp_auxlib
//! @{


//! wrapper for accesing external functions for sparse matrices defined in ARPACK
class sp_auxlib
  {
  public:

  //
  // eigs_sym() via ARPACK
  
  template<typename eT, typename T1>
  inline static bool eigs_sym(Col<eT>& eigval, Mat<eT>& eigvec, const SpBase<eT, T1>& X, const uword n_eigvals);
  
  template<typename T, typename T1>
  inline static bool eigs_sym(Col<T>& eigval, Mat< std::complex<T> >& eigvec, const SpBase< std::complex<T>, T1 >& X, const uword n_eigvals);
  
  
  //
  // eigs_gen() via ARPACK
  
  template<typename T, typename T1>
  inline static bool eigs_gen(Col< std::complex<T> >& eigval, Mat< std::complex<T> >& eigvec, const SpBase<T, T1>& X, const uword n_eigvals);
  
  template<typename T, typename T1>
  inline static bool eigs_gen(Col< std::complex<T> >& eigval, Mat< std::complex<T> >& eigvec, const SpBase< std::complex<T>, T1>& X, const uword n_eigvals);
  
  
  private:
  
  // calls arpack saupd()/naupd() because the code is so similar for each
  // all of the extra variables are later used by seupd()/neupd(), but those
  // functions are very different and we can't combine their code
  
  template<typename eT, typename T, typename T1>
  inline static void run_aupd
    (
    const uword n_eigvals, const SpProxy<T1>& p, const bool sym,
    blas_int& n, eT& tol,
    podarray<T>& resid, blas_int& ncv, podarray<T>& v, blas_int& ldv,
    podarray<blas_int>& iparam, podarray<blas_int>& ipntr,
    podarray<T>& workd, podarray<T>& workl, blas_int& lworkl, podarray<eT>& rwork,
    blas_int& info
    );

  };
