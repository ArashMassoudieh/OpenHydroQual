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



class glue_histc
   {
   public:
   
   template<typename eT>
   inline static void apply_noalias(Mat<uword>& C, const Mat<eT>& A, const Mat<eT>& B, const uword dim);
   
   template<typename T1, typename T2>
   inline static void apply(Mat<uword>& C, const mtGlue<uword,T1,T2,glue_histc>& expr);
   };



class glue_histc_default
   {
   public:
   
   template<typename T1, typename T2>
   inline static void apply(Mat<uword>& C, const mtGlue<uword,T1,T2,glue_histc_default>& expr);
   };
