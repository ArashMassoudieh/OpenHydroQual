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
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#pragma once
#include <stdexcept>

namespace crow
{
    struct bad_request : public std::runtime_error
    {
        bad_request(const std::string& what_arg)
            : std::runtime_error(what_arg) {}

        bad_request(const char* what_arg)
            : std::runtime_error(what_arg) {}
    };
}