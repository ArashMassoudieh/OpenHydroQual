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


#ifndef SAFEVECTOR_H
#define SAFEVECTOR_H

#include <vector>
#include <iostream>

 /**
  * @brief SafeVector is a wrapper around std::vector with bounds-checked element access.
  *
  * @tparam T Type of elements stored in the vector.
  */
template<class T>
class SafeVector : public std::vector<T>
{
public:
    /// Default constructor
    SafeVector();

    /// Destructor
    ~SafeVector();

    /**
     * @brief Access element with bounds checking.
     * @param i Index (signed int)
     * @return Reference to the element if index is valid; otherwise, reference to a static dummy variable.
     */
    T& operator[](int i);

    /**
     * @brief Access element with bounds checking.
     * @param i Index (unsigned int)
     * @return Reference to the element if index is valid; otherwise, reference to a static dummy variable.
     */
    T& operator[](unsigned int i);

    /**
     * @brief Convert to std::vector<T>.
     * @return A copy of the SafeVector as std::vector<T>
     */
    std::vector<T> toStdVector() const;

    /**
     * @brief Construct a SafeVector from std::vector<T>.
     * @param x Standard vector to copy from
     * @return A new SafeVector<T> with the same contents
     */
    static SafeVector fromStdVector(const std::vector<T>& x);

    /**
     * @brief Set value at given index if valid.
     * @param i Index
     * @param val Value to set
     * @return true if successful, false if index is out of bounds
     */
    bool SetVal(unsigned int i, const T& val);

    /**
     * @brief Find the index of a value in the vector.
     * @param x Value to search for
     * @return Index of the first match, or size() if not found
     */
    unsigned int lookup(const T& x) const;

    /**
     * @brief Append elements from another SafeVector.
     * @param v Vector to append
     */
    void append(const SafeVector<T>& v);

    /**
     * @brief Append a single element to the vector.
     * @param x Element to append
     */
    void append(const T& x);
};

#include "safevector.hpp"

#endif // SAFEVECTOR_H

