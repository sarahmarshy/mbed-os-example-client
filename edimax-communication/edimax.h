/* Communicate serially with edimax box
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef EDIMAX_H
#define EDIMAX_H

#include "mbed.h"
//class ATParser;
#include "ATParser.h" 

/** Edimax class 
*/
struct edimax_data{
    float temp;
    float humidity;
    uint32_t pm10;
    uint32_t pm25;
    uint32_t pm100;
};

class Edimax
{
    public:
        Edimax(PinName tx, PinName rx, Callback<void(edimax_data)> func);
        ~Edimax();
    private:
        
        Callback<void(edimax_data)> _func;
        Semaphore rx_sem;
        bool data_recvd;
        Thread listen_serial_data;
        BufferedSerial _serial;
        ATParser _parser;
        void rx_sem_release();
        void handle_serial_data();
        edimax_data *_data;
};

#endif 
