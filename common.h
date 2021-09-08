#ifndef _COMMON_H
#define _COMMON_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define STRING_SIZE 256
#define REPLY_SIZE  4096

#define SERVER_NAME "./3430Server"
#define CLIENT_NAME "./3430Client%c"

#define ERROR_MESSAGE "bad message"

// provide boolean for ourselves
typedef enum
{
  false,
  true
} boolean;

#endif

