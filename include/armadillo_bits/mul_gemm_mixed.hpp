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


// Copyright (C) 2008-2011 Conrad Sanderson
// Copyright (C) 2008-2011 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup gemm_mixed
//! @{



//! \brief
//! Matrix multplication where the matrices have differing element types.
//! Uses caching for speedup.
//! Matrix 'C' is assumed to have been set to the correct size (i.e. taking into account transposes)

template<const bool do_trans_A=false, const bool do_trans_B=false, const bool use_alpha=false, const bool use_beta=false>
class gemm_mixed_large
  {
  public:
  
  template<typename out_eT, typename in_eT1, typename in_eT2>
  arma_hot
  inline
  static
  void
  apply
    (
          Mat<out_eT>& C,
    const Mat<in_eT1>& A,
    const Mat<in_eT2>& B,
    const out_eT       alpha = out_eT(1),
    const out_eT       beta  = out_eT(0)
    )
    {
    arma_extra_debug_sigprint();
    
    const uword A_n_rows = A.n_rows;
    const uword A_n_cols = A.n_cols;
    
    const uword B_n_rows = B.n_rows;
    const uword B_n_cols = B.n_cols;
    
    if( (do_trans_A == false) && (do_trans_B == false) )
      {
      podarray<in_eT1> tmp(A_n_cols);
      in_eT1* A_rowdata = tmp.memptr();
      
      for(uword row_A=0; row_A < A_n_rows; ++row_A)
        {
        tmp.copy_row(A, row_A);
        
        for(uword col_B=0; col_B < B_n_cols; ++col_B)
          {
          const in_eT2* B_coldata = B.colptr(col_B);
          
          out_eT acc = out_eT(0);
          for(uword i=0; i < B_n_rows; ++i)
            {
            acc += upgrade_val<in_eT1,in_eT2>::apply(A_rowdata[i]) * upgrade_val<in_eT1,in_eT2>::apply(B_coldata[i]);
            }
        
               if( (use_alpha == false) && (use_beta == false) )  { C.at(row_A,col_B) =       acc;                          }
          else if( (use_alpha == true ) && (use_beta == false) )  { C.at(row_A,col_B) = alpha*acc;                          }
          else if( (use_alpha == false) && (use_beta == true ) )  { C.at(row_A,col_B) =       acc + beta*C.at(row_A,col_B); }
          else if( (use_alpha == true ) && (use_beta == true ) )  { C.at(row_A,col_B) = alpha*acc + beta*C.at(row_A,col_B); }
          }
        }
      }
    else
    if( (do_trans_A == true) && (do_trans_B == false) )
      {
      for(uword col_A=0; col_A < A_n_cols; ++col_A)
        {
        // col_A is interpreted as row_A when storing the results in matrix C
        
        const in_eT1* A_coldata = A.colptr(col_A);
        
        for(uword col_B=0; col_B < B_n_cols; ++col_B)
          {
          const in_eT2* B_coldata = B.colptr(col_B);
          
          out_eT acc = out_eT(0);
          for(uword i=0; i < B_n_rows; ++i)
            {
            acc += upgrade_val<in_eT1,in_eT2>::apply(A_coldata[i]) * upgrade_val<in_eT1,in_eT2>::apply(B_coldata[i]);
            }
        
               if( (use_alpha == false) && (use_beta == false) )  { C.at(col_A,col_B) =       acc;                          }
          else if( (use_alpha == true ) && (use_beta == false) )  { C.at(col_A,col_B) = alpha*acc;                          }
          else if( (use_alpha == false) && (use_beta == true ) )  { C.at(col_A,col_B) =       acc + beta*C.at(col_A,col_B); }
          else if( (use_alpha == true ) && (use_beta == true ) )  { C.at(col_A,col_B) = alpha*acc + beta*C.at(col_A,col_B); }
          }
        }
      }
    else
    if( (do_trans_A == false) && (do_trans_B == true) )
      {
      Mat<in_eT2> B_tmp;
      
      op_strans::apply_mat_noalias(B_tmp, B);
      
      gemm_mixed_large<false, false, use_alpha, use_beta>::apply(C, A, B_tmp, alpha, beta);
      }
    else
    if( (do_trans_A == true) && (do_trans_B == true) )
      {
      // mat B_tmp = trans(B);
      // dgemm_arma<true, false,  use_alpha, use_beta>::apply(C, A, B_tmp, alpha, beta);
      
      
      // By using the trans(A)*trans(B) = trans(B*A) equivalency,
      // transpose operations are not needed
      
      podarray<in_eT2> tmp(B_n_cols);
      in_eT2* B_rowdata = tmp.memptr();
      
      for(uword row_B=0; row_B < B_n_rows; ++row_B)
        {
        tmp.copy_row(B, row_B);
        
        for(uword col_A=0; col_A < A_n_cols; ++col_A)
          {
          const in_eT1* A_coldata = A.colptr(col_A);
          
          out_eT acc = out_eT(0);
          for(uword i=0; i < A_n_rows; ++i)
            {
            acc += upgrade_val<in_eT1,in_eT2>::apply(B_rowdata[i]) * upgrade_val<in_eT1,in_eT2>::apply(A_coldata[i]);
            }
          
               if( (use_alpha == false) && (use_beta == false) )  { C.at(col_A,row_B) =       acc;                          }
          else if( (use_alpha == true ) && (use_beta == false) )  { C.at(col_A,row_B) = alpha*acc;                          }
          else if( (use_alpha == false) && (use_beta == true ) )  { C.at(col_A,row_B) =       acc + beta*C.at(col_A,row_B); }
          else if( (use_alpha == true ) && (use_beta == true ) )  { C.at(col_A,row_B) = alpha*acc + beta*C.at(col_A,row_B); }
          }
        }
      
      }
    }
    
  };



