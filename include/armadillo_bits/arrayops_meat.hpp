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


// Copyright (C) 2011-2015 Conrad Sanderson
// Copyright (C) 2011-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup arrayops
//! @{



template<typename eT>
arma_hot
arma_inline
void
arrayops::copy(eT* dest, const eT* src, const uword n_elem)
  {
  if( (n_elem <= 16) && (is_cx<eT>::no) )
    {
    arrayops::copy_small(dest, src, n_elem);
    }
  else
    {
    std::memcpy(dest, src, n_elem*sizeof(eT));
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::copy_small(eT* dest, const eT* src, const uword n_elem)
  {
  switch(n_elem)
    {
    case 16:  dest[15] = src[15];
    case 15:  dest[14] = src[14];
    case 14:  dest[13] = src[13];
    case 13:  dest[12] = src[12];
    case 12:  dest[11] = src[11];
    case 11:  dest[10] = src[10];
    case 10:  dest[ 9] = src[ 9];
    case  9:  dest[ 8] = src[ 8];
    case  8:  dest[ 7] = src[ 7];
    case  7:  dest[ 6] = src[ 6];
    case  6:  dest[ 5] = src[ 5];
    case  5:  dest[ 4] = src[ 4];
    case  4:  dest[ 3] = src[ 3];
    case  3:  dest[ 2] = src[ 2];
    case  2:  dest[ 1] = src[ 1];
    case  1:  dest[ 0] = src[ 0];
    default:  ;
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::copy_forwards(eT* dest, const eT* src, const uword n_elem)
  {
  // can't use std::memcpy(), as we don't know how it copies data
  uword j;
  
  for(j=1; j < n_elem; j+=2)
    {
    const eT tmp_i = (*src);  src++;
    const eT tmp_j = (*src);  src++;
    
    (*dest) = tmp_i;  dest++;
    (*dest) = tmp_j;  dest++;
    }
  
  if((j-1) < n_elem)
    {
    (*dest) = (*src);
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::copy_backwards(eT* dest, const eT* src, const uword n_elem)
  {
  // can't use std::memcpy(), as we don't know how it copies data
  
  // for(uword i=0; i < n_elem; ++i) 
  //   {
  //   const uword j = n_elem-i-1;
  //   
  //   dest[j] = src[j];
  //   }
  
  if(n_elem > 0)
    {
          eT* dest_it = &(dest[n_elem-1]);
    const eT*  src_it = &( src[n_elem-1]);
    
    uword j;
    for(j=1; j < n_elem; j+=2) 
      {
      const eT tmp_i = (*src_it);  src_it--;
      const eT tmp_j = (*src_it);  src_it--;
      
      (*dest_it) = tmp_i;  dest_it--;
      (*dest_it) = tmp_j;  dest_it--;
      }
    
    if((j-1) < n_elem)
      {
      (*dest_it) = (*src_it);
      }
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::fill_zeros(eT* dest, const uword n_elem)
  {
  arrayops::inplace_set(dest, eT(0), n_elem);
  }



template<typename out_eT, typename in_eT>
arma_hot
arma_inline
void
arrayops::convert_cx_scalar
  (
        out_eT& out,
  const in_eT&  in,
  const typename arma_not_cx<out_eT>::result* junk1,
  const typename arma_not_cx< in_eT>::result* junk2
  )
  {
  arma_ignore(junk1);
  arma_ignore(junk2);
  
  out = out_eT(in);
  }



template<typename out_eT, typename in_T>
arma_hot
arma_inline
void
arrayops::convert_cx_scalar
  (
        out_eT&             out,
  const std::complex<in_T>& in,
  const typename arma_not_cx<out_eT>::result* junk
  )
  {
  arma_ignore(junk);
  
  out = out_eT( in.real() );
  }



template<typename out_T, typename in_T>
arma_hot
arma_inline
void
arrayops::convert_cx_scalar
  (
        std::complex<out_T>& out,
  const std::complex< in_T>& in
  )
  {
  typedef std::complex<out_T> out_eT;
  
  out = out_eT(in);
  }



template<typename out_eT, typename in_eT>
arma_hot
inline
void
arrayops::convert(out_eT* dest, const in_eT* src, const uword n_elem)
  {
  if(is_same_type<out_eT,in_eT>::value)
    {
    const out_eT* src2 = (const out_eT*)src;
    
    if(dest != src2)  { arrayops::copy(dest, src2, n_elem); }
    
    return;
    }
  
  
  uword j;
  
  for(j=1; j<n_elem; j+=2)
    {
    const in_eT tmp_i = (*src);  src++;
    const in_eT tmp_j = (*src);  src++;
    
    // dest[i] = out_eT( tmp_i );
    // dest[j] = out_eT( tmp_j );
    
    (*dest) = (is_signed<out_eT>::value)
              ? out_eT( tmp_i )
              : ( cond_rel< is_signed<in_eT>::value >::lt(tmp_i, in_eT(0)) ? out_eT(0) : out_eT(tmp_i) );
    
    dest++;
    
    (*dest) = (is_signed<out_eT>::value)
              ? out_eT( tmp_j )
              : ( cond_rel< is_signed<in_eT>::value >::lt(tmp_j, in_eT(0)) ? out_eT(0) : out_eT(tmp_j) );
    dest++;
    }
  
  if((j-1) < n_elem)
    {
    const in_eT tmp_i = (*src);
    
    // dest[i] = out_eT( tmp_i );
    
    (*dest) = (is_signed<out_eT>::value)
              ? out_eT( tmp_i )
              : ( cond_rel< is_signed<in_eT>::value >::lt(tmp_i, in_eT(0)) ? out_eT(0) : out_eT(tmp_i) );
    }
  }



template<typename out_eT, typename in_eT>
arma_hot
inline
void
arrayops::convert_cx(out_eT* dest, const in_eT* src, const uword n_elem)
  {
  uword j;
  
  for(j=1; j<n_elem; j+=2)
    {
    arrayops::convert_cx_scalar( (*dest), (*src) );  dest++; src++;
    arrayops::convert_cx_scalar( (*dest), (*src) );  dest++; src++;
    }
  
  if((j-1) < n_elem)
    {
    arrayops::convert_cx_scalar( (*dest), (*src) );
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_plus(eT* dest, const eT* src, const uword n_elem)
  {
  if(memory::is_aligned(dest))
    {
    memory::mark_as_aligned(dest);
    
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      
      arrayops::inplace_plus_base(dest, src, n_elem);
      }
    else
      {
      arrayops::inplace_plus_base(dest, src, n_elem);
      }
    }
  else
    {
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      
      arrayops::inplace_plus_base(dest, src, n_elem);
      }
    else
      {
      arrayops::inplace_plus_base(dest, src, n_elem);
      }
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_minus(eT* dest, const eT* src, const uword n_elem)
  {
  if(memory::is_aligned(dest))
    {
    memory::mark_as_aligned(dest);
    
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      
      arrayops::inplace_minus_base(dest, src, n_elem);
      }
    else
      {
      arrayops::inplace_minus_base(dest, src, n_elem);
      }
    }
  else
    {
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      
      arrayops::inplace_minus_base(dest, src, n_elem);
      }
    else
      {
      arrayops::inplace_minus_base(dest, src, n_elem);
      }
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_mul(eT* dest, const eT* src, const uword n_elem)
  {
  if(memory::is_aligned(dest))
    {
    memory::mark_as_aligned(dest);
    
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      
      arrayops::inplace_mul_base(dest, src, n_elem);
      }
    else
      {
      arrayops::inplace_mul_base(dest, src, n_elem);
      }
    }
  else
    {
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      
      arrayops::inplace_mul_base(dest, src, n_elem);
      }
    else
      {
      arrayops::inplace_mul_base(dest, src, n_elem);
      }
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_div(eT* dest, const eT* src, const uword n_elem)
  {
  if(memory::is_aligned(dest))
    {
    memory::mark_as_aligned(dest);
    
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      
      arrayops::inplace_div_base(dest, src, n_elem);
      }
    else
      {
      arrayops::inplace_div_base(dest, src, n_elem);
      }
    }
  else
    {
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      
      arrayops::inplace_div_base(dest, src, n_elem);
      }
    else
      {
      arrayops::inplace_div_base(dest, src, n_elem);
      }
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_plus_base(eT* dest, const eT* src, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] += src[i];
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      const eT tmp_i = src[i];
      const eT tmp_j = src[j];
      
      dest[i] += tmp_i;
      dest[j] += tmp_j;
      }
    
    if(i < n_elem)
      {
      dest[i] += src[i];
      }
    }
  #endif
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_minus_base(eT* dest, const eT* src, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] -= src[i];
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      const eT tmp_i = src[i];
      const eT tmp_j = src[j];
      
      dest[i] -= tmp_i;
      dest[j] -= tmp_j;
      }
    
    if(i < n_elem)
      {
      dest[i] -= src[i];
      }
    }
  #endif
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_mul_base(eT* dest, const eT* src, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] *= src[i];
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      const eT tmp_i = src[i];
      const eT tmp_j = src[j];
      
      dest[i] *= tmp_i;
      dest[j] *= tmp_j;
      }
    
    if(i < n_elem)
      {
      dest[i] *= src[i];
      }
    }
  #endif
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_div_base(eT* dest, const eT* src, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] /= src[i];
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      const eT tmp_i = src[i];
      const eT tmp_j = src[j];
      
      dest[i] /= tmp_i;
      dest[j] /= tmp_j;
      }
    
    if(i < n_elem)
      {
      dest[i] /= src[i];
      }
    }
  #endif
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_set(eT* dest, const eT val, const uword n_elem)
  {
  typedef typename get_pod_type<eT>::result pod_type;
  
  if( (n_elem <= 16) && (is_cx<eT>::no) )
    {
    arrayops::inplace_set_small(dest, val, n_elem);
    }
  else
    {
    if( (val == eT(0)) && (std::numeric_limits<eT>::is_integer || (std::numeric_limits<pod_type>::is_iec559 && is_real<pod_type>::value)) )
      {
      std::memset(dest, 0, sizeof(eT)*n_elem);
      }
    else
      {
      if(memory::is_aligned(dest))
        {
        memory::mark_as_aligned(dest);
        
        arrayops::inplace_set_base(dest, val, n_elem);
        }
      else
        {
        arrayops::inplace_set_base(dest, val, n_elem);
        }
      }
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_set_base(eT* dest, const eT val, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] = val;
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      dest[i] = val;
      dest[j] = val;
      }
    
    if(i < n_elem)
      {
      dest[i] = val;
      }
    }
  #endif
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_set_small(eT* dest, const eT val, const uword n_elem)
  {
  switch(n_elem)
    {
    case 16: dest[15] = val;
    case 15: dest[14] = val;
    case 14: dest[13] = val;
    case 13: dest[12] = val;
    case 12: dest[11] = val;
    case 11: dest[10] = val;
    case 10: dest[ 9] = val;
    case  9: dest[ 8] = val;
    case  8: dest[ 7] = val;
    case  7: dest[ 6] = val;
    case  6: dest[ 5] = val;
    case  5: dest[ 4] = val;
    case  4: dest[ 3] = val;
    case  3: dest[ 2] = val;
    case  2: dest[ 1] = val;
    case  1: dest[ 0] = val;
    default:;
    }
  }



template<typename eT, const uword n_elem>
arma_hot
inline
void
arrayops::inplace_set_fixed(eT* dest, const eT val)
  {
  for(uword i=0; i<n_elem; ++i)
    {
    dest[i] = val;
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_plus(eT* dest, const eT val, const uword n_elem)
  {
  if(memory::is_aligned(dest))
    {
    memory::mark_as_aligned(dest);
    
    arrayops::inplace_plus_base(dest, val, n_elem);
    }
  else
    {
    arrayops::inplace_plus_base(dest, val, n_elem);
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_minus(eT* dest, const eT val, const uword n_elem)
  {
  if(memory::is_aligned(dest))
    {
    memory::mark_as_aligned(dest);
    
    arrayops::inplace_minus_base(dest, val, n_elem);
    }
  else
    {
    arrayops::inplace_minus_base(dest, val, n_elem);
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_mul(eT* dest, const eT val, const uword n_elem)
  {
  if(memory::is_aligned(dest))
    {
    memory::mark_as_aligned(dest);
    
    arrayops::inplace_mul_base(dest, val, n_elem);
    }
  else
    {
    arrayops::inplace_mul_base(dest, val, n_elem);
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_div(eT* dest, const eT val, const uword n_elem)
  {
  if(memory::is_aligned(dest))
    {
    memory::mark_as_aligned(dest);
    
    arrayops::inplace_div_base(dest, val, n_elem);
    }
  else
    {
    arrayops::inplace_div_base(dest, val, n_elem);
    }
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_plus_base(eT* dest, const eT val, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] += val;
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      dest[i] += val;
      dest[j] += val;
      }
    
    if(i < n_elem)
      {
      dest[i] += val;
      }
    }
  #endif
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_minus_base(eT* dest, const eT val, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] -= val;
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      dest[i] -= val;
      dest[j] -= val;
      }
    
    if(i < n_elem)
      {
      dest[i] -= val;
      }
    }
  #endif
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_mul_base(eT* dest, const eT val, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] *= val;
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      dest[i] *= val;
      dest[j] *= val;
      }
    
    if(i < n_elem)
      {
      dest[i] *= val;
      }
    }
  #endif
  }



template<typename eT>
arma_hot
inline
void
arrayops::inplace_div_base(eT* dest, const eT val, const uword n_elem)
  {
  #if defined(ARMA_SIMPLE_LOOPS)
    {
    for(uword i=0; i<n_elem; ++i)
      {
      dest[i] /= val;
      }
    }
  #else
    {
    uword i,j;
    
    for(i=0, j=1; j<n_elem; i+=2, j+=2)
      {
      dest[i] /= val;
      dest[j] /= val;
      }
    
    if(i < n_elem)
      {
      dest[i] /= val;
      }
    }
  #endif
  }



template<typename eT>
arma_hot
arma_pure
inline
eT
arrayops::accumulate(const eT* src, const uword n_elem)
  {
  #if defined(__FINITE_MATH_ONLY__) && (__FINITE_MATH_ONLY__ > 0)
    {
    eT acc = eT(0);
    
    if(memory::is_aligned(src))
      {
      memory::mark_as_aligned(src);
      for(uword i=0; i<n_elem; ++i)  { acc += src[i]; }
      }
    else
      {
      for(uword i=0; i<n_elem; ++i)  { acc += src[i]; }
      }
    
    return acc;
    }
  #else
    {
    eT acc1 = eT(0);
    eT acc2 = eT(0);
    
    uword j;
    
    for(j=1; j<n_elem; j+=2)
      {
      acc1 += (*src);  src++;
      acc2 += (*src);  src++;
      }
    
    if((j-1) < n_elem)
      {
      acc1 += (*src);
      }
    
    return acc1 + acc2;
    }
  #endif
  }



template<typename eT>
arma_hot
arma_pure
inline
eT
arrayops::product(const eT* src, const uword n_elem)
  {
  eT val1 = eT(1);
  eT val2 = eT(1);
  
  uword i,j;
  
  for(i=0, j=1; j<n_elem; i+=2, j+=2)
    {
    val1 *= src[i];
    val2 *= src[j];
    }
  
  if(i < n_elem)
    {
    val1 *= src[i];
    }
  
  return val1 * val2;
  }



template<typename eT>
arma_hot
arma_pure
inline
bool
arrayops::is_finite(const eT* src, const uword n_elem)
  {
  uword j;
  
  for(j=1; j<n_elem; j+=2)
    {
    const eT val_i = (*src);  src++;
    const eT val_j = (*src);  src++;
    
    if( (arma_isfinite(val_i) == false) || (arma_isfinite(val_j) == false) )
      {
      return false;
      }
    }
  
  if((j-1) < n_elem)
    {
    if(arma_isfinite(*src) == false)
      {
      return false;
      }
    }
  
  return true;
  }



template<typename eT>
arma_hot
arma_pure
inline
bool
arrayops::has_inf(const eT* src, const uword n_elem)
  {
  uword j;
  
  for(j=1; j<n_elem; j+=2)
    {
    const eT val_i = (*src);  src++;
    const eT val_j = (*src);  src++;
    
    if( arma_isinf(val_i) || arma_isinf(val_j) )  { return true; }
    }
  
  if((j-1) < n_elem)
    {
    if(arma_isinf(*src))  { return true; }
    }
  
  return false;
  }



template<typename eT>
arma_hot
arma_pure
inline
bool
arrayops::has_nan(const eT* src, const uword n_elem)
  {
  uword j;
  
  for(j=1; j<n_elem; j+=2)
    {
    const eT val_i = (*src);  src++;
    const eT val_j = (*src);  src++;
    
    if( arma_isnan(val_i) || arma_isnan(val_j) )  { return true; }
    }
  
  if((j-1) < n_elem)
    {
    if(arma_isnan(*src))  { return true; }
    }
  
  return false;
  }



//! @}
