#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "common.h"

// This enum really shouldn't be here but is needed for the struct definition,
// but I didn't feel like doing any void pointer stuff to hide the internal data.

// states that we use to track messages
enum PROTOCOL_STATES
{
  START_TAG_1,       // looking for the initial start char
  START_TAG_2,       // looking for the second start char
  START_ID,          // looking for the ID in start tag
  END_TAG_1,         // got a potential initial end char
  END_TAG_2,         // looking for the second end char
  END_ID             // looking for the ID in end tag
};

typedef enum PROTOCOL_STATES ProtocolState;

// information we need to pass back and forth to process messages
struct PROTOCOL_DATA
{
  ProtocolState currState;
  char message[REPLY_SIZE];
  int messageIndex;
  boolean completeMessage;
  char sourceID;
};

typedef struct PROTOCOL_DATA ProtocolData;

void initProtocol( ProtocolData *state );
void processProtocol( char byte, ProtocolData *state );

#endif

