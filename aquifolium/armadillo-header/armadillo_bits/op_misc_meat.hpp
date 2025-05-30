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


// Copyright (C) 2008-2012 Conrad Sanderson
// Copyright (C) 2008-2012 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup op_misc
//! @{



template<typename T1>
inline
void
op_real::apply( Mat<typename T1::pod_type>& out, const mtOp<typename T1::pod_type, T1, op_real>& X )
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::pod_type T;
  
  const Proxy<T1> P(X.m);
  
  const uword n_rows = P.get_n_rows();
  const uword n_cols = P.get_n_cols();
    
  out.set_size(n_rows, n_cols);
  
  T* out_mem = out.memptr();
  
  if(Proxy<T1>::prefer_at_accessor == false)
    {
    typedef typename Proxy<T1>::ea_type ea_type;
    
    const uword   n_elem  = P.get_n_elem();
          ea_type A       = P.get_ea();
    
    for(uword i=0; i < n_elem; ++i)
      {
      out_mem[i] = std::real( A[i] );
      }
    }
  else
    {
    for(uword col=0; col < n_cols; ++col)
    for(uword row=0; row < n_rows; ++row)
      {
      *out_mem = std::real( P.at(row,col) );
      out_mem++;
      }
    }
  }



template<typename T1>
inline
void
op_real::apply( Cube<typename T1::pod_type>& out, const mtOpCube<typename T1::pod_type, T1, op_real>& X )
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::pod_type T;
  
  const ProxyCube<T1> P(X.m);
  
  const uword n_rows   = P.get_n_rows();
  const uword n_cols   = P.get_n_cols();
  const uword n_slices = P.get_n_slices();
    
  out.set_size(n_rows, n_cols, n_slices);
  
  T* out_mem = out.memptr();

  if(ProxyCube<T1>::prefer_at_accessor == false)
    {
    typedef typename ProxyCube<T1>::ea_type ea_type;
    
    const uword   n_elem  = P.get_n_elem();
          ea_type A       = P.get_ea();
    
    for(uword i=0; i < n_elem; ++i)
      {
      out_mem[i] = std::real( A[i] );
      }
    }
  else
    {
    for(uword slice=0; slice < n_slices; ++slice)
    for(uword col=0;   col   < n_cols;   ++col  )
    for(uword row=0;   row   < n_rows;   ++row  )
      {
      *out_mem = std::real( P.at(row,col,slice) );
      out_mem++;
      }
    }
  }



template<typename T1>
inline
void
op_imag::apply( Mat<typename T1::pod_type>& out, const mtOp<typename T1::pod_type, T1, op_imag>& X )
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::pod_type T;
  
  const Proxy<T1> P(X.m);
  
  const uword n_rows = P.get_n_rows();
  const uword n_cols = P.get_n_cols();
    
  out.set_size(n_rows, n_cols);
  
  T* out_mem = out.memptr();
  
  if(Proxy<T1>::prefer_at_accessor == false)
    {
    typedef typename Proxy<T1>::ea_type ea_type;
    
    const uword   n_elem  = P.get_n_elem();
          ea_type A       = P.get_ea();
    
    for(uword i=0; i < n_elem; ++i)
      {
      out_mem[i] = std::imag( A[i] );
      }
    }
  else
    {
    for(uword col=0; col < n_cols; ++col)
    for(uword row=0; row < n_rows; ++row)
      {
      *out_mem = std::imag( P.at(row,col) );
      out_mem++;
      }
    }
  }



template<typename T1>
inline
void
op_imag::apply( Cube<typename T1::pod_type>& out, const mtOpCube<typename T1::pod_type, T1, op_imag>& X )
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::pod_type T;
  
  const ProxyCube<T1> P(X.m);
  
  const uword n_rows   = P.get_n_rows();
  const uword n_cols   = P.get_n_cols();
  const uword n_slices = P.get_n_slices();
    
  out.set_size(n_rows, n_cols, n_slices);
  
  T* out_mem = out.memptr();

  if(ProxyCube<T1>::prefer_at_accessor == false)
    {
    typedef typename ProxyCube<T1>::ea_type ea_type;
    
    const uword   n_elem  = P.get_n_elem();
          ea_type A       = P.get_ea();
    
    for(uword i=0; i < n_elem; ++i)
      {
      out_mem[i] = std::imag( A[i] );
      }
    }
  else
    {
    for(uword slice=0; slice < n_slices; ++slice)
    for(uword col=0;   col   < n_cols;   ++col  )
    for(uword row=0;   row   < n_rows;   ++row  )
      {
      *out_mem = std::imag( P.at(row,col,slice) );
      out_mem++;
      }
    }
  }



template<typename T1>
inline
void
op_abs::apply( Mat<typename T1::pod_type>& out, const mtOp<typename T1::pod_type, T1, op_abs>& X )
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::pod_type T;
  
  const Proxy<T1> P(X.m);
  
  const uword n_rows = P.get_n_rows();
  const uword n_cols = P.get_n_cols();
    
  out.set_size(n_rows, n_cols);
  
  T* out_mem = out.memptr();
  
  if(Proxy<T1>::prefer_at_accessor == false)
    {
    typedef typename Proxy<T1>::ea_type ea_type;
    
    const uword   n_elem  = P.get_n_elem();
          ea_type A       = P.get_ea();
    
    for(uword i=0; i < n_elem; ++i)
      {
      out_mem[i] = std::abs( A[i] );
      }
    }
  else
    {
    for(uword col=0; col < n_cols; ++col)
    for(uword row=0; row < n_rows; ++row)
      {
      *out_mem = std::abs( P.at(row,col) );
      out_mem++;
      }
    }
  }



template<typename T1>
inline
void
op_abs::apply( Cube<typename T1::pod_type>& out, const mtOpCube<typename T1::pod_type, T1, op_abs>& X )
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::pod_type T;
  
  const ProxyCube<T1> P(X.m);
  
  const uword n_rows   = P.get_n_rows();
  const uword n_cols   = P.get_n_cols();
  const uword n_slices = P.get_n_slices();
    
  out.set_size(n_rows, n_cols, n_slices);
  
  T* out_mem = out.memptr();

  if(ProxyCube<T1>::prefer_at_accessor == false)
    {
    typedef typename ProxyCube<T1>::ea_type ea_type;
    
    const uword   n_elem  = P.get_n_elem();
          ea_type A       = P.get_ea();
    
    for(uword i=0; i < n_elem; ++i)
      {
      out_mem[i] = std::abs( A[i] );
      }
    }
  else
    {
    for(uword slice=0; slice < n_slices; ++slice)
    for(uword col=0;   col   < n_cols;   ++col  )
    for(uword row=0;   row   < n_rows;   ++row  )
      {
      *out_mem = std::abs( P.at(row,col,slice) );
      out_mem++;
      }
    }
  }



//! @}
