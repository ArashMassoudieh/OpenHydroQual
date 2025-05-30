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


// This Source Code Form is a compilation of:
// (1) source code written by Ryan Curtin and Conrad Sanderson, and
// (2) extracts from SuperLU 4.3 source code.

// This compilation is Copyright (C) 2015 Ryan Curtin and Conrad Sanderson
// and is subject to the terms of the Mozilla Public License, v. 2.0.
// 
// The source code that is distinct and separate from SuperLU 4.3 source code
// is Copyright (C) 2015 Ryan Curtin and Conrad Sanderson and is subject to the
// terms of the Mozilla Public License, v. 2.0.
// 
// If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// The original SuperLU 4.3 source code is licensed under a 3-clause BSD license,
// as follows:
// 
// Copyright (c) 2003, The Regents of the University of California, through
// Lawrence Berkeley National Laboratory (subject to receipt of any required 
// approvals from U.S. Dept. of Energy) 
// 
// All rights reserved. 
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 
// 
// (1) Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer. 
// (2) Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution. 
// (3) Neither the name of Lawrence Berkeley National Laboratory, U.S. Dept. of
// Energy nor the names of its contributors may be used to endorse or promote
// products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 


#if defined(ARMA_USE_SUPERLU)


#if defined(ARMA_USE_SUPERLU_HEADERS) || defined(ARMA_SUPERLU_INCLUDE_DIR)

// Since we need to suport float, double, cx_float and cx_double,
// as well as preserve the sanity of the user,
// we cannot simply include all the SuperLU headers due to their messy state
// (duplicate definitions, pollution of global namespace, bizarro defines).
// As such we are forced to include only a subset of the headers
// and manually specify a few SuperLU structures and function prototypes.
//
// CAVEAT:
// This code requires SuperLU version 4.3,
// and assumes that newer 4.x versions will have no API changes.

namespace arma
{

namespace superlu
  {
  // slu_*defs.h has int typedef'fed to int_t.  I'll just write it as int for
  // simplicity, where I can, but supermatrix.h needs int_t.
  typedef int int_t;
  
  // Include supermatrix.h.  This gives us SuperMatrix.
  // Put it in the slu namespace.
  // For versions of SuperLU I am familiar with, supermatrix.h does not include any other files.
  // Therefore, putting it in the superlu namespace is reasonably safe.
  // This same reasoning is true for superlu_enum_consts.h.
  
  #if defined(ARMA_SUPERLU_INCLUDE_DIR)
    #define ARMA_SLU_STR(x) x
    #define ARMA_SLU_STR2(x) ARMA_SLU_STR(x)
    
    #define ARMA_SLU_SUPERMATRIX_H         ARMA_SLU_STR2(ARMA_SUPERLU_INCLUDE_DIR)ARMA_SLU_STR2(supermatrix.h)
    #define ARMA_SLU_SUPERLU_ENUM_CONSTS_H ARMA_SLU_STR2(ARMA_SUPERLU_INCLUDE_DIR)ARMA_SLU_STR2(superlu_enum_consts.h)
  #else
    #define ARMA_SLU_SUPERMATRIX_H         supermatrix.h
    #define ARMA_SLU_SUPERLU_ENUM_CONSTS_H superlu_enum_consts.h
  #endif
  
  #include ARMA_INCFILE_WRAP(ARMA_SLU_SUPERMATRIX_H)
  #include ARMA_INCFILE_WRAP(ARMA_SLU_SUPERLU_ENUM_CONSTS_H)
  
  #undef ARMA_SLU_SUPERMATRIX_H
  #undef ARMA_SLU_SUPERLU_ENUM_CONSTS_H
  
  
  typedef struct
    {
    int*    panel_histo;
    double* utime;
    float*  ops;
    int     TinyPivots;
    int     RefineSteps;
    int     expansions;
    } SuperLUStat_t;
  
  
  typedef struct
    {
    fact_t        Fact;
    yes_no_t      Equil;
    colperm_t     ColPerm;
    trans_t       Trans;
    IterRefine_t  IterRefine;
    double        DiagPivotThresh;
    yes_no_t      SymmetricMode;
    yes_no_t      PivotGrowth;
    yes_no_t      ConditionNumber;
    rowperm_t     RowPerm;
    int           ILU_DropRule;
    double        ILU_DropTol;
    double        ILU_FillFactor;
    norm_t        ILU_Norm;
    double        ILU_FillTol;
    milu_t        ILU_MILU;
    double        ILU_MILU_Dim;
    yes_no_t      ParSymbFact;
    yes_no_t      ReplaceTinyPivot;
    yes_no_t      SolveInitialized;
    yes_no_t      RefineInitialized;
    yes_no_t      PrintStat;
    int           nnzL, nnzU;
    int           num_lookaheads;
    yes_no_t      lookahead_etree;
    yes_no_t      SymPattern;
    } superlu_options_t;
  }
}