//! Matrix multplication where the matrices have different element types.
//! Simple version (no caching).
//! Matrix 'C' is assumed to have been set to the correct size (i.e. taking into account transposes)
template<const bool do_trans_A=false, const bool do_trans_B=false, const bool use_alpha=false, const bool use_beta=false>
class gemm_mixed_small
  {
  public:
  
  template<typename out_eT, typename in_eT1, typename in_eT2>
  arma_hot
  inline
  static
  void
  apply
    (
          Mat<out_eT>& C,
    const Mat<in_eT1>& A,
    const Mat<in_eT2>& B,
    const out_eT       alpha = out_eT(1),
    const out_eT       beta  = out_eT(0)
    )
    {
    arma_extra_debug_sigprint();
    
    const uword A_n_rows = A.n_rows;
    const uword A_n_cols = A.n_cols;
    
    const uword B_n_rows = B.n_rows;
    const uword B_n_cols = B.n_cols;
    
    if( (do_trans_A == false) && (do_trans_B == false) )
      {
      for(uword row_A = 0; row_A < A_n_rows; ++row_A)
        {
        for(uword col_B = 0; col_B < B_n_cols; ++col_B)
          {
          const in_eT2* B_coldata = B.colptr(col_B);
          
          out_eT acc = out_eT(0);
          for(uword i = 0; i < B_n_rows; ++i)
            {
            const out_eT val1 = upgrade_val<in_eT1,in_eT2>::apply(A.at(row_A,i));
            const out_eT val2 = upgrade_val<in_eT1,in_eT2>::apply(B_coldata[i]);
            acc += val1 * val2;
            //acc += upgrade_val<in_eT1,in_eT2>::apply(A.at(row_A,i)) * upgrade_val<in_eT1,in_eT2>::apply(B_coldata[i]);
            }
          
               if( (use_alpha == false) && (use_beta == false) )  { C.at(row_A,col_B) =       acc;                          }
          else if( (use_alpha == true ) && (use_beta == false) )  { C.at(row_A,col_B) = alpha*acc;                          }
          else if( (use_alpha == false) && (use_beta == true ) )  { C.at(row_A,col_B) =       acc + beta*C.at(row_A,col_B); }
          else if( (use_alpha == true ) && (use_beta == true ) )  { C.at(row_A,col_B) = alpha*acc + beta*C.at(row_A,col_B); }
          }
        }
      }
    else
    if( (do_trans_A == true) && (do_trans_B == false) )
      {
      for(uword col_A=0; col_A < A_n_cols; ++col_A)
        {
        // col_A is interpreted as row_A when storing the results in matrix C
        
        const in_eT1* A_coldata = A.colptr(col_A);
        
        for(uword col_B=0; col_B < B_n_cols; ++col_B)
          {
          const in_eT2* B_coldata = B.colptr(col_B);
          
          out_eT acc = out_eT(0);
          for(uword i=0; i < B_n_rows; ++i)
            {
            acc += upgrade_val<in_eT1,in_eT2>::apply(A_coldata[i]) * upgrade_val<in_eT1,in_eT2>::apply(B_coldata[i]);
            }
        
               if( (use_alpha == false) && (use_beta == false) )  { C.at(col_A,col_B) =       acc;                          }
          else if( (use_alpha == true ) && (use_beta == false) )  { C.at(col_A,col_B) = alpha*acc;                          }
          else if( (use_alpha == false) && (use_beta == true ) )  { C.at(col_A,col_B) =       acc + beta*C.at(col_A,col_B); }
          else if( (use_alpha == true ) && (use_beta == true ) )  { C.at(col_A,col_B) = alpha*acc + beta*C.at(col_A,col_B); }
          }
        }
      }
    else
    if( (do_trans_A == false) && (do_trans_B == true) )
      {
      for(uword row_A = 0; row_A < A_n_rows; ++row_A)
        {
        for(uword row_B = 0; row_B < B_n_rows; ++row_B)
          {
          out_eT acc = out_eT(0);
          for(uword i = 0; i < B_n_cols; ++i)
            {
            acc += upgrade_val<in_eT1,in_eT2>::apply(A.at(row_A,i)) * upgrade_val<in_eT1,in_eT2>::apply(B.at(row_B,i));
            }
          
               if( (use_alpha == false) && (use_beta == false) )  { C.at(row_A,row_B) =       acc;                          }
          else if( (use_alpha == true ) && (use_beta == false) )  { C.at(row_A,row_B) = alpha*acc;                          }
          else if( (use_alpha == false) && (use_beta == true ) )  { C.at(row_A,row_B) =       acc + beta*C.at(row_A,row_B); }
          else if( (use_alpha == true ) && (use_beta == true ) )  { C.at(row_A,row_B) = alpha*acc + beta*C.at(row_A,row_B); }
          }
        }
      }
    else
    if( (do_trans_A == true) && (do_trans_B == true) )
      {
      for(uword row_B=0; row_B < B_n_rows; ++row_B)
        {
        
        for(uword col_A=0; col_A < A_n_cols; ++col_A)
          {
          const in_eT1* A_coldata = A.colptr(col_A);
          
          out_eT acc = out_eT(0);
          for(uword i=0; i < A_n_rows; ++i)
            {
            acc += upgrade_val<in_eT1,in_eT2>::apply(B.at(row_B,i)) * upgrade_val<in_eT1,in_eT2>::apply(A_coldata[i]);
            }
        
               if( (use_alpha == false) && (use_beta == false) )  { C.at(col_A,row_B) =       acc;                          }
          else if( (use_alpha == true ) && (use_beta == false) )  { C.at(col_A,row_B) = alpha*acc;                          }
          else if( (use_alpha == false) && (use_beta == true ) )  { C.at(col_A,row_B) =       acc + beta*C.at(col_A,row_B); }
          else if( (use_alpha == true ) && (use_beta == true ) )  { C.at(col_A,row_B) = alpha*acc + beta*C.at(col_A,row_B); }
          }
        }
      
      }
    }
    
  };





