#pragma once
#include "Request.h"

namespace mini_http {
    class Connection;

    Request parseRequest(Connection& conn);
}