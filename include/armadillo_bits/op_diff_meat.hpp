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


//! \addtogroup op_diff
//! @{


template<typename eT>
inline
void
op_diff::apply_noalias(Mat<eT>& out, const Mat<eT>& X, const uword k, const uword dim)
  {
  arma_extra_debug_sigprint();
  
  uword n_rows = X.n_rows;
  uword n_cols = X.n_cols;
  
  if(dim == 0)
    {
    if(n_rows <= k)  { out.set_size(0,n_cols); return; }
    
    --n_rows;
    
    out.set_size(n_rows,n_cols);
    
    for(uword col=0; col < n_cols; ++col)
      {
            eT* out_colmem = out.colptr(col);
      const eT*   X_colmem =   X.colptr(col);
      
      for(uword row=0; row < n_rows; ++row)
        {
        const eT val0 = X_colmem[row  ];
        const eT val1 = X_colmem[row+1];
        
        out_colmem[row] = val1 - val0;
        }
      }
    
    if(k >= 2)
      {
      for(uword iter=2; iter <= k; ++iter)
        {
        --n_rows;
        
        for(uword col=0; col < n_cols; ++col)
          {
          eT* colmem = out.colptr(col);
          
          for(uword row=0; row < n_rows; ++row)
            {
            const eT val0 = colmem[row  ];
            const eT val1 = colmem[row+1];
            
            colmem[row] = val1 - val0;
            }
          }
        }
      
      out = out( span(0,n_rows-1), span::all );
      }
    }
  else
  if(dim == 1)
    {
    if(n_cols <= k)  { out.set_size(n_rows,0); return; }
    
    --n_cols;
    
    out.set_size(n_rows,n_cols);
    
    if(n_rows == 1)
      {
      const eT*   X_mem =   X.memptr();
            eT* out_mem = out.memptr();
            
      for(uword col=0; col < n_cols; ++col)
        {
        const eT val0 = X_mem[col  ];
        const eT val1 = X_mem[col+1];
        
        out_mem[col] = val1 - val0;
        }
      }
    else
      {
      for(uword col=0; col < n_cols; ++col)
        {
        eT* out_col_mem = out.colptr(col);
              
        const eT* X_col0_mem = X.colptr(col  );
        const eT* X_col1_mem = X.colptr(col+1);
        
        for(uword row=0; row < n_rows; ++row)
          {
          out_col_mem[row] = X_col1_mem[row] - X_col0_mem[row];
          }
        }
      }
    
    if(k >= 2)
      {
      for(uword iter=2; iter <= k; ++iter)
        {
        --n_cols;
        
        if(n_rows == 1)
          {
          eT* out_mem = out.memptr();
                
          for(uword col=0; col < n_cols; ++col)
            {
            const eT val0 = out_mem[col  ];
            const eT val1 = out_mem[col+1];
            
            out_mem[col] = val1 - val0;
            }
          }
        else
          {
          for(uword col=0; col < n_cols; ++col)
            {
                  eT* col0_mem = out.colptr(col  );
            const eT* col1_mem = out.colptr(col+1);
            
            for(uword row=0; row < n_rows; ++row)
              {
              col0_mem[row] = col1_mem[row] - col0_mem[row];
              }
            }
          }
        }
      
      out = out( span::all, span(0,n_cols-1) );
      }
    }
  }



template<typename T1>
inline
void
op_diff::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_diff>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const uword k   = in.aux_uword_a;
  const uword dim = in.aux_uword_b;
  
  arma_debug_check( (dim > 1), "diff(): parameter 'dim' must be 0 or 1" );
  
  if(k == 0)  { out = in.m; return; }
  
  const quasi_unwrap<T1> U(in.m);
  
  if(U.is_alias(out))
    {
    Mat<eT> tmp;
    
    op_diff::apply_noalias(tmp, U.M, k, dim);
    
    out.steal_mem(tmp);
    }
  else
    {
    op_diff::apply_noalias(out, U.M, k, dim);
    }
  }



template<typename T1>
inline
void
op_diff_default::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_diff_default>& in)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const uword k = in.aux_uword_a;
  
  if(k == 0)  { out = in.m; return; }
  
  const quasi_unwrap<T1> U(in.m);
  
  const uword dim = (T1::is_row) ? 1 : 0;
  
  if(U.is_alias(out))
    {
    Mat<eT> tmp;
    
    op_diff::apply_noalias(tmp, U.M, k, dim);
    
    out.steal_mem(tmp);
    }
  else
    {
    op_diff::apply_noalias(out, U.M, k, dim);
    }
  }



//! @}

