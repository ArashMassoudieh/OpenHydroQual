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

