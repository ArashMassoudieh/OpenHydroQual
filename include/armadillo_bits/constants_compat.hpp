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


//! \addtogroup constants_compat
//! @{


// the Math and Phy classes are kept for compatibility with old code;
// for new code, use the Datum class instead
// eg. instead of math::pi(), use datum::pi

template<typename eT>
class Math
  {
  public:
  
  // the long lengths of the constants are for future support of "long double"
  // and any smart compiler that does high-precision computation at compile-time
  
  //! ratio of any circle's circumference to its diameter
  static eT pi()        { return eT(3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679); }
  
  //! base of the natural logarithm
  static eT e()         { return eT(2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274); }
  
  //! Euler's constant, aka Euler-Mascheroni constant
  static eT euler()     { return eT(0.5772156649015328606065120900824024310421593359399235988057672348848677267776646709369470632917467495); }
  
  //! golden ratio
  static eT gratio()    { return eT(1.6180339887498948482045868343656381177203091798057628621354486227052604628189024497072072041893911374); }
  
  //! square root of 2
  static eT sqrt2()     { return eT(1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727); }
  
  //! the difference between 1 and the least value greater than 1 that is representable
  static eT eps()       { return std::numeric_limits<eT>::epsilon(); }
  
  //! log of the minimum representable value
  static eT log_min()   { static const eT out = std::log(std::numeric_limits<eT>::min()); return out; }
    
  //! log of the maximum representable value
  static eT log_max()   { static const eT out = std::log(std::numeric_limits<eT>::max()); return out; }
  
  //! "not a number"
  static eT nan()       { return priv::Datum_helper::nan<eT>(); }
  
  //! infinity 
  static eT inf()       { return priv::Datum_helper::inf<eT>(); }
  };



//! Physical constants taken from NIST 2010 CODATA values, and some from WolframAlpha (values provided as of 2009-06-23)
//! http://physics.nist.gov/cuu/Constants
//! http://www.wolframalpha.com
//! See also http://en.wikipedia.org/wiki/Physical_constant
template<typename eT>
class Phy
  {
  public:
  
  //! atomic mass constant (in kg)
  static eT m_u()       {  return eT(1.660539040e-27); }
  
  //! Avogadro constant
  static eT N_A()       {  return eT(6.022140857e23); }
  
  //! Boltzmann constant (in joules per kelvin)
  static eT k()         {  return eT(1.38064852e-23); }
  
  //! Boltzmann constant (in eV/K)
  static eT k_evk()     {  return eT(8.6173303e-5); }
  
  //! Bohr radius (in meters)
  static eT a_0()       { return eT(0.52917721067e-10); }
  
  //! Bohr magneton
  static eT mu_B()      { return eT(927.4009994e-26); }
  
  //! characteristic impedance of vacuum (in ohms)
  static eT Z_0()       { return eT(3.76730313461771e-2); }
  
  //! conductance quantum (in siemens)
  static eT G_0()       { return eT(7.7480917310e-5); }
  
  //! Coulomb's constant (in meters per farad)
  static eT k_e()       { return eT(8.9875517873681764e9); }
  
  //! electric constant (in farads per meter)
  static eT eps_0()     { return eT(8.85418781762039e-12); }
  
  //! electron mass (in kg)
  static eT m_e()       { return eT(9.10938356e-31); }
  
  //! electron volt (in joules)
  static eT eV()        { return eT(1.6021766208e-19); }
  
  //! elementary charge (in coulombs)
  static eT e()         { return eT(1.6021766208e-19); }
  
  //! Faraday constant (in coulombs)
  static eT F()         { return eT(96485.33289); }
  
  //! fine-structure constant
  static eT alpha()     { return eT(7.2973525664e-3); }
  
  //! inverse fine-structure constant
  static eT alpha_inv() { return eT(137.035999139); }
  
  //! Josephson constant
  static eT K_J()       { return eT(483597.8525e9); }
  
  //! magnetic constant (in henries per meter)
  static eT mu_0()      { return eT(1.25663706143592e-06); }
  
  //! magnetic flux quantum (in webers)
  static eT phi_0()     { return eT(2.067833667e-15); }
  
  //! molar gas constant (in joules per mole kelvin)
  static eT R()         { return eT(8.3144598); }
  
  //! Newtonian constant of gravitation (in newton square meters per kilogram squared)
  static eT G()         { return eT(6.67408e-11); }
  
  //! Planck constant (in joule seconds)
  static eT h()         { return eT(6.626070040e-34); }
  
  //! Planck constant over 2 pi, aka reduced Planck constant (in joule seconds)
  static eT h_bar()     { return eT(1.054571800e-34); }
  
  //! proton mass (in kg)
  static eT m_p()       { return eT(1.672621898e-27); }
  
  //! Rydberg constant (in reciprocal meters)
  static eT R_inf()     { return eT(10973731.568508); }
  
  //! speed of light in vacuum (in meters per second)
  static eT c_0()       { return eT(299792458.0); }
  
  //! Stefan-Boltzmann constant
  static eT sigma()     { return eT(5.670367e-8); }
  
  //! von Klitzing constant (in ohms)
  static eT R_k()       { return eT(25812.8074555); }
  
  //! Wien wavelength displacement law constant
  static eT b()         { return eT(2.8977729e-3); }
  };



typedef Math<float>  fmath;
typedef Math<double> math;

typedef Phy<float>   fphy;
typedef Phy<double>  phy;



//! @}
