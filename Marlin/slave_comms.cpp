
#include "Marlin.h"
#include "slave_comms.h"

#ifdef REPRAPPRO_MULTIMATERIALS

char slaveXmitBuffer[SLAVE_BUF];
char slaveRcvBuffer[SLAVE_BUF];
boolean setDir[EXTRUDERS];
boolean firstTalk;
boolean inSlaveMessage;
char slaveError;
boolean driveOn[EXTRUDERS];
unsigned long timeout;
unsigned long slaveDelay;
long precision[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};

void slaveErrorReport()
{
  if(!slaveError)
    return;
    
  SERIAL_PROTOCOL("Slave error(s); comms errors will also cause heater errors: ");
  if(slaveError & SLAVE_HEAT1)
    SERIAL_PROTOCOL("Dud heater 1, ");
  if(slaveError & SLAVE_HEAT2)
    SERIAL_PROTOCOL("Dud heater 2, ");
  if(slaveError & SLAVE_TIMEOUT)
    SERIAL_PROTOCOL("Comms timeout, ");
  if(slaveError & SLAVE_OVERFLOW)  
    SERIAL_PROTOCOL("Comms buffer overflow, ");
  SERIAL_PROTOCOLLN("");
}

void setup_slave()
{
	MYSERIAL1.begin(SLAVE_BAUD);
	SET_OUTPUT(SLAVE_CLOCK);
        slaveError = 0;
        firstTalk = true;
        inSlaveMessage = false;
        for(int i = 0; i < EXTRUDERS; i++)
        {
          setDir[i] = true;
          driveOn[i] = false;
        }
  //get slave extruder temps until readings are sensible
    SERIAL_PROTOCOLLNPGM("Slave init. Please wait ... ");
    for(uint8_t i=0;i<20;i++)
    {
        slaveError &= ~SLAVE_HEAT1;
        slaveError &= ~SLAVE_HEAT2;
        SERIAL_PROTOCOLPGM(".");
/*        if(slaveDegHotend(1) > -1)
        {
            if(slaveDegHotend(2) > -1)
            {
                SERIAL_PROTOCOLLNPGM("Slave ready");
                return;
            }
            slaveError |= SLAVE_HEAT2;
        }
        slaveError |= SLAVE_HEAT1;*/
        

        if(slaveDegHotend(1) <= -1)
            slaveError |= SLAVE_HEAT1;
            
        if(slaveDegHotend(2) <= -1)
          slaveError |= SLAVE_HEAT2;
        
        if(!((slaveError & SLAVE_HEAT1) || (slaveError & SLAVE_HEAT2)))
        {
          SERIAL_PROTOCOLLNPGM("Slave ready");
          return;
        }
        delay(1000);
    }
    slaveErrorReport();
    return;
}

//
// Produce a formatted string in a buffer corresponding to the value provided.
// If the 'width' parameter is non-zero, the value will be padded with leading
// zeroes to achieve the specified width.  The number of characters added to
// the buffer (not including the null termination) is returned.
//
unsigned
fmtUnsigned(unsigned long val, char *buf, unsigned bufLen, byte width)
{
  if (!buf || !bufLen)
    return(0);

  // produce the digit string (backwards in the digit buffer)
  char dbuf[10];
  unsigned idx = 0;
  while (idx < sizeof(dbuf))
  {
    dbuf[idx++] = (val % 10) + '0';
    if ((val /= 10) == 0)
      break;
  }

  // copy the optional leading zeroes and digits to the target buffer
  unsigned len = 0;
  byte padding = (width > idx) ? width - idx : 0;
  char c = '0';
  while ((--bufLen > 0) && (idx || padding))
  {
    if (padding)
      padding--;
    else
      c = dbuf[--idx];
    *buf++ = c;
    len++;
  }

  // add the null termination
  *buf = '\0';
  return(len);
}

//
// Format a floating point value with number of decimal places.
// The 'precision' parameter is a number from 0 to 6 indicating the desired decimal places.
// The 'buf' parameter points to a buffer to receive the formatted string.  This must be
// sufficiently large to contain the resulting string.  The buffer's length may be
// optionally specified.  If it is given, the maximum length of the generated string
// will be one less than the specified value.
//
// example: ftoa(3.1415, 2, buf); // produces 3.14 (two decimal places)
//
//void fmtDouble(double val, byte precision, char *buf, unsigned bufLen)
//void fmtDouble(double val, byte precision, char *buf, unsigned bufLen)
char* ftoa(char *buf, float val, byte precision)
{
  char* result = buf;
  unsigned bufLen = SLAVE_BUF - 5;
  if (!buf)
    return buf;

  // limit the precision to the maximum allowed value
  const byte maxPrecision = 6;
  if (precision > maxPrecision)
    precision = maxPrecision;

  if (--bufLen > 0)
  {
    // check for a negative value
    if (val < 0.0)
    {
      val = -val;
      *buf = '-';
      bufLen--;
    }

    // compute the rounding factor and fractional multiplier
    float roundingFactor = 0.5;
    unsigned long mult = 1;
    for (byte i = 0; i < precision; i++)
    {
      roundingFactor /= 10.0;
      mult *= 10;
    }

    if (bufLen > 0)
    {
      // apply the rounding factor
      val += roundingFactor;

      // add the integral portion to the buffer
      unsigned len = fmtUnsigned((unsigned long)val, buf, bufLen, 0);
      buf += len;
      bufLen -= len;
    }

    // handle the fractional portion
    if ((precision > 0) && (bufLen > 0))
    {
      *buf++ = '.';
      if (--bufLen > 0)
        buf += fmtUnsigned((unsigned long)((val - (unsigned long)val) * mult), buf, bufLen, precision);
    }
  }

  // null-terminate the string
  *buf = '\0';
  
  return result;
}

/*
char* ftoa(char *a, float f, int prec)
{
   char *ret = a;
   long whole = (long)f;
   if(!whole && f < 0.0)
     *a++ = '-';
   itoa(whole, a, 10);
   while (*a != '\0') a++;
   *a++ = '.';
   f = abs(f);
   whole = labs(whole);
   f = f - (float)whole;
   //long decimal;// = ((long)f * precision[prec]);
   int i = 0;
   char digit;
   while(i < prec)
   {
      f *= 10.0;
      digit = (char)f;
      *a++ = '0' + digit;
      //itoa(whole, a, 10);
      //a++;
      i++;
   }
   *a = 0;
   //itoa(decimal, a, 10);
   return ret;
}
*/

#endif
