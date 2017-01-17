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
 
#include "edimax.h" 

/** Edimax class 
*/

Edimax::Edimax(PinName tx, PinName rx, Callback<void(edimax_data)> func)
    : _serial(tx, rx, 1024), _parser(_serial)
{
    _func = func;
    _serial.baud(9600);
    _parser.setDelimiter("\n");
    _parser.setTimeout(3000);
    _parser.debugOn(1);
    //Attach a serial callback on serial RX
    _serial.attach(callback(this, &Edimax::rx_sem_release));
    listen_serial_data.start(callback(this, &Edimax::handle_serial_data));
}

Edimax::~Edimax(){
}

void Edimax::rx_sem_release(){
    rx_sem.release();
}

void Edimax::handle_serial_data()
{
    while(true){
        //Wait for serial rx event
        rx_sem.wait();
        struct edimax_data data;
        //Read the data
        data_recvd = _parser.recv("%.1f/%.1f%% PM10:%d PM25:%d PM100:%d", &data.temp, &data.humidity, &data.pm10, &data.pm25, &data.pm100);
        //Call our callback (passed in constructor)
        _func(data); 
   }
}


