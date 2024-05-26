#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "http.h"


// reads through buffer to find headers
int make_headerpair(HTTPRequest *req, char* buffer, size_t buffer_idx, ssize_t buffer_len) {
  char *k = malloc(buffer_len); // stores key
  memset(k, 0, buffer_len);
  char *v = malloc(buffer_len); // stores value
  memset(v, 0, buffer_len);

  int kv_idx = 0; // keeps track of index in key/value
  int is_key = 1; // checks if looking for key or value
  int skip =  0;
  unsigned i = 0;
  for (i = buffer_idx; i < buffer_len; i++) {
    // skips over \n and spaces
    if (buffer[i] == '\n') skip = 1; 
    if (buffer[i] == ' ') skip = 1;
    if (buffer[i] == '\r') { // end of data
      v[kv_idx] = '\0';
      
      if ((k[0] == 0 && v[0] == 0) || // key and value are null or is duplicate
        (req->header_count > 0 && strcmp(req->headers[req->header_count-1]->key, k) == 0)) {
        free(k);
        free(v);
        return -1;
      }
      HeaderPair *tmp = malloc(sizeof (HeaderPair)); // create new pair
      tmp->key = calloc(strlen(k)+1, sizeof(char));
      strcpy(tmp->key, k);
      tmp->value = calloc(strlen(v)+1, sizeof(char));
      strcpy(tmp->value, v);
      if (strcmp(tmp->key, "Content-Length") == 0) {
        req->content_len = malloc(strlen(tmp->value)+1);
        strcpy(req->content_len, tmp->value);
        // req->content_len = tmp->value;
      }
      req->headers[req->header_count] = tmp; // update req
      req->header_count++;
      break;
    }
    if (buffer[i] == ':') {
      k[kv_idx] = '\0';
      is_key = 0;
      kv_idx = 0;
    }
    else if (!skip) {
      if (is_key) { // adds to k
        k[kv_idx] = buffer[i];
      } else v[kv_idx] = buffer[i]; // adds to v
      kv_idx++;
    }
    skip = 0;
  }
  free(k);
  free(v);
  return i;
}

/**
 * httprequest_parse_headers
 * 
 * Populate a `req` with the contents of `buffer`, returning the number of bytes used from `buf`.
 */
ssize_t httprequest_parse_headers(HTTPRequest *req, char *buffer, ssize_t buffer_len) {
  req->header_count = 0;
  int buffer_idx = 0;
  char *tmp_act = malloc(buffer_len); // stores action
  memset(tmp_act, 0, buffer_len);
  for (unsigned i = 0; i < buffer_len; i++) {
    if (buffer[i] == ' ') {
      tmp_act[i] = '\0';
      buffer_idx = i+1; // skips over " " 
      break;
    }
    tmp_act[i] = buffer[i];
  }
  req->action = calloc(strlen(tmp_act)+1, sizeof(char));
  strcpy(req->action, tmp_act);
  char *tmp_path = malloc(buffer_len); // stores path
  for (unsigned i = buffer_idx; i < buffer_len; i++) {
    if (buffer[i] == ' ') {
      tmp_path[i-buffer_idx] = '\0';
      buffer_idx = i + 1; // skips over " " 
      break;
    }
    tmp_path[i-buffer_idx] = buffer[i];
  }
  req->path = calloc(strlen(tmp_path) + 1,  sizeof(char)); 
  strcpy(req->path, tmp_path);
  char *tmp_ver = malloc(buffer_len); // stores version
  for (unsigned i = buffer_idx; i < buffer_len; i++) {
    if (buffer[i] == '\r') {
      tmp_ver[i-buffer_idx] = '\0';
      buffer_idx = i+1; // skips over \n
      break;
    }
    tmp_ver[i-buffer_idx] = buffer[i];
  }
  req->version = calloc(strlen(tmp_ver)+1, sizeof(char));
  strcpy(req->version, tmp_ver);
  req->content_len = NULL;
  while (buffer_idx < buffer_len) { // check for headers
    int idx = make_headerpair(req, buffer, buffer_idx, buffer_len);
    if (idx == -1) break; // end of buffer
    else buffer_idx = idx + 2;
  }
  req->payload = NULL;
  free(tmp_act);
  free(tmp_path);
  free(tmp_ver);
  return strlen(req->action) + strlen(req->version);
}


/**
 * httprequest_read
 * 
 * Populate a `req` from the socket `sockfd`, returning the number of bytes read to populate `req`.
 */
ssize_t httprequest_read(HTTPRequest *req, int sockfd) {
  int Length = (5 * 1024 * 1024) + 150;
  char *buffer = malloc(Length);
  ssize_t total_bytes = 0;
  while (total_bytes < Length) {
      ssize_t r = read(sockfd, buffer+total_bytes, Length-total_bytes);
      if(r < 1){
          break;
      }
      total_bytes+=r;
  }
  
  buffer[total_bytes] = '\0';
  ssize_t bytes = httprequest_parse_headers(req, buffer, strlen(buffer));
  req->payload = NULL;
  char* content_len = httprequest_get_header(req, "Content-Length");
  if (content_len) {
    int amt = atoi(content_len);
   
    if (amt > 0) {
      req->payload = malloc(amt+1);
      memcpy(req->payload, &buffer[total_bytes-amt], amt+1);
    }
  } 
  free(buffer);
  return bytes;
}


/**
 * httprequest_get_action
 * 
 * Returns the HTTP action verb for a given `req`.
 */
const char *httprequest_get_action(HTTPRequest *req) {
  return req->action;
}


/**
 * httprequest_get_header
 * 
 * Returns the value of the HTTP header `key` for a given `req`.
 */
const char *httprequest_get_header(HTTPRequest *req, const char *key) {
  if (strcmp(key, "Content-Length") == 0) {
    return req->content_len;
  }
  for (int i = 0; i < req->header_count; i++) {
    if (req->headers[i]->key == NULL) continue;
    if (strcmp(req->headers[i]->key, key) == 0) return req->headers[i]->value;
  }
  
  return NULL;
}


/**
 * httprequest_get_path
 * 
 * Returns the requested path for a given `req`.
 */
const char *httprequest_get_path(HTTPRequest *req) {
  return req->path;
}


/**
 * httprequest_destroy
 * 
 * Destroys a `req`, freeing all associated memory.
 */
void httprequest_destroy(HTTPRequest *req) {
  free(req->action);
  free(req->version);
  free(req->path);
  if (req->content_len) free(req->content_len);
  if (req->payload) free(req->payload);
  for (int i = 0; i < req->header_count; i++) {
    HeaderPair *tmp = req->headers[i];
    free(tmp->key);
    free(tmp->value);
    free(tmp);
  }
  
}