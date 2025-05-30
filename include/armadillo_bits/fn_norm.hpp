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


// Copyright (C) 2008-2015 Conrad Sanderson
// Copyright (C) 2008-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup fn_norm
//! @{



template<typename T1>
inline
arma_warn_unused
typename enable_if2< is_arma_type<T1>::value, typename T1::pod_type >::result
norm
  (
  const T1&   X,
  const uword k = uword(2),
  const typename arma_real_or_cx_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::pod_type T;
  
  const Proxy<T1> P(X);
  
  if(P.get_n_elem() == 0)
    {
    return T(0);
    }
  
  const bool is_vec = (T1::is_row) || (T1::is_col) || (P.get_n_rows() == 1) || (P.get_n_cols() == 1);
  
  if(is_vec)
    {
    switch(k)
      {
      case 1:
        return op_norm::vec_norm_1(P);
        break;
      
      case 2:
        return op_norm::vec_norm_2(P);
        break;
      
      default:
        {
        arma_debug_check( (k == 0), "norm(): k must be greater than zero" );
        return op_norm::vec_norm_k(P, int(k));
        }
      }
    }
  else
    {
    switch(k)
      {
      case 1:
        return op_norm::mat_norm_1(P);
        break;
      
      case 2:
        return op_norm::mat_norm_2(P);
        break;
      
      default:
        arma_stop("norm(): unsupported matrix norm type");
        return T(0);
      }
    }
  
  return T(0);  // prevent erroneous compiler warnings
  }



template<typename T1>
inline
arma_warn_unused
typename enable_if2< is_arma_type<T1>::value, typename T1::pod_type >::result
norm
  (
  const T1&   X,
  const char* method,
  const typename arma_real_or_cx_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::pod_type T;
  
  const Proxy<T1> P(X);
  
  if(P.get_n_elem() == 0)
    {
    return T(0);
    }
  
  const char sig    = (method != NULL) ? method[0] : char(0);
  const bool is_vec = (T1::is_row) || (T1::is_col) || (P.get_n_rows() == 1) || (P.get_n_cols() == 1);
  
  if(is_vec)
    {
    if( (sig == 'i') || (sig == 'I') || (sig == '+') )   // max norm
      {
      return op_norm::vec_norm_max(P);
      }
    else
    if(sig == '-')   // min norm
      {
      return op_norm::vec_norm_min(P);
      }
    else
    if( (sig == 'f') || (sig == 'F') )
      {
      return op_norm::vec_norm_2(P);
      }
    else
      {
      arma_stop("norm(): unsupported vector norm type");
      return T(0);
      }
    }
  else
    {
    if( (sig == 'i') || (sig == 'I') || (sig == '+') )   // inf norm
      {
      return op_norm::mat_norm_inf(P);
      }
    else
    if( (sig == 'f') || (sig == 'F') )
      {
      return op_norm::vec_norm_2(P);
      }
    else
      {
      arma_stop("norm(): unsupported matrix norm type");
      return T(0);
      }
    }
  }



//
// norms for sparse matrices


template<typename T1>
inline
arma_warn_unused
typename enable_if2< is_arma_sparse_type<T1>::value, typename T1::pod_type >::result
norm
  (
  const T1&   X,
  const uword k = uword(2),
  const typename arma_real_or_cx_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::elem_type eT;
  typedef typename T1::pod_type   T;
  
  const SpProxy<T1> P(X);
  
  if(P.get_n_nonzero() == 0)
    {
    return T(0);
    }
  
  const bool is_vec = (P.get_n_rows() == 1) || (P.get_n_cols() == 1);
  
  if(is_vec == true)
    {
    const unwrap_spmat<typename SpProxy<T1>::stored_type> tmp(P.Q);
    const SpMat<eT>& A = tmp.M;
    
    // create a fake dense vector to allow reuse of code for dense vectors
    Col<eT> fake_vector( access::rwp(A.values), A.n_nonzero, false );
    
    const Proxy< Col<eT> > P_fake_vector(fake_vector);
    
    switch(k)
      {
      case 1:
        return op_norm::vec_norm_1(P_fake_vector);
        break;
      
      case 2:
        return op_norm::vec_norm_2(P_fake_vector);
        break;
      
      default:
        {
        arma_debug_check( (k == 0), "norm(): k must be greater than zero"   );
        return op_norm::vec_norm_k(P_fake_vector, int(k));
        }
      }
    }
  else
    {
    switch(k)
      {
      case 1:
        return op_norm::mat_norm_1(P);
        break;
      
      case 2:
        return op_norm::mat_norm_2(P);
        break;
      
      default:
        arma_stop("norm(): unsupported or unimplemented norm type for sparse matrices");
        return T(0);
      }
    }
  }



template<typename T1>
inline
arma_warn_unused
typename enable_if2< is_arma_sparse_type<T1>::value, typename T1::pod_type >::result
norm
  (
  const T1&   X,
  const char* method,
  const typename arma_real_or_cx_only<typename T1::elem_type>::result* junk = 0
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk);
  
  typedef typename T1::elem_type eT;
  typedef typename T1::pod_type   T;
  
  const SpProxy<T1> P(X);
  
  if(P.get_n_nonzero() == 0)
    {
    return T(0);
    }
  
  
  const unwrap_spmat<typename SpProxy<T1>::stored_type> tmp(P.Q);
  const SpMat<eT>& A = tmp.M;
  
  // create a fake dense vector to allow reuse of code for dense vectors
  Col<eT> fake_vector( access::rwp(A.values), A.n_nonzero, false );
  
  const Proxy< Col<eT> > P_fake_vector(fake_vector);
  
  
  const char sig    = (method != NULL) ? method[0] : char(0);
  const bool is_vec = (P.get_n_rows() == 1) || (P.get_n_cols() == 1);  // TODO: (T1::is_row) || (T1::is_col) || ...
  
  if(is_vec == true)
    {
    if( (sig == 'i') || (sig == 'I') || (sig == '+') )   // max norm
      {
      return op_norm::vec_norm_max(P_fake_vector);
      }
    else
    if(sig == '-')   // min norm
      {
      const T val = op_norm::vec_norm_min(P_fake_vector);
      
      if( P.get_n_nonzero() < P.get_n_elem() )
        {
        return (std::min)(T(0), val);
        }
      else
        {
        return val;
        }
      }
    else
    if( (sig == 'f') || (sig == 'F') )
      {
      return op_norm::vec_norm_2(P_fake_vector);
      }
    else
      {
      arma_stop("norm(): unsupported vector norm type");
      return T(0);
      }
    }
  else
    {
    if( (sig == 'i') || (sig == 'I') || (sig == '+') )   // inf norm
      {
      return op_norm::mat_norm_inf(P);
      }
    else
    if( (sig == 'f') || (sig == 'F') )
      {
      return op_norm::vec_norm_2(P_fake_vector);
      }
    else
      {
      arma_stop("norm(): unsupported matrix norm type");
      return T(0);
      }
    }
  }



//! @}