#else

// Not using any SuperLU headers, so define all required enums and structs.
// 
// CAVEAT:
// This code requires SuperLU version 4.3,
// and assumes that newer 4.x versions will have no API changes.

namespace arma
{

namespace superlu
  {
  typedef int int_t;
  
  typedef enum
    {
    SLU_NC,
    SLU_NCP,
    SLU_NR,
    SLU_SC,
    SLU_SCP,
    SLU_SR,
    SLU_DN,
    SLU_NR_loc
    } Stype_t;
  
  
  typedef enum
    {
    SLU_S,
    SLU_D,
    SLU_C,
    SLU_Z
    } Dtype_t;
  
  
  typedef enum
    {
    SLU_GE,
    SLU_TRLU,
    SLU_TRUU,
    SLU_TRL,
    SLU_TRU,
    SLU_SYL,
    SLU_SYU,
    SLU_HEL,
    SLU_HEU
    } Mtype_t;
  
  
  typedef struct
    {
    Stype_t Stype;
    Dtype_t Dtype;
    Mtype_t Mtype;
    int_t   nrow;
    int_t   ncol;
    void*   Store;
    } SuperMatrix;
  
  
  typedef struct
    {
    int*    panel_histo;
    double* utime;
    float*  ops;
    int     TinyPivots;
    int     RefineSteps;
    int     expansions;
    } SuperLUStat_t;
  
  
  typedef enum {NO, YES}                                          yes_no_t;
  typedef enum {DOFACT, SamePattern, SamePattern_SameRowPerm, FACTORED} fact_t;
  typedef enum {NOROWPERM, LargeDiag, MY_PERMR}                   rowperm_t;
  typedef enum {NATURAL, MMD_ATA, MMD_AT_PLUS_A, COLAMD,
                METIS_AT_PLUS_A, PARMETIS, ZOLTAN, MY_PERMC}      colperm_t;
  typedef enum {NOTRANS, TRANS, CONJ}                             trans_t;
  typedef enum {NOREFINE, SLU_SINGLE=1, SLU_DOUBLE, SLU_EXTRA}    IterRefine_t;
  typedef enum {ONE_NORM, TWO_NORM, INF_NORM}                     norm_t;
  typedef enum {SILU, SMILU_1, SMILU_2, SMILU_3}                  milu_t;


  typedef struct
    {
    fact_t        Fact;
    yes_no_t      Equil;
    colperm_t     ColPerm;
    trans_t       Trans;
    IterRefine_t  IterRefine;
    double        DiagPivotThresh;
    yes_no_t      SymmetricMode;
    yes_no_t      PivotGrowth;
    yes_no_t      ConditionNumber;
    rowperm_t     RowPerm;
    int           ILU_DropRule;
    double        ILU_DropTol;
    double        ILU_FillFactor;
    norm_t        ILU_Norm;
    double        ILU_FillTol;
    milu_t        ILU_MILU;
    double        ILU_MILU_Dim;
    yes_no_t      ParSymbFact;
    yes_no_t      ReplaceTinyPivot;
    yes_no_t      SolveInitialized;
    yes_no_t      RefineInitialized;
    yes_no_t      PrintStat;
    int           nnzL, nnzU;
    int           num_lookaheads;
    yes_no_t      lookahead_etree;
    yes_no_t      SymPattern;
    } superlu_options_t;
  
  
  typedef struct
    {
    int_t  nnz;
    void*  nzval;
    int_t* rowind;
    int_t* colptr;
    } NCformat;
  
  
  typedef struct
    {
    int_t lda;
    void* nzval;
    } DNformat;
  
  }
}

#endif


#endif
