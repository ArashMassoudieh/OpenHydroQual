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


// Copyright (C) 2013 Conrad Sanderson
// Copyright (C) 2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



//! \addtogroup op_any
//! @{



template<typename T1>
inline
bool
op_any::any_vec_helper(const Base<typename T1::elem_type, T1>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const Proxy<T1> P(X.get_ref());
  
  const uword n_elem = P.get_n_elem();
  
  if(Proxy<T1>::prefer_at_accessor == false)
    {
    typename Proxy<T1>::ea_type Pea = P.get_ea();
    
    for(uword i=0; i<n_elem; ++i)
      {
      if(Pea[i] != eT(0))  { return true; }
      }
    }
  else
    {
    const uword n_rows = P.get_n_rows();
    const uword n_cols = P.get_n_cols();
    
    for(uword col=0; col < n_cols; ++col)
    for(uword row=0; row < n_rows; ++row)
      {
      if(P.at(row,col) != eT(0))  { return true; }
      }
    }
  
  return false;
  }



template<typename T1, typename op_type>
inline
bool
op_any::any_vec_helper
  (
  const mtOp<uword, T1, op_type>& X,
  const typename arma_op_rel_only<op_type>::result           junk1,
  const typename arma_not_cx<typename T1::elem_type>::result junk2
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk1);
  arma_ignore(junk2);
  
  typedef typename T1::elem_type eT;
  
  const eT val = X.aux;
  
  const Proxy<T1> P(X.m);
  
  
  if(Proxy<T1>::prefer_at_accessor == false)
    {
    typename Proxy<T1>::ea_type Pea = P.get_ea();
    
    const uword n_elem = P.get_n_elem();
  
    for(uword i=0; i < n_elem; ++i)
      {
      const eT tmp = Pea[i];
      
           if(is_same_type<op_type, op_rel_lt_pre   >::yes)  { if(val <  tmp) { return true; } }
      else if(is_same_type<op_type, op_rel_lt_post  >::yes)  { if(tmp <  val) { return true; } }
      else if(is_same_type<op_type, op_rel_gt_pre   >::yes)  { if(val >  tmp) { return true; } }
      else if(is_same_type<op_type, op_rel_gt_post  >::yes)  { if(tmp >  val) { return true; } }
      else if(is_same_type<op_type, op_rel_lteq_pre >::yes)  { if(val <= tmp) { return true; } }
      else if(is_same_type<op_type, op_rel_lteq_post>::yes)  { if(tmp <= val) { return true; } }
      else if(is_same_type<op_type, op_rel_gteq_pre >::yes)  { if(val >= tmp) { return true; } }
      else if(is_same_type<op_type, op_rel_gteq_post>::yes)  { if(tmp >= val) { return true; } }
      else if(is_same_type<op_type, op_rel_eq       >::yes)  { if(tmp == val) { return true; } }
      else if(is_same_type<op_type, op_rel_noteq    >::yes)  { if(tmp != val) { return true; } }
      }
    }
  else
    {
    const uword n_rows = P.get_n_rows();
    const uword n_cols = P.get_n_cols();
    
    for(uword col=0; col < n_cols; ++col)
    for(uword row=0; row < n_rows; ++row)
      {
      const eT tmp = P.at(row,col);
      
           if(is_same_type<op_type, op_rel_lt_pre   >::yes)  { if(val <  tmp) { return true; } }
      else if(is_same_type<op_type, op_rel_lt_post  >::yes)  { if(tmp <  val) { return true; } }
      else if(is_same_type<op_type, op_rel_gt_pre   >::yes)  { if(val >  tmp) { return true; } }
      else if(is_same_type<op_type, op_rel_gt_post  >::yes)  { if(tmp >  val) { return true; } }
      else if(is_same_type<op_type, op_rel_lteq_pre >::yes)  { if(val <= tmp) { return true; } }
      else if(is_same_type<op_type, op_rel_lteq_post>::yes)  { if(tmp <= val) { return true; } }
      else if(is_same_type<op_type, op_rel_gteq_pre >::yes)  { if(val >= tmp) { return true; } }
      else if(is_same_type<op_type, op_rel_gteq_post>::yes)  { if(tmp >= val) { return true; } }
      else if(is_same_type<op_type, op_rel_eq       >::yes)  { if(tmp == val) { return true; } }
      else if(is_same_type<op_type, op_rel_noteq    >::yes)  { if(tmp != val) { return true; } }
      }
    }
  
  return false;
  }



