/* Course: Comp3430
 * Assignment : 2 
 * Question: 1
 * Student Name : Thai Tran
 * Student number : 7785767
 * Userid : tranttt 
 * Purpose: implement a concurrency server
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "common.h"
#include "protocol.h"
#include "threadpool.h"

//-------------------------------------------------------------------------------------
// CONSTANTS and TYPES
//-------------------------------------------------------------------------------------

#define START_TAG "STS"
#define END_TAG "ETS"

#define MESSAGE_FIELDS 9

#ifndef NUM_CLIENTS
#define NUM_CLIENTS 10
#endif

// indicator that a thread can exit
#define TERMINATE -1

//Thread num
#define THREAD_NUM 4

#define THREAD_MAX 20
//-------------------------------------------------------------------------------------
// VARIABLES
//-------------------------------------------------------------------------------------

// track the FIFO id here so we can exit cleanly...
static int serverFIFO;

// stores our message template
static char template[REPLY_SIZE];

// information we need to pass on to a thread for message processing
static char message[STRING_SIZE];
static int messageSize = 0;
static int clientID;

// Semaphore initialize
static sem lock, worker_done_buffer, shared_buffer_ready, worker_ready;

//-------------------------------------------------------------------------------------
// PROTOTYPES
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------

// extracts the message data and generates the reply message, returns the length of the reply
static int processMessage(char *message, char *reply)
{
  char cust_data[MESSAGE_FIELDS][STRING_SIZE];
  int element = 0;
  char *next;
  char **last = &message;
  char ch;
  int index = 0;

  // parse the data into separate elements
  // since we're reusing this code we have to use the reentrant version of strtok...
  next = strtok_r(message, "|", last);
  for (element = 0; element < MESSAGE_FIELDS && next != NULL; element++)
  {
    strcpy(cust_data[element], next);
    next = strtok_r(NULL, "|", last);
  }

  // start with the initial message tag
  strcpy(&reply[index], START_TAG);
  index += 3;

  // make sure we have valid data
  if (MESSAGE_FIELDS == element)
  {
    // generate the output by reading and parsing the template
    element = 0;
    ch = template[element++];
    while (element < strlen(template))
    {
      switch (ch)
      {
      // check for tags
      case '$':
        ch = template[element++];
        if (ch == '$')
          reply[index++] = '$';
        else
        {
          strcpy(&reply[index], cust_data[atoi(&ch)]);
          index += strlen(cust_data[atoi(&ch)]);
        }
        break;

      // simply include all other characters
      default:
        reply[index++] = ch;
        break;
      }

      ch = template[element++];
    }
  }

  // indicate a message problem
  else
  {
    strcpy(&reply[index], ERROR_MESSAGE);
    index += strlen(ERROR_MESSAGE);
  }

  strcpy(&reply[index], END_TAG);
  index += strlen(END_TAG);
  reply[index] = '\0';

  return index;
}

// caches the template file for responding to requests
static void loadTemplate()
{
  FILE *templateFile = NULL;
  int index = 0;

  templateFile = fopen("template.txt", "r");
  if (templateFile != NULL)
  {
    // load the template file into our buffer
    while (fgets(&template[index], STRING_SIZE, templateFile) != NULL)
      index += strlen(&template[index]);

    fclose(templateFile);
  }
}

// catches a TERM signal so we can exit, this is the default kill signal...
static void exitServer(int theSignal)
{
  if (SIGTERM == theSignal)
  {
    // clean up our FIFO
    printf("%s", "catch exit signal");
    close(serverFIFO);
    unlink(SERVER_NAME);

    // wake up all waiting thread;
    sem_broadcast(&lock);
    sem_broadcast(&worker_done_buffer);
    sem_broadcast(&worker_ready);
    sem_broadcast(&shared_buffer_ready);
    
    // signal time to stop, wait for all thread finish and free up memory
    clean(&pool);
    
    // destroy all the lock that been created 
    sem_destroy(&lock);
    sem_destroy(&worker_done_buffer);
    sem_destroy(&worker_ready);
    sem_destroy(&shared_buffer_ready);

    exit(0);
  }
}

// catches a HUP signal so we can reload the template if we change it
// I thought it would be cool to not have to restart the server...
// static void reloadTemplate( int theSignal )
// {
//   if ( SIGHUP == theSignal )
//   {
//     loadTemplate();
//     signal( SIGHUP, reloadTemplate );
//   }
// }

// process the message and send the result to the requesting client
void messageHandler()
{
  char myMessage[STRING_SIZE];
  int myMessageSize = 0;
  int myClientID;
  int clientFIFO;
  char fifoName[STRING_SIZE];
  char reply[REPLY_SIZE];
  int replySize;

  // copy the data we need so the main thread can continue
  sem_wait(&lock);
  myClientID = clientID;
  myMessageSize = messageSize;
  strcpy(myMessage, message);
  sem_post(&lock);
  sem_post(&worker_done_buffer);

  // reply to the client
  sprintf(fifoName, CLIENT_NAME, myClientID);
  clientFIFO = open(fifoName, O_WRONLY, 0);
  if (-1 != clientFIFO)
  {
    replySize = processMessage(myMessage, reply);
    // we always write the entire message at once since the FIFO guarantees that the message will be atomic
    write(clientFIFO, reply, replySize);
    close(clientFIFO);
  }
  else
    fprintf(stderr, "unable to open client %c fifo\n", myClientID);
}
// work dispacher for each worker thread
void *dispatch()
{
  while (1)
  {
    sem_wait(&shared_buffer_ready);
    if (pool.shutdown == TERMINATE)
    {
      pthread_exit(0);
      break;
    }
    (*(pool.function))();
    sem_post(&worker_ready);
  }
}
// setup our server system
static void init()
{
  // initialize our semaphore or just a lock or counting variable
  sem_init(&lock, 1);
  sem_init(&worker_done_buffer, 0);
  sem_init(&shared_buffer_ready, 0);
  sem_init(&worker_ready, THREAD_NUM);

  // where the live of worker begin
  threadpool_init(messageHandler, dispatch, THREAD_NUM);

  // initialize our signal catchers
  signal(SIGTERM, exitServer);
  //signal( SIGHUP, reloadTemplate );

  loadTemplate();

  // make sure we have our fifo
  mkfifo(SERVER_NAME, 0666);
}

// reads requests from the FIFO and sends back results
int main(int argc, char *argv[])
{
  char mainMessage[STRING_SIZE];
  int mainMessageSize = 0;
  int mainClientID;
  char next_byte;
  ProtocolData protocol;
  int readSize;

  // setup our server

  init();
  // open up our FIFO so we can receive requests
  serverFIFO = open(SERVER_NAME, O_RDONLY, 0);
  if (-1 != serverFIFO)
  {
    initProtocol(&protocol);

    // wait for messages to process, until we receive a signal
    while (true)
    {
      // process one byte at a time to handle the protocol
      readSize = read(serverFIFO, &next_byte, 1);
      if (1 == readSize)
      {
        processProtocol(next_byte, &protocol);
        if (protocol.completeMessage)
        {
          // clear the flag to  acknowledge that we've processed the message
          protocol.completeMessage = false;

          // transfer to local buffer
          strcpy(mainMessage, protocol.message);
          mainMessageSize = protocol.messageIndex;
          mainClientID = protocol.sourceID;

          // relying on the protocol to ensure that we don't have buffer problems...
          // can be considered dangerous since the server *should* sanitze inputs

          sem_wait(&lock);
          strcpy(message, mainMessage);
          messageSize = mainMessageSize;
          clientID = mainClientID;
          sem_post(&lock);
          sem_wait(&worker_ready);        // wait for worker ready
          sem_post(&shared_buffer_ready); // signal buffer ready
          sem_wait(&worker_done_buffer);  // wait for worker done witht the buffer
        }
      }
    }
  }
  else
    fprintf(stderr, "unable to open server fifo\n");

  return 0;
}
