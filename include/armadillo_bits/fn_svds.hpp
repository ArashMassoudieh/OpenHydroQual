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


// Copyright (C) 2015 Conrad Sanderson
// Copyright (C) 2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_svds
//! @{


template<typename T1>
inline
bool
svds_helper
  (
           Mat<typename T1::elem_type>&    U,
           Col<typename T1::pod_type >&    S,
           Mat<typename T1::elem_type>&    V,
  const SpBase<typename T1::elem_type,T1>& X,
  const uword                              k,
  const typename T1::pod_type              tol,
  const bool                               calc_UV,
  const typename arma_real_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::elem_type eT;
  typedef typename T1::pod_type   T;
  
  if(arma_config::arpack == false)
    {
    arma_stop("svds(): use of ARPACK needs to be enabled");
    return false;
    }
  
  arma_debug_check
    (
    ( ((void*)(&U) == (void*)(&S)) || (&U == &V) || ((void*)(&S) == (void*)(&V)) ),
    "svds(): two or more output objects are the same object"
    );
  
  arma_debug_check( (tol < T(0)), "svds(): tol must be >= 0" );
  
  const unwrap_spmat<T1> tmp(X.get_ref());
  const SpMat<eT>& A =   tmp.M;
  
  const uword kk = (std::min)( (std::min)(A.n_rows, A.n_cols), k );
  
  const T A_max = (A.n_nonzero > 0) ? T(max(abs(Col<eT>(const_cast<eT*>(A.values), A.n_nonzero, false)))) : T(0);
  
  if(A_max == T(0))
    {
    // TODO: use reset instead ?
    S.zeros(kk);
    
    if(calc_UV)
      {
      U.eye(A.n_rows, kk);
      V.eye(A.n_cols, kk);
      }
    }
  else
    {
    SpMat<eT> C( (A.n_rows + A.n_cols), (A.n_rows + A.n_cols) );
    
    SpMat<eT> B  = A / A_max;
    SpMat<eT> Bt = B.t();
    
    C(0, A.n_rows, size(B) ) = B;
    C(A.n_rows, 0, size(Bt)) = Bt;
    
    Bt.reset();
    B.reset();
    
    Col<eT> eigval;
    Mat<eT> eigvec;
    
    const bool status = sp_auxlib::eigs_sym(eigval, eigvec, C, kk, "la", (tol / Datum<T>::sqrt2));
    
    if(status == false)
      {
      U.reset();
      S.reset();
      V.reset();
      
      return false;
      }
    
    const T A_norm = max(eigval);
    
    const T tol2 = tol / Datum<T>::sqrt2 * A_norm;
    
    uvec indices = find(eigval > tol2);
    
    if(indices.n_elem > kk)
      {
      indices = indices.subvec(0,kk-1);
      }
    else
    if(indices.n_elem < kk)
      {
      const uvec indices2 = find(abs(eigval) <= tol2);
      
      const uword N_extra = (std::min)( indices2.n_elem, (kk - indices.n_elem) );
      
      if(N_extra > 0)  { indices = join_cols(indices, indices2.subvec(0,N_extra-1)); }
      }
    
    const uvec sorted_indices = sort_index(eigval, "descend");
    
    S = eigval.elem(sorted_indices);  S *= A_max;
    
    if(calc_UV)
      {
      uvec U_row_indices(A.n_rows);  for(uword i=0; i < A.n_rows; ++i)  { U_row_indices[i] = i;            }
      uvec V_row_indices(A.n_cols);  for(uword i=0; i < A.n_cols; ++i)  { V_row_indices[i] = i + A.n_rows; }
      
      U = Datum<T>::sqrt2 * eigvec(U_row_indices, sorted_indices);
      V = Datum<T>::sqrt2 * eigvec(V_row_indices, sorted_indices);
      }
    }
  
  arma_debug_warn( (S.n_elem < k), "svds(): warning: found fewer singular values than specified" );
  
  return true;
  }



template<typename T1>
inline
bool
svds_helper
  (
           Mat<typename T1::elem_type>&    U,
           Col<typename T1::pod_type >&    S,
           Mat<typename T1::elem_type>&    V,
  const SpBase<typename T1::elem_type,T1>& X,
  const uword                              k,
  const typename T1::pod_type              tol,
  const bool                               calc_UV,
  const typename arma_cx_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::elem_type eT;
  typedef typename T1::pod_type   T;
  
  if(arma_config::arpack == false)
    {
    arma_stop("svds(): use of ARPACK needs to be enabled");
    return false;
    }
  
  arma_debug_check
    (
    ( ((void*)(&U) == (void*)(&S)) || (&U == &V) || ((void*)(&S) == (void*)(&V)) ),
    "svds(): two or more output objects are the same object"
    );
  
  arma_debug_check( (tol < T(0)), "svds(): tol must be >= 0" );
  
  const unwrap_spmat<T1> tmp(X.get_ref());
  const SpMat<eT>& A =   tmp.M;
  
  const uword kk = (std::min)( (std::min)(A.n_rows, A.n_cols), k );
  
  const T A_max = (A.n_nonzero > 0) ? T(max(abs(Col<eT>(const_cast<eT*>(A.values), A.n_nonzero, false)))) : T(0);
  
  if(A_max == T(0))
    {
    // TODO: use reset instead ?
    S.zeros(kk);
    
    if(calc_UV)
      {
      U.eye(A.n_rows, kk);
      V.eye(A.n_cols, kk);
      }
    }
  else
    {
    SpMat<eT> C( (A.n_rows + A.n_cols), (A.n_rows + A.n_cols) );
    
    SpMat<eT> B  = A / A_max;
    SpMat<eT> Bt = B.t();
    
    C(0, A.n_rows, size(B) ) = B;
    C(A.n_rows, 0, size(Bt)) = Bt;
    
    Bt.reset();
    B.reset();
    
    Col<eT> eigval_tmp;
    Mat<eT> eigvec;
    
    const bool status = sp_auxlib::eigs_gen(eigval_tmp, eigvec, C, kk, "lr", (tol / Datum<T>::sqrt2));
    
    if(status == false)
      {
      U.reset();
      S.reset();
      V.reset();
      arma_bad("svds(): failed to converge", false);
      
      return false;
      }
    
    const Col<T> eigval = real(eigval_tmp);
    
    const T A_norm = max(eigval);
    
    const T tol2 = tol / Datum<T>::sqrt2 * A_norm;
    
    uvec indices = find(eigval > tol2);
    
    if(indices.n_elem > kk)
      {
      indices = indices.subvec(0,kk-1);
      }
    else
    if(indices.n_elem < kk)
      {
      const uvec indices2 = find(abs(eigval) <= tol2);
      
      const uword N_extra = (std::min)( indices2.n_elem, (kk - indices.n_elem) );
      
      if(N_extra > 0)  { indices = join_cols(indices, indices2.subvec(0,N_extra-1)); }
      }
    
    const uvec sorted_indices = sort_index(eigval, "descend");
    
    S = eigval.elem(sorted_indices);  S *= A_max;
    
    if(calc_UV)
      {
      uvec U_row_indices(A.n_rows);  for(uword i=0; i < A.n_rows; ++i)  { U_row_indices[i] = i;            }
      uvec V_row_indices(A.n_cols);  for(uword i=0; i < A.n_cols; ++i)  { V_row_indices[i] = i + A.n_rows; }
      
      U = Datum<T>::sqrt2 * eigvec(U_row_indices, sorted_indices);
      V = Datum<T>::sqrt2 * eigvec(V_row_indices, sorted_indices);
      }
    }
  
  arma_debug_warn( (S.n_elem < k), "svds(): warning: found fewer singular values than specified" );
  
  return true;
  }



//! find the k largest singular values and corresponding singular vectors of sparse matrix X
template<typename T1>
inline
bool
svds
  (
           Mat<typename T1::elem_type>&    U,
           Col<typename T1::pod_type >&    S,
           Mat<typename T1::elem_type>&    V,
  const SpBase<typename T1::elem_type,T1>& X,
  const uword                              k,
  const typename T1::pod_type              tol  = 0.0,
  const typename arma_real_or_cx_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  const bool status = svds_helper(U, S, V, X.get_ref(), k, tol, true);
  
  if(status == false)
    {
    arma_bad("svds(): failed to converge", false);
    }
  
  return status;
  }



//! find the k largest singular values of sparse matrix X
template<typename T1>
inline
bool
svds
  (
           Col<typename T1::pod_type >&    S,
  const SpBase<typename T1::elem_type,T1>& X,
  const uword                              k,
  const typename T1::pod_type              tol  = 0.0,
  const typename arma_real_or_cx_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  Mat<typename T1::elem_type> U;
  Mat<typename T1::elem_type> V;
  
  const bool status = svds_helper(U, S, V, X.get_ref(), k, tol, false);

  if(status == false)
    {
    arma_bad("svds(): failed to converge", false);
    }
  
  return status;
  }



//! find the k largest singular values of sparse matrix X
template<typename T1>
inline
Col<typename T1::pod_type>
svds
  (
  const SpBase<typename T1::elem_type,T1>& X,
  const uword                              k,
  const typename T1::pod_type              tol  = 0.0,
  const typename arma_real_or_cx_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  Col<typename T1::pod_type>  S;

  Mat<typename T1::elem_type> U;
  Mat<typename T1::elem_type> V;
  
  const bool status = svds_helper(U, S, V, X.get_ref(), k, tol, false);
  
  if(status == false)
    {
    arma_bad("svds(): failed to converge", true);
    }
  
  return S;
  }



//! @}
