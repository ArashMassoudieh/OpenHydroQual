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
#include "crow/http_request.h"
#include "crow/http_response.h"

namespace crow
{

    struct UTF8
    {
        struct context
        {};

        void before_handle(request& /*req*/, response& /*res*/, context& /*ctx*/)
        {}

        void after_handle(request& /*req*/, response& res, context& /*ctx*/)
        {
            if (get_header_value(res.headers, "Content-Type").empty())
            {
                res.set_header("Content-Type", "text/plain; charset=utf-8");
            }
        }
    };

} // namespace crow
