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


// Copyright (C) 2008-2013 Conrad Sanderson
// Copyright (C) 2008-2013 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


//! \addtogroup Col
//! @{

//! Class for column vectors (matrices with only one column)

template<typename eT>
class Col : public Mat<eT>
  {
  public:
  
  typedef eT                                elem_type;
  typedef typename get_pod_type<eT>::result pod_type;
  
  static const bool is_col = true;
  static const bool is_row = false;
  
  inline          Col();
  inline          Col(const Col<eT>& X);
  inline explicit Col(const uword n_elem);
  inline          Col(const uword in_rows, const uword in_cols);
  
  template<typename fill_type> inline Col(const uword n_elem,                       const fill::fill_class<fill_type>& f);
  template<typename fill_type> inline Col(const uword in_rows, const uword in_cols, const fill::fill_class<fill_type>& f);
  
  inline                  Col(const char*        text);
  inline const Col& operator=(const char*        text);
  
  inline                  Col(const std::string& text);
  inline const Col& operator=(const std::string& text);
  
  inline                  Col(const std::vector<eT>& x);
  inline const Col& operator=(const std::vector<eT>& x);
  
  #if defined(ARMA_USE_CXX11)
  inline                  Col(const std::initializer_list<eT>& list);
  inline const Col& operator=(const std::initializer_list<eT>& list);
  
  inline                  Col(Col&& m);
  inline const Col& operator=(Col&& m);
  #endif
  
  inline explicit Col(const SpCol<eT>& X);
  
  inline const Col& operator=(const eT val);
  inline const Col& operator=(const Col& m);
  
  template<typename T1> inline                   Col(const Base<eT,T1>& X);
  template<typename T1> inline const Col&  operator=(const Base<eT,T1>& X);
  
  inline Col(      eT* aux_mem, const uword aux_length, const bool copy_aux_mem = true, const bool strict = true);
  inline Col(const eT* aux_mem, const uword aux_length);
  
  template<typename T1, typename T2>
  inline explicit Col(const Base<pod_type,T1>& A, const Base<pod_type,T2>& B);
  
  template<typename T1> inline                  Col(const BaseCube<eT,T1>& X);
  template<typename T1> inline const Col& operator=(const BaseCube<eT,T1>& X);
  
  inline                  Col(const subview_cube<eT>& X);
  inline const Col& operator=(const subview_cube<eT>& X);
  
  inline mat_injector<Col> operator<<(const eT val);
  
  arma_inline const Op<Col<eT>,op_htrans>  t() const;
  arma_inline const Op<Col<eT>,op_htrans> ht() const;
  arma_inline const Op<Col<eT>,op_strans> st() const;
  
  arma_inline       subview_col<eT> row(const uword row_num);
  arma_inline const subview_col<eT> row(const uword row_num) const;
  
  using Mat<eT>::rows;
  using Mat<eT>::operator();
  
  arma_inline       subview_col<eT> rows(const uword in_row1, const uword in_row2);
  arma_inline const subview_col<eT> rows(const uword in_row1, const uword in_row2) const;
  
  arma_inline       subview_col<eT> subvec(const uword in_row1, const uword in_row2);
  arma_inline const subview_col<eT> subvec(const uword in_row1, const uword in_row2) const;
  
  arma_inline       subview_col<eT> rows(const span& row_span);
  arma_inline const subview_col<eT> rows(const span& row_span) const;
  
  arma_inline       subview_col<eT> subvec(const span& row_span);
  arma_inline const subview_col<eT> subvec(const span& row_span) const;
  
  arma_inline       subview_col<eT> operator()(const span& row_span);
  arma_inline const subview_col<eT> operator()(const span& row_span) const;
  
  
  inline void shed_row (const uword row_num);
  inline void shed_rows(const uword in_row1, const uword in_row2);
  
                        inline void insert_rows(const uword row_num, const uword N, const bool set_to_zero = true);
  template<typename T1> inline void insert_rows(const uword row_num, const Base<eT,T1>& X);
  
  
  arma_inline arma_warn_unused       eT& at(const uword i);
  arma_inline arma_warn_unused const eT& at(const uword i) const;
  
  arma_inline arma_warn_unused       eT& at(const uword in_row, const uword in_col);
  arma_inline arma_warn_unused const eT& at(const uword in_row, const uword in_col) const;
  
  
  typedef       eT*       row_iterator;
  typedef const eT* const_row_iterator;
  
  inline       row_iterator begin_row(const uword row_num);
  inline const_row_iterator begin_row(const uword row_num) const;
  
  inline       row_iterator end_row  (const uword row_num);
  inline const_row_iterator end_row  (const uword row_num) const;
  
  
  template<uword fixed_n_elem> class fixed;
  
  
  protected:
  
  inline Col(const arma_fixed_indicator&, const uword in_n_elem, const eT* in_mem);
  
  
  public:
  
  #ifdef ARMA_EXTRA_COL_PROTO
    #include ARMA_INCFILE_WRAP(ARMA_EXTRA_COL_PROTO)
  #endif
  };



