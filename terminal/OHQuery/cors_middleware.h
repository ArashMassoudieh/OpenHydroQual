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


#ifndef CORS_MIDDLEWARE_H
#define CORS_MIDDLEWARE_H

#include "crow.h"

struct CORS
{
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context&)
    {
        // Handle OPTIONS preflight globally
        if (req.method == crow::HTTPMethod::OPTIONS)
        {
            res.code = 200;
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.end();  // Stop processing — don't hit other routes
        }
    }

    void after_handle(crow::request& /*req*/, crow::response& res, context&)
    {
        // Add CORS headers to all responses
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }
};

#endif // CORS_MIDDLEWARE_H

