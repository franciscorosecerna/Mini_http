#pragma once

namespace mini_http {
    enum class HttpMethod {
        GET,
        POST,
        PUT,
        DELETE_,
        PATCH,
        OPTIONS,
        HEAD
    };
}