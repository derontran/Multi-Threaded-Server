#include "protocol.h"

//-------------------------------------------------------------------------------------
// CONSTANTS and TYPES
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// VARIABLES
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// PROTOTYPES
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------


// track our messages to validate the message structure, on any errors we reset to look for another start of message
void processProtocol( char byte, ProtocolData *state )
{
  switch( state->currState )
  {
    // hunt for a starting message tag
    case START_TAG_1:
      if ( byte == 'S' )
        state->currState = START_TAG_2;
      break;

    case START_TAG_2:
      if ( byte == 'T' )
        state->currState = START_ID;
      else
        state->currState = START_TAG_1;
      break;

    case START_ID:
      // extract the ID and move on -- assumes the ID is always 1 character...
      state->sourceID = byte;
      state->messageIndex = 0;
      state->currState = END_TAG_1;
      break;

    // hunt for the end message tag
    case END_TAG_1:
      if ( byte != 'E' )
      {
        // we should really do more than this to protect against buffer overruns!
        if ( state->messageIndex < REPLY_SIZE-4 )
          state->message[state->messageIndex++] = byte;
      }
      else
        state->currState = END_TAG_2;
      break;

    case END_TAG_2:
      // not an end of message, include text and keep searching
      if ( byte != 'T' )
      {
        if ( state->messageIndex < REPLY_SIZE-4 )
        {
          state->message[state->messageIndex++] = 'E';
          state->message[state->messageIndex++] = byte;
        }
        state->currState = END_TAG_1;
      }
      else
        state->currState = END_ID;
      break;

    case END_ID:
      if ( byte == state->sourceID )
      {
        state->completeMessage = true;
        state->message[state->messageIndex] = '\0';
        state->currState = START_TAG_1;
      }
      // not an end of message, include text and keep searching
      else
      {
        if ( state->messageIndex < REPLY_SIZE-4 )
        {
          state->message[state->messageIndex++] = 'E';
          state->message[state->messageIndex++] = 'T';
          state->message[state->messageIndex++] = byte;
        }
        state->currState = END_TAG_1;
      }
      break;
  }
}

// intializes all data related to protocol processing to start handling messages
void initProtocol( ProtocolData *state )
{
  state->currState = START_TAG_1;
  state->message[0] = '\0';
  state->messageIndex = 0;
  state->completeMessage = false;
}