template<typename T1, typename T2, typename glue_type>
inline
bool
op_any::any_vec_helper
  (
  const mtGlue<uword, T1, T2, glue_type>& X,
  const typename arma_glue_rel_only<glue_type>::result       junk1,
  const typename arma_not_cx<typename T1::elem_type>::result junk2,
  const typename arma_not_cx<typename T2::elem_type>::result junk3
  )
  {
  arma_extra_debug_sigprint();
  arma_ignore(junk1);
  arma_ignore(junk2);
  arma_ignore(junk3);
  
  typedef typename T1::elem_type eT1;
  typedef typename T2::elem_type eT2;
  
  typedef typename Proxy<T1>::ea_type ea_type1;
  typedef typename Proxy<T2>::ea_type ea_type2;
  
  const Proxy<T1> A(X.A);
  const Proxy<T2> B(X.B);
  
  arma_debug_assert_same_size(A, B, "relational operator");
  
  const bool prefer_at_accessor = Proxy<T1>::prefer_at_accessor || Proxy<T2>::prefer_at_accessor;
  
  if(prefer_at_accessor == false)
    {
    ea_type1 PA = A.get_ea();
    ea_type2 PB = B.get_ea();
    
    const uword n_elem = A.get_n_elem();
    
    for(uword i=0; i<n_elem; ++i)
      {
      const eT1 tmp1 = PA[i];
      const eT2 tmp2 = PB[i];
      
           if(is_same_type<glue_type, glue_rel_lt    >::yes)  { if(tmp1 <  tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_gt    >::yes)  { if(tmp1 >  tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_lteq  >::yes)  { if(tmp1 <= tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_gteq  >::yes)  { if(tmp1 >= tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_eq    >::yes)  { if(tmp1 == tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_noteq >::yes)  { if(tmp1 != tmp2) { return true; } }
      }
    }
  else
    {
    const uword n_rows = A.get_n_rows();
    const uword n_cols = A.get_n_cols();
    
    for(uword col=0; col < n_cols; ++col)
    for(uword row=0; row < n_rows; ++row)
      {
      const eT1 tmp1 = A.at(row,col);
      const eT2 tmp2 = B.at(row,col);
      
           if(is_same_type<glue_type, glue_rel_lt    >::yes)  { if(tmp1 <  tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_gt    >::yes)  { if(tmp1 >  tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_lteq  >::yes)  { if(tmp1 <= tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_gteq  >::yes)  { if(tmp1 >= tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_eq    >::yes)  { if(tmp1 == tmp2) { return true; } }
      else if(is_same_type<glue_type, glue_rel_noteq >::yes)  { if(tmp1 != tmp2) { return true; } }
      }
    }
  
  return false;
  }



template<typename T1>
inline
bool
op_any::any_vec(T1& X)
  {
  arma_extra_debug_sigprint();
  
  return op_any::any_vec_helper(X);
  }



template<typename T1>
inline
void
op_any::apply_helper(Mat<uword>& out, const Proxy<T1>& P, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  const uword n_rows = P.get_n_rows();
  const uword n_cols = P.get_n_cols();
  
  typedef typename Proxy<T1>::elem_type eT;
  
  if(dim == 0)  // traverse rows (ie. process each column)
    {
    out.zeros(1, n_cols);
    
    uword* out_mem = out.memptr();
    
    if(is_Mat<typename Proxy<T1>::stored_type>::value == true)
      {
      const unwrap<typename Proxy<T1>::stored_type> U(P.Q);
      
      for(uword col=0; col < n_cols; ++col)
        {
        const eT* colmem = U.M.colptr(col);
        
        for(uword row=0; row < n_rows; ++row)
          {
          if(colmem[row] != eT(0))  { out_mem[col] = uword(1); break; }
          }
        }
      }
    else
      {
      for(uword col=0; col < n_cols; ++col)
        {
        for(uword row=0; row < n_rows; ++row)
          {
          if(P.at(row,col) != eT(0))  { out_mem[col] = uword(1); break; }
          }
        }
      }
    }
  else
    {
    out.zeros(n_rows, 1);
    
    uword* out_mem = out.memptr();
    
    if(is_Mat<typename Proxy<T1>::stored_type>::value == true)
      {
      const unwrap<typename Proxy<T1>::stored_type> U(P.Q);
      
      for(uword col=0; col < n_cols; ++col)
        {
        const eT* colmem = U.M.colptr(col);
        
        for(uword row=0; row < n_rows; ++row)
          {
          if(colmem[row] != eT(0))  { out_mem[row] = uword(1); }
          }
        }
      }
    else
      {
      for(uword col=0; col < n_cols; ++col)
        {
        for(uword row=0; row < n_rows; ++row)
          {
          if(P.at(row,col) != eT(0))  { out_mem[row] = uword(1); }
          }
        }
      }
    }
  }



template<typename T1>
inline
void
op_any::apply(Mat<uword>& out, const mtOp<uword, T1, op_any>& X)
  {
  arma_extra_debug_sigprint();
  
  const uword dim = X.aux_uword_a;
  
  const Proxy<T1> P(X.m);
  
  if(P.is_alias(out) == false)
    {
    op_any::apply_helper(out, P, dim);
    }
  else
    {
    Mat<uword> out2;
    
    op_any::apply_helper(out2, P, dim);
    
    out.steal_mem(out2);
    }
  }



//! @}