template<typename eT>
template<uword fixed_n_elem>
class Col<eT>::fixed : public Col<eT>
  {
  private:
  
  static const bool use_extra = (fixed_n_elem > arma_config::mat_prealloc);
  
  arma_align_mem eT mem_local_extra[ (use_extra) ? fixed_n_elem : 1 ];
  
  arma_inline void change_to_row();
  
  
  public:
  
  typedef fixed<fixed_n_elem>               Col_fixed_type;
  
  typedef eT                                elem_type;
  typedef typename get_pod_type<eT>::result pod_type;
  
  static const bool is_col = true;
  static const bool is_row = false;
  
  static const uword n_rows = fixed_n_elem;
  static const uword n_cols = 1;
  static const uword n_elem = fixed_n_elem;
  
  arma_inline fixed();
  arma_inline fixed(const fixed<fixed_n_elem>& X);
       inline fixed(const subview_cube<eT>& X);
  
  template<typename fill_type>       inline fixed(const fill::fill_class<fill_type>& f);
  template<typename T1>              inline fixed(const Base<eT,T1>& A);
  template<typename T1, typename T2> inline fixed(const Base<pod_type,T1>& A, const Base<pod_type,T2>& B);
  
  inline fixed(const eT* aux_mem);
  
  inline fixed(const char*        text);
  inline fixed(const std::string& text);
  
  template<typename T1> inline const Col& operator=(const Base<eT,T1>& A);
  
  inline const Col& operator=(const eT val);
  inline const Col& operator=(const char*        text);
  inline const Col& operator=(const std::string& text);
  inline const Col& operator=(const subview_cube<eT>& X);
  
  using Col<eT>::operator();
  
  #if defined(ARMA_USE_CXX11)
    inline                fixed(const std::initializer_list<eT>& list);
    inline const Col& operator=(const std::initializer_list<eT>& list);
  #endif
  
  arma_inline const Col& operator=(const fixed<fixed_n_elem>& X);
  
  arma_inline const Op< Col_fixed_type, op_htrans >  t() const;
  arma_inline const Op< Col_fixed_type, op_htrans > ht() const;
  arma_inline const Op< Col_fixed_type, op_strans > st() const;
  
  arma_inline arma_warn_unused const eT& at_alt     (const uword i) const;
  
  arma_inline arma_warn_unused       eT& operator[] (const uword i);
  arma_inline arma_warn_unused const eT& operator[] (const uword i) const;
  arma_inline arma_warn_unused       eT& at         (const uword i);
  arma_inline arma_warn_unused const eT& at         (const uword i) const;
  arma_inline arma_warn_unused       eT& operator() (const uword i);
  arma_inline arma_warn_unused const eT& operator() (const uword i) const;
  
  arma_inline arma_warn_unused       eT& at         (const uword in_row, const uword in_col);
  arma_inline arma_warn_unused const eT& at         (const uword in_row, const uword in_col) const;
  arma_inline arma_warn_unused       eT& operator() (const uword in_row, const uword in_col);
  arma_inline arma_warn_unused const eT& operator() (const uword in_row, const uword in_col) const;
  
  arma_inline arma_warn_unused       eT* memptr();
  arma_inline arma_warn_unused const eT* memptr() const;
  
  arma_hot inline const Col<eT>& fill(const eT val);
  arma_hot inline const Col<eT>& zeros();
  arma_hot inline const Col<eT>& ones();
  };



//! @}