//! \brief
//! Matrix multplication where the matrices have differing element types.

template<const bool do_trans_A=false, const bool do_trans_B=false, const bool use_alpha=false, const bool use_beta=false>
class gemm_mixed
  {
  public:
  
  //! immediate multiplication of matrices A and B, storing the result in C
  template<typename out_eT, typename in_eT1, typename in_eT2>
  inline
  static
  void
  apply
    (
          Mat<out_eT>& C,
    const Mat<in_eT1>& A,
    const Mat<in_eT2>& B,
    const out_eT       alpha = out_eT(1),
    const out_eT       beta  = out_eT(0)
    )
    {
    arma_extra_debug_sigprint();
    
    Mat<in_eT1> tmp_A;
    Mat<in_eT2> tmp_B;
    
    const bool predo_trans_A = ( (do_trans_A == true) && (is_cx<in_eT1>::yes) );
    const bool predo_trans_B = ( (do_trans_B == true) && (is_cx<in_eT2>::yes) );
    
    if(do_trans_A)
      {
      op_htrans::apply_mat_noalias(tmp_A, A);
      }
    
    if(do_trans_B)
      {
      op_htrans::apply_mat_noalias(tmp_B, B);
      }
     
    const Mat<in_eT1>& AA = (predo_trans_A == false) ? A : tmp_A;
    const Mat<in_eT2>& BB = (predo_trans_B == false) ? B : tmp_B;
    
    if( (AA.n_elem <= 64u) && (BB.n_elem <= 64u) )
      {
      gemm_mixed_small<((predo_trans_A) ? false : do_trans_A), ((predo_trans_B) ? false : do_trans_B), use_alpha, use_beta>::apply(C, AA, BB, alpha, beta);
      }
    else
      {
      gemm_mixed_large<((predo_trans_A) ? false : do_trans_A), ((predo_trans_B) ? false : do_trans_B), use_alpha, use_beta>::apply(C, AA, BB, alpha, beta);
      }
    }
  
  
  };



//! @}
