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
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup subview_cube_each
//! @{



template<typename eT>
class subview_cube_each_common
  {
  public:
  
  const Cube<eT>& p;
  
  inline void check_size(const Mat<eT>& A) const;
  
  
  protected:
  
  arma_inline subview_cube_each_common(const Cube<eT>& in_p);
  
  arma_cold inline const std::string incompat_size_string(const Mat<eT>& A) const;
  
  
  private:
  
  subview_cube_each_common();
  };




template<typename eT>
class subview_cube_each1 : public subview_cube_each_common<eT>
  {
  protected:
  
  arma_inline subview_cube_each1(const Cube<eT>& in_p);
  
  
  public:
  
  inline ~subview_cube_each1();
  
  // deliberately returning void
  template<typename T1> inline void operator=  (const Base<eT,T1>& x);
  template<typename T1> inline void operator+= (const Base<eT,T1>& x);
  template<typename T1> inline void operator-= (const Base<eT,T1>& x);
  template<typename T1> inline void operator%= (const Base<eT,T1>& x);
  template<typename T1> inline void operator/= (const Base<eT,T1>& x);
  
  
  private:
  
  friend class Cube<eT>;
  };



template<typename eT, typename TB>
class subview_cube_each2 : public subview_cube_each_common<eT>
  {
  protected:
  
  inline subview_cube_each2(const Cube<eT>& in_p, const Base<uword, TB>& in_indices);
  
  
  public:
  
  const Base<uword, TB>& base_indices;
  
  inline void check_indices(const Mat<uword>& indices) const;
  inline ~subview_cube_each2();
  
  // deliberately returning void
  template<typename T1> inline void operator=  (const Base<eT,T1>& x);
  template<typename T1> inline void operator+= (const Base<eT,T1>& x);
  template<typename T1> inline void operator-= (const Base<eT,T1>& x);
  template<typename T1> inline void operator%= (const Base<eT,T1>& x);
  template<typename T1> inline void operator/= (const Base<eT,T1>& x);
  
  
  private:
  
  friend class Cube<eT>;
  };



class subview_cube_each1_aux
  {
  public:
  
  template<typename eT, typename T2>
  static inline Cube<eT> operator_plus(const subview_cube_each1<eT>& X, const Base<eT,T2>& Y);
    
  template<typename eT, typename T2>
  static inline Cube<eT> operator_minus(const subview_cube_each1<eT>& X, const Base<eT,T2>& Y);
  
  template<typename T1, typename eT>
  static inline Cube<eT> operator_minus(const Base<eT,T1>& X, const subview_cube_each1<eT>& Y);
  
  template<typename eT, typename T2>
  static inline Cube<eT> operator_schur(const subview_cube_each1<eT>& X, const Base<eT,T2>& Y);
  
  template<typename eT, typename T2>
  static inline Cube<eT> operator_div(const subview_cube_each1<eT>& X,const Base<eT,T2>& Y);
  
  template<typename T1, typename eT>
  static inline Cube<eT> operator_div(const Base<eT,T1>& X, const subview_cube_each1<eT>& Y);
  };



class subview_cube_each2_aux
  {
  public:
  
  template<typename eT, typename TB, typename T2>
  static inline Cube<eT> operator_plus(const subview_cube_each2<eT,TB>& X, const Base<eT,T2>& Y);
    
  template<typename eT, typename TB, typename T2>
  static inline Cube<eT> operator_minus(const subview_cube_each2<eT,TB>& X, const Base<eT,T2>& Y);
  
  template<typename T1, typename eT, typename TB>
  static inline Cube<eT> operator_minus(const Base<eT,T1>& X, const subview_cube_each2<eT,TB>& Y);
  
  template<typename eT, typename TB, typename T2>
  static inline Cube<eT> operator_schur(const subview_cube_each2<eT,TB>& X, const Base<eT,T2>& Y);
  
  template<typename eT, typename TB, typename T2>
  static inline Cube<eT> operator_div(const subview_cube_each2<eT,TB>& X, const Base<eT,T2>& Y);
  
  template<typename T1, typename eT, typename TB>
  static inline Cube<eT> operator_div(const Base<eT,T1>& X, const subview_cube_each2<eT,TB>& Y);
  };



//! @}
