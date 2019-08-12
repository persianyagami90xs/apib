/*
Copyright 2019 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef APIB_MESSAGE_H
#define APIB_MESSAGE_H

#include <sys/types.h>

#include "src/apib_lines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { Request, Response } MessageType;

// State values. They are increasing integers so that you can do a ">"
#define MESSAGE_INIT 0
#define MESSAGE_STATUS 1
#define MESSAGE_HEADERS 2
#define MESSAGE_BODY 3
#define MESSAGE_DONE 4

#define CHUNK_INIT 0
#define CHUNK_LENGTH 1
#define CHUNK_CHUNK 2
#define CHUNK_END 3

typedef struct {
  MessageType type;
  int state;

  // Available when state >= MESSAGE_STATUS
  int majorVersion;
  int minorVersion;

  // Only available for a response
  int statusCode;

  // Only available for a request
  char* method;
  char* path;

  // Available when state >= MESSAGE_HEADERS
  int contentLength;
  int chunked;
  int shouldClose;

  // Available when state >= MESSAGE_BODY
  int bodyLength;

  // Internal stuff for chunked encoding.
  int chunkState;
  int chunkLength;
  int chunkPosition;
} HttpMessage;

/*
One-time initialization of response processing. This is required to be called once,
and is generally idempotent but doesn't go to great lengths to ensure that
in a multi-threaded environment.
*/
extern void message_Init();

/*
Create a response object.
*/
extern HttpMessage* message_NewResponse();

/*
Create a request.
*/
extern HttpMessage* message_NewRequest();

/*
Free a message.
*/
extern void message_Free(HttpMessage* r);

/*
Add data to a response object. The data should consist of a valid
HTTP response. Returns 0 on success and non-zero on error.
Callers should check the "state" parameter and keep feeding data
until the state is MESSAGE_DONE. An error means that we got 
invalid HTTP data. The passed "LineState" MUST be in "HTTP" mode.
*/
extern int message_Fill(HttpMessage* r, LineState* buf);

#ifdef __cplusplus
}
#endif

#endif  // APIB_MESSAGE_H