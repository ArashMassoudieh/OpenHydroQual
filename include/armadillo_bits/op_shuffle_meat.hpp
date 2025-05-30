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


// Copyright (C) 2009-2015 Conrad Sanderson
// Copyright (C) 2009-2015 NICTA (www.nicta.com.au)
// Copyright (C) 2009-2010 Dimitrios Bouzas
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



//! \addtogroup op_shuffle
//! @{



template<typename eT>
inline
void
op_shuffle::apply_direct(Mat<eT>& out, const Mat<eT>& X, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  if(X.is_empty()) { out.copy_size(X); return; }
  
  const uword N = (dim == 0) ? X.n_rows : X.n_cols;
  
  
  // see op_sort_index_bones.hpp for the definition of arma_sort_index_packet
  // and the associated comparison functor
  std::vector< arma_sort_index_packet<int,uword> > packet_vec(N);
  
  for(uword i=0; i<N; ++i)
    {
    packet_vec[i].val   = int(arma_rng::randi<int>());
    packet_vec[i].index = i;
    }
  
  arma_sort_index_helper_ascend comparator;
  
  std::sort( packet_vec.begin(), packet_vec.end(), comparator );
  
  const bool is_alias = (&out == &X);
  
  if(X.is_vec() == false)
    {
    if(is_alias == false)
      {
      arma_extra_debug_print("op_shuffle::apply(): matrix");
      
      out.copy_size(X);
      
      if(dim == 0)
        {
        for(uword i=0; i<N; ++i) { out.row(i) = X.row(packet_vec[i].index); }
        }
      else
        {
        for(uword i=0; i<N; ++i) { out.col(i) = X.col(packet_vec[i].index); }
        }
      }
    else  // in-place shuffle
      {
      arma_extra_debug_print("op_shuffle::apply(): in-place matrix");
      
      // reuse the val member variable of packet_vec
      // to indicate whether a particular row or column
      // has already been shuffled
      
      for(uword i=0; i<N; ++i)
        {
        packet_vec[i].val = 0;
        }
        
      if(dim == 0)
        {
        for(uword i=0; i<N; ++i)
          {
          if(packet_vec[i].val == 0)
            {
            const uword j = packet_vec[i].index;
            
            out.swap_rows(i, j);
            
            packet_vec[j].val = 1;
            }
          }
        }
      else
        {
        for(uword i=0; i<N; ++i)
          {
          if(packet_vec[i].val == 0)
            {
            const uword j = packet_vec[i].index;
            
            out.swap_cols(i, j);
            
            packet_vec[j].val = 1;
            }
          }
        }
      }
    }
  else  // we're dealing with a vector
    {
    if(is_alias == false)
      {
      arma_extra_debug_print("op_shuffle::apply(): vector");
      
      out.copy_size(X);
      
      if(dim == 0)
        {
        if(X.n_rows > 1)  // i.e. column vector
          {
          for(uword i=0; i<N; ++i) { out[i] = X[ packet_vec[i].index ]; }
          }
        else
          {
          out = X;
          }
        }
      else
        {
        if(X.n_cols > 1)  // i.e. row vector
          {
          for(uword i=0; i<N; ++i) { out[i] = X[ packet_vec[i].index ]; }
          }
        else
          {
          out = X;
          }
        }
      }
    else  // in-place shuffle
      {
      arma_extra_debug_print("op_shuffle::apply(): in-place vector");
      
      // reuse the val member variable of packet_vec
      // to indicate whether a particular row or column
      // has already been shuffled
      
      for(uword i=0; i<N; ++i)
        {
        packet_vec[i].val = 0;
        }
        
      if(dim == 0)
        {
        if(X.n_rows > 1)  // i.e. column vector
          {
          for(uword i=0; i<N; ++i)
            {
            if(packet_vec[i].val == 0)
              {
              const uword j = packet_vec[i].index;
              
              std::swap(out[i], out[j]);
              
              packet_vec[j].val = 1;
              }
            }
          }
        }
      else
        {
        if(X.n_cols > 1)  // i.e. row vector
          {
          for(uword i=0; i<N; ++i)
            {
            if(packet_vec[i].val == 0)
              {
              const uword j = packet_vec[i].index;
              
              std::swap(out[i], out[j]);
              
              packet_vec[j].val = 1;
              }
            }
          }
        }
      }
    }
  
  }



template<typename T1>
inline
void
op_shuffle::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_shuffle>& in)
  {
  arma_extra_debug_sigprint();
  
  const unwrap<T1> U(in.m);
  
  const uword dim = in.aux_uword_a;
  
  arma_debug_check( (dim > 1), "shuffle(): parameter 'dim' must be 0 or 1" );
  
  op_shuffle::apply_direct(out, U.M, dim);
  }



template<typename T1>
inline
void
op_shuffle_default::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_shuffle_default>& in)
  {
  arma_extra_debug_sigprint();
  
  const unwrap<T1> U(in.m);
  
  const uword dim = (T1::is_row) ? 1 : 0;
  
  op_shuffle::apply_direct(out, U.M, dim);
  }



//! @}
