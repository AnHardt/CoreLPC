/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ANALOGOUT_H
#define ANALOGOUT_H

#include "Core.h"

// Initialise this module
void AnalogOutInit();

/*
 * \brief Writes an analog value (PWM wave) to a pin.
 *
 * \param ulPin
 * \param ulValue, will be constrained to be within 0.0 to 1.0 within this function
 * \param freq (optional)
 */


constexpr uint8_t MaxNumberSoftwarePWMPins = 8;
constexpr size_t MaxTimerEntries = 3; //MR0 for the Frequency and MR1-3 for the Timer PWM

constexpr uint16_t HardwarePWMFrequency = 50; //50Hz for Servos

void AnalogOut(Pin pin, float ulValue, uint16_t freq = 1000);
bool IsPwmCapable(Pin pin);
bool ConfigurePinForPWM(Pin pin, bool outputHigh);
void ReleasePWMPin(Pin pin);


bool IsServoCapable(Pin pin);
bool ConfigurePinForServo(Pin pin, bool outputHigh);
void ReleaseServoPin(Pin pin);
bool AnalogWriteServo(float ulValue, uint16_t freq, Pin pin);

bool AnalogWriteSoftwarePWM(float ulValue, uint16_t freq, Pin pin);
bool CanDoSoftwarePWM(Pin pin);
bool ConfigurePinForSoftwarePWM(Pin pin);
void ReleaseSoftwarePWMPin(Pin pin);

bool AnalogWriteHWPWM(const PinDescription& pinDesc, float ulValue, uint16_t freq, Pin pin);
bool CanDoHWPWM(Pin pin);
bool ConfigurePinForHWPWM(Pin pin, bool outputHigh);
bool ReleaseHWPWMPin(Pin pin);



extern Pin UsedHardwarePWMChannel[NumPwmChannels];
extern Pin Timer2PWMPins[MaxTimerEntries];
class SoftwarePWM; //fwd decl
extern SoftwarePWM* softwarePWMEntries[MaxNumberSoftwarePWMPins];

#endif // ANALOGOUT_H
