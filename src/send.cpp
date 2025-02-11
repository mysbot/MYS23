// 01_send.ino

// Simple example of sending codes with a Radio Frequencies device.
// Sends code 4 times every 5 seconds.

/*
  Copyright 2021 Sébastien Millet

  `RF433send' is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  `RF433send' is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program. If not, see
  <https://www.gnu.org/licenses>.
*/

// Schematic:
//   RF433 TRANSMITTER data pin plugged on Arduino D4

#include "RF433send.h"
#include "send.h"

#define PIN_RFOUT  4

RFSender::RFSender(uint8_t pin) : pin(pin), tx_whatever(nullptr) {}

void RFSender::begin() {
    pinMode(pin, OUTPUT);   

    tx_whatever = rfsend_builder(
        RfSendEncoding::MANCHESTER,
        pin,
        RFSEND_DEFAULT_CONVENTION,
        4,
        nullptr,
        5500,
        0,
        0,
        0,
        1150,
        2330,
        0,
        0,
        0,
        5500,
        32
    );
}

void RFSender::sendLoop() {
    static int count = 0;
    byte n = tx_whatever->send(sizeof(data), data);
    Serial.print("Envoi effectué ");
    Serial.print(n);
    Serial.print(" fois\n");
    delay(5000);
}
