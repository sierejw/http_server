#pragma once

struct response {
    int ver;
    int status;
    char* message;
};

void construct_response(struct response* rs, const char* request, const int len);