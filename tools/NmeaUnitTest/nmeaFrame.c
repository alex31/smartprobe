#include "nmeaFrame.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "fnmatch.h"




static int32_t parseNMEA (const NmeaBinder *nbs, const void * const  userData,
			  char *nmeaFrame, ErrorCallback error_cb);
static bool verifyChecksum (NmeaStateMachine *sm);


void initStateMachine (NmeaStateMachine *sm)
{
  sm->receivedChksm1 = sm->receivedChksm2 = sm->currentChksm = sm->writeIndex = 0;
  sm->state = NS_WAIT_BEGIN;
}

static bool verifyChecksum (NmeaStateMachine *sm)
{
  const uint8_t chks1 = toupper ((int) sm->receivedChksm1);
  const uint8_t chks2 = toupper ((int) sm->receivedChksm2);

  uint8_t chksm = (chks1 - ((chks1 > '9') ? ('A'-10) : '0')) * 16;
  chksm += chks2 - ((chks2 > '9') ? ('A'- 10) : '0');

  if (chksm != sm->currentChksm) {
    printf ("DBG> received %c%c[0x%x] ; calculated 0x%x",
	    sm->receivedChksm1, sm->receivedChksm2, chksm, 
	    sm->currentChksm);
  }
  return (chksm == sm->currentChksm);
}




void feedNmea (const NmeaBinder *nbs, NmeaStateMachine *sm, 
	       const void * const  userData, char c, ErrorCallback error_cb)
{
  (void) nbs;
  

  switch (sm->state) {
  case NS_WAIT_BEGIN :
    if (c == '$') {
      initStateMachine (sm);
      sm->state = NS_WAIT_CHKSUM;
      sm->buffer[sm->writeIndex++] = c;
    }
    break;

  case NS_WAIT_CHKSUM :
    if (c == '*') {
      sm->buffer[sm->writeIndex] = 0; // null terminated string
      sm->state = NS_WAIT_CTRL1;
    } else {
      sm->buffer[sm->writeIndex++] = c;
      sm->currentChksm ^= c;
      if (sm->writeIndex >= sizeof (sm->buffer)) {
	// if message length is greater than max, there is an error 
	// we reinitialise state machine to wait for new message
	sm->buffer[sizeof(sm->buffer)-1] = 0; // null terminated string
	sm->state = NS_WAIT_BEGIN;
	(error_cb) (NMEA_OVERLENGTH_ERR, userData,sm->buffer);
      }
    }
    break;

  case NS_WAIT_CTRL1 :
    sm->receivedChksm1 = c;
    sm->state = NS_WAIT_CTRL2;
    break;

  case NS_WAIT_CTRL2 :
    sm->receivedChksm2 = c;
    //DebugTrace ("\r\n\ncomplete Buffer = %s*%c%c", sm->buffer, sm->receivedChksm1, sm->receivedChksm2);
    if (verifyChecksum (sm)) {
      parseNMEA (nbs, userData, sm->buffer, error_cb);
    } else {
      (error_cb) (NMEA_CHKSUM_ERR, userData, sm->buffer);
    }
    sm->state = NS_WAIT_BEGIN;
    break;
  }
}





// parse a correct, complete nmea message
static int32_t parseNMEA (const NmeaBinder *nbs, const void * const  userData, 
			  char *nmeaFrame, ErrorCallback error_cb) 
{
  int32_t sep = 0;
  char *field;
  char *fieldsPtr[MAX_NUM_OF_FIELD];
  NmeaParam nmeaParam[MAX_NUM_OF_FIELD];
  uint32_t i=0;

  while ((field = strsep (&nmeaFrame, ",")) != NULL)  {
    // case of PUBX where the first comma sepatared field is not a parameter
    // but the PUBX sub message index
    if (strcmp(field, "$PUBX") == 0) {
      strsep (&nmeaFrame, ","); // go to real first parameter
      field[5] = ','; // revert the \0 that have been placed by a comma
    }
    fieldsPtr[sep++] = field;
    if (sep >= MAX_NUM_OF_FIELD) {
      (error_cb) ( NMEA_OVERFIELD_ERR, userData, nmeaFrame);
      return sep;
    }
  }
   
  if (sep == 0) 
    return sep;

  while (nbs[i].fieldClass != NULL) {
    const NmeaBinder * const nb = &nbs[i++];
    //    if (strcmp(nb->fieldClass, fieldsPtr[0]) == 0) {
    if (fnmatch(nb->fieldClass, fieldsPtr[0], 0) == 0) {
      // recuperer les arguments
      uint32_t j=0;
      uint8_t writeFieldIndex=0;
      uint8_t fieldIndex;
      while ((fieldIndex = nb->field[j].fieldIndex) != 0) {
	NmeaParam *np = &(nmeaParam[writeFieldIndex++]);
	np->fieldDesc = &(nb->field[j++]);
	np->f_raw = fieldsPtr[fieldIndex];
	switch (np->fieldDesc->fieldType) {
	case NMEA_INT :
	  np->f_i = atoi (np->f_raw);
	  break;
	case NMEA_LONG :
	  np->f_l = atol (np->f_raw);
	  break;
	case NMEA_FLOAT :
	  np->f_f = (float) atof (np->f_raw);
	  break;
	case NMEA_DOUBLE :
	  np->f_d = atof (np->f_raw);
	  break;
	case NMEA_CHAR :
	  np->f_c = (np->f_raw)[0];
	  break;
	case NMEA_STRING :
	  np->f_s = np->f_raw;
	  break;
	}
      }
      // appeler la callback
      (nb->msgCb) (userData, writeFieldIndex, nmeaParam);
    }
  }
   
  return sep;
}

