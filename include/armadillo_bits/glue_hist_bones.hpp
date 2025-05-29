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


// Copyright (C) 2012-2015 Conrad Sanderson
// Copyright (C) 2012-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



class glue_hist
   {
   public:
   
   template<typename eT>
   inline static void apply_noalias(Mat<uword>& out, const Mat<eT>& X, const Mat<eT>& C, const uword dim);
   
   template<typename T1, typename T2>
   inline static void apply(Mat<uword>& out, const mtGlue<uword,T1,T2,glue_hist>& expr);
   };



class glue_hist_default
   {
   public:
   
   template<typename T1, typename T2>
   inline static void apply(Mat<uword>& out, const mtGlue<uword,T1,T2,glue_hist_default>& expr);
   };
