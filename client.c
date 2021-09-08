#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
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


// extracts option information and returns the client number
static char parseOptions( int argc, char *argv[] )
{
  int ch;
  char id = -1;

  ch = getopt( argc, argv, "c:" );
  while ( -1 != ch )
  {
    switch( ch )
    {
      case 'c':
        id = optarg[0];
        break;
    }

    ch = getopt( argc, argv, "c:" );
  }

  if ( -1 == id )
    printf( "Usage: client -c x\n" );

  return id;
}

// use our protocol routines to parse and process the reply from the server
// if the message is bad, returns false to indicate that there was a problem
static boolean processReply( ProtocolData *protocol, char *message )
{
  boolean success = true;

  // only perform further processing if we have a valid message
  if ( protocol->completeMessage )
  {
    // validate that this is from the server
    if ( 'S' == protocol->sourceID )
    {
      // print out the results
      if ( 0 != strcmp( protocol->message, ERROR_MESSAGE ) )
      {
        printf( "%s\n\n", protocol->message );
        fflush( stdout );
      }

      // we had a problem
      else
      {
        fprintf( stderr, "Invalid message \"%s\" sent to the server, exiting...\n", message );
        success = false;
      }
    }

    // invalid message source
    else
      fprintf( stderr, "Message received from unkown source\n" );
  }

  else
    fprintf( stderr, "Error due to partial message reception with \"%s\" of length %d\n", protocol->message, protocol->messageIndex );

  return success;
}

// retreive a response and print it to standard output
static void receiveReply(char *fifoName)
{
  char message[STRING_SIZE];
  int clientFIFO = -1;
  boolean done = false;
  int replySize;
  char next_byte;
  ProtocolData protocol;

  // open up our FIFO so we can receive results -- we do it here since we otherwise wait for a writer
  clientFIFO = open( fifoName, O_RDONLY, 0 );
  if ( -1 != clientFIFO )
  {
    // get ready to process the message
    initProtocol( &protocol );

    // read the reply 1 byte at a time until we get our response
    while ( !protocol.completeMessage )
    {
      replySize = read( clientFIFO, &next_byte, 1 );
      if ( 1 == replySize )
      {
        processProtocol( next_byte, &protocol );
        if ( protocol.completeMessage )
        {
          done = !processReply( &protocol, message );
        }
      }
    }

    // close the FIFO until we're ready to receive another message
    close( clientFIFO );
  }

  else
    fprintf( stderr, "unable to open client fifo\n" );
}

// read from standard input and transmit the contents to our server for processing
int main( int argc, char *argv[] )
{
  char message[STRING_SIZE];
  char fifoName[STRING_SIZE];
  char id = parseOptions( argc, argv );
  int serverFIFO = -1;
  boolean done = false;
  char buffer[REPLY_SIZE];

  // only continue if we have a valid ID
  if ( -1 != id )
  {
    // make sure we have our fifo
    sprintf( fifoName, CLIENT_NAME, id );
    mkfifo( fifoName, 0666 );

    while ( !done && fgets( message, STRING_SIZE, stdin ) )
    {
      // assume that the message contains a new line at the end of the message that we want to ignore...
      message[strlen( message )-1] = '\0';

      // since we don't know how often we'll have messages we want to be nice and only have the FIFO
      // open while we're actually talking to the server. It also allows us to deal with a dead server.
      serverFIFO = open( SERVER_NAME, O_WRONLY, 0 );
      if ( -1 != serverFIFO )
      {
        // build the message info we need -- assumes a single digit for the ID...
        // this sure is dangerous, given that the input hasn't been sanitized!
        sprintf( buffer, "ST%c%sET%c", id, message, id );
        // transmit the request to the server
        // we always write the entire message at once since the FIFO guarantees that the message will be atomic
        write( serverFIFO, buffer, strlen( buffer ) );

        // done with the server FIFO
        close( serverFIFO );

        receiveReply(fifoName);
      }

      else
        fprintf( stderr, "unable to open server fifo\n" );
    }
    
    // clean up our FIFO
    unlink( fifoName );
  }

  return 0;
}