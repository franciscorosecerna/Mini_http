#pragma once
#include "Request.h"

class Connection;

Request parseRequest(Connection& conn);