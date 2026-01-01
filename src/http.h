#pragma once

struct response {
    float ver;
    int status;
    char* reason;
    char* message;
};

void construct_response(struct response* rs, const char* request, const int len);