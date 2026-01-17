#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http.h"

void construct_response(struct response* rs, const char* request, const int len) 
{
    const char* p = request;
    char method[5];
    char* space = strchr(p, ' ');
    if (space == NULL) {
        rs->ver = 1.0;
        rs->status = 400;
        rs->reason = "Bad Request";
        rs->message = "<h1> Invalid Request <\\h1>";
        return;
    }
    
    int size = space - p;
    strncpy(method, p, size);
    method[4] = '\0';

    // Set response version to 1.0 always   
    rs->ver = 1.0;

    if (strcmp(method, "GET") == 0) {
        rs->status = 200;
        rs->reason = "OK";
        rs->message = "";
    } 
    else if (strcmp(method, "POST") == 0 || strcmp(method, "HEAD") == 0) {
        rs->status = 501;
        rs->reason = "Not Implemented";
        rs->message = "<h1> Request not implemented yet <\\h1>";
        return;
    }
    else {
        rs->status = 400;
        rs->reason = "Bad Request";
        rs->message = "<h1> Invalid Method <\\h1>";
        return;
    }

    p = space + 1;
    space = strchr(p, ' ');

    if (space == NULL) {
        space = strchr(p,'\r');
        //rs->ver = 0.9;
    }
    
    if (space == NULL) {
        space = strchr(p, '\n');
    }

    if (space == NULL) {
        rs->status = 400;
        rs->reason = "Bad Request";
        rs->message = "<h1> Invalid Format <\\h1>";
        return;
    }    

    
    size = space - p;
    char* url = malloc(size + 1);
    

    if (url == NULL) {
        fprintf(stderr, "unable to parse URL");
        exit(EXIT_FAILURE);
    }

    

    strncpy(url, p, size);
    if (*url != '/') {
        rs->status = 400;
        rs->reason = "Bad Request";
        rs->message = "<h1> Invalid URL <\\h>";
        return; 
    }

    if (*space == ' ') {
        char version[9];
        version[8] = '\0';
        p = space + 1;
        space = strchr(p, '\r');

        if (space == NULL) {
            rs->status = 400;
            rs->reason = "Bad Request";
            rs->message = "<h1> Invalid Format <\\h1>";
            return;
        }

        size = space - p;
        strncpy(version, p, size);
        if ((strcmp(version, "HTTP/1.0") != 0) || (*(space + 1) != '\n')) {
            rs->status = 400;
            rs->reason = "Bad Request";
            rs->message = "<h1> Invalid Version <\\h>";
        }   
    }
}