/* sys.c */
/*****************************************************************************
COPYRIGHT NOTICE

PTP Notice for PTPd Software

The following copyright notice applies to all files which compose the original
PTPd Software. This notice applies as if the text was explicitly included each
file.

Copyright (c) 2006 Aidan Williams
Copyright (c) 2005-2007 Kendall Correll

Permission is hereby granted to use, copy, modify, and distribute this software
for any purpose and without fee, provided that this notice appears in all
copies. The authors make no representations about the suitability of this
software for any purpose. This software is provided "as is" without express or
implied warranty.

National Semiconductor Notice for Modified Software

The following copyright notice applies Modifications of the PTPd Software 
developed by National Semiconductor Corporation, and distributed by 
National Semiconductor as a Modified Version of the PTPd Software.  

Copyright (c) 2008 National Semiconductor

The associated Modified Software is distributed by National Semiconductor under 
the above PTPd Notice and Permission, and under the following License, 
Restrictions, Disclaimers and Limitations:

LICENSE:  Permission is granted to copy, use, modify and/or distribute the 
Software in Source and/or Binary form, including as an FPGA or other hardware 
implementation of the Software, subject to the Restrictions, Disclaimers and 
Limitations.  NSC and/or its licensors retain ownership of all copyright, 
patent and other intellectual property rights in the Software, and COMPANY 
shall not remove or alter any copyright or other notices associated with the 
Software

RESTRICTIONS: The Software may be distributed only in connection the 
distribution of COMPANY�s Products, and only subject to the following 
additional Restrictions:  (a) NSC Components:  The Software may be used 
only in connection with Components that are incorporated into COMPANY's 
Products; (b) Sublicensing Source:  The Software may be sublicensed in 
Source form, without any right to grant further downstream sublicenses, 
solely in accordance with this Agreement including these Restrictions;  
(c) Confidentiality.  The Source form of the Software is confidential, 
and unauthorized use or disclosure is prohibited; and (d) Export Compliance.  
The Software is subject to United States export control laws and regulations, 
and any product, software or technical data acquired from NSC, or any direct 
product thereof, shall not be, directly or indirectly, exported, re-exported, 
or released to any destination without first obtaining any export license or 
other approval required by the U.S. government, or other applicable non-U.S. 
governments.  This provision shall survive any termination of this Agreement.

DISCLAIMERS:   The Software is provided "AS IS" without warranty of any kind, 
including any warranty as to the design or manufacture of COMPANY Products 
incorporating Components.  NSC and its licensors expressly disclaim all 
warranties, expressed, implied or otherwise, including without limitation, 
warranties of merchantability, fitness for a particular purpose and 
non-infringement of intellectual property rights.  
Any COMPANY Product incorporating any Software (or any FPGA or other hardware 
implementation) and associated Components should not be released to production 
without full test, verification, and qualification, including verification of 
the selection, configuration and performance of any Software (or any FPGA or 
other hardware implementation) and/or Components, and including verification 
that the product design meets functional, performance, reliability and any 
applicable export or other regulatory requirements.

LIMITATIONS ON LIABILITY.  NSC or its licensors shall not be liable for any 
direct or indirect or punitive damages of any kind, including but not limited 
to any special, consequential or incidental damages, including any costs of 
labor, delay, requalification, or replacement, and any lost business 
opportunity, profits, or goodwill, whether arising out of the use or 
inability to use the Software (or the use or inability to use any COMPANY 
Product incorporating the Software), even if NSC is advised of the 
possibility of such damages.  In no event shall NSC's aggregate liability from 
any obligation arising out of or in connection with the license or use of the 
Software, under any theory of liability including but not limited to contract, 
tort or promissory fraud, exceed the consideration received by NSC, if any, 
for the license to the Software.  The foregoing limitation shall not apply to 
damages resulting directly from NSC's willful and wanton conduct.  To the 
maximum extent permitted under law, the limitations in this paragraph shall 
apply even if this limited remedy is found to have failed of its essential 
purpose.

******************************************************************************/


#include "../ptpd.h"
extern RunTimeOpts rtOpts;

void displayStats(const PtpClock *ptpClock)
{
}

boolean nanoSleep(const TimeInternal *t)
{
#if defined(linux)

  struct timespec ts, tr;
  
  ts.tv_sec = t->seconds;
  ts.tv_nsec = t->nanoseconds;
  
  if(nanosleep(&ts, &tr) < 0)
  {
    t->seconds = tr.tv_sec;
    t->nanoseconds = tr.tv_nsec;
    return FALSE;
  }
  
  return TRUE;
  
#else
  //TODO: implement
  return 0;
#endif
}

void getTime(TimeInternal *time)
{
#if defined( linux)
  struct timeval tv;
  
  gettimeofday(&tv, 0);
  time->seconds = tv.tv_sec;
  time->nanoseconds = tv.tv_usec*1000;
#endif // linux

    PTPClockReadCurrent( rtOpts.epl_port_handle, &time->seconds, &time->nanoseconds);
    return;
}

void setTime(const TimeInternal *time)
{
#if defined( linux)
  struct timeval tv;
  
  tv.tv_sec = time->seconds;
  tv.tv_usec = time->nanoseconds/1000;
  settimeofday(&tv, 0);
  
  NOTIFY("resetting system clock to %ds %dns\n", time->seconds, time->nanoseconds);
#endif // linux

  PTPClockSet( rtOpts.epl_port_handle, abs(time->seconds), (time->nanoseconds));
}

int seeded = 0;

uint32_t getRand(uint32_t rand_max)
{
    if ( !seeded)
    {
        srand((unsigned int)12);
        seeded = 1;
    }

    return rand() % rand_max;
}

boolean adjFreq(int32_t adj)
{
  if(adj > ADJ_FREQ_MAX)
    adj = ADJ_FREQ_MAX;
  else if(adj < -ADJ_FREQ_MAX)
    adj = -ADJ_FREQ_MAX;

  int sign = (adj >= 0) ? false:true;
  //TODO: figure this out
  PTPClockSetRateAdjustment(rtOpts.epl_port_handle, abs(adj), false,  sign);
  return true;
}

