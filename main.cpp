/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "simpleclient.h"
#include <string>
#include <sstream>
#include <vector>
#include "mbed-trace/mbed_trace.h"
#include "mbedtls/entropy_poll.h"

#include "security.h"

#include "mbed.h"
#include "rtos.h"
#include "application-resources/ReadOnlyResource.h"
#include "MTSASInterface.h"
#include "edimax.h"
MTSASInterface cell(RADIO_TX, RADIO_RX);
AirBoxResource temp_obj("3303", "5700", "Temperature", M2MResourceInstance::FLOAT);
AirBoxResource humidity_obj("3304", "5700", "Humidity", M2MResourceInstance::FLOAT);
AirBoxResource air_obj("3305", "5700", "Air", M2MResourceInstance::STRING);
AirBoxResource gps_obj("4000", "5700", "GPS", M2MResourceInstance::STRING);
AirBoxResource alias_obj("4001", "5700", "alias", M2MResourceInstance::STRING);

Edimax edimax(D1, D0);
#ifndef MESH
// This is address to mbed Device Connector
#define MBED_SERVER_ADDRESS "coap://api.connector.mbed.com:5684"
#else
// This is address to mbed Device Connector
#define MBED_SERVER_ADDRESS "coaps://[2607:f0d0:2601:52::20]:5684"
#endif

EventQueue gps_queue;


RawSerial output(USBTX, USBRX);

// These are example resource values for the Device Object
struct MbedClientDevice device = {
    "Manufacturer_String",      // Manufacturer
    "Type_String",              // Type
    "ModelNumber_String",       // ModelNumber
    "SerialNumber_String"       // SerialNumber
};

// Instantiate the class which implements LWM2M Client API (from simpleclient.h)
MbedClient mbed_client(device);

// Network interaction must be performed outside of interrupt context
Semaphore updates(0);
volatile bool registered = false;
volatile bool clicked = false;
osThreadId mainThread;
void add_gps_event();

void handle_edimax_data(struct edimax_data data){
    temp_obj.update_value("%3.2f", data.temp);
    humidity_obj.update_value("%3.2f", data.humidity);
    air_obj.update_value("%lu,%lu,%lu", data.pm10, data.pm25, data.pm100);
}

void handle_gps(){
    printf("GPS event!");
    struct gps_data data;
    data = cell.get_gps_location();
    gps_obj.update_value("%s,%s", data.latitude, data.longitude);
    add_gps_event();
}

void add_gps_event(){
    gps_queue.call(handle_gps);
}

void unregister() {
    registered = false;
    updates.release();
}

void button_clicked() {
    clicked = true;
    updates.release();
}

// debug printf function
void trace_printer(const char* str) {
    printf("%s\r\n", str);
}



// Entry point to the program
int main() {

    unsigned int seed;
    size_t len;

#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT
    // Used to randomize source port
    mbedtls_hardware_poll(NULL, (unsigned char *) &seed, sizeof seed, &len);

#elif defined MBEDTLS_TEST_NULL_ENTROPY

#warning "mbedTLS security feature is disabled. Connection will not be secure !! Implement proper hardware entropy for your selected hardware."
    // Used to randomize source port
    mbedtls_null_entropy_poll( NULL,(unsigned char *) &seed, sizeof seed, &len);

#else

#error "This hardware does not have entropy, endpoint will not register to Connector.\
You need to enable NULL ENTROPY for your application, but if this configuration change is made then no security is offered by mbed TLS.\
Add MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES and MBEDTLS_TEST_NULL_ENTROPY in mbed_app.json macros to register your endpoint."

#endif

    srand(seed);
    // Keep track of the main thread
    mainThread = osThreadGetId();

    // Sets the console baud-rate
    output.baud(115200);

    output.printf("Starting mbed Client example...\r\n");

    mbed_trace_init();
    mbed_trace_print_function_set(trace_printer);
    NetworkInterface *network_interface = 0;
    int connect_success = -1;
    output.printf("Using Cell\r\n");
    static const char apn[] = "wap.cingular";
    connect_success = cell.connect(apn);
    network_interface = &cell;
    if(connect_success == 0) {
    output.printf("\n\rConnected to Network successfully\r\n");
    } else {
        output.printf("\n\rConnection to Network Failed %d! Exiting application....\r\n", connect_success);
        return 0;
    }
    const char *ip_addr = network_interface->get_ip_address();
    if (ip_addr) {
        output.printf("IP address %s\r\n", ip_addr);
    } else {
        output.printf("No IP address\r\n");
    }
    mbed_client.create_interface(MBED_SERVER_ADDRESS, network_interface);

    // Create Objects of varying types, see simpleclient.h for more details on implementation.
    M2MSecurity* register_object = mbed_client.create_register_object(); // server object specifying connector info
    M2MDevice*   device_object   = mbed_client.create_device_object();   // device resources object

    // Create list of Objects to register
    M2MObjectList object_list;

    // Add objects to list
    object_list.push_back(device_object);
    object_list.push_back(temp_obj.get_object());
    object_list.push_back(humidity_obj.get_object());
    object_list.push_back(air_obj.get_object());
    object_list.push_back(gps_obj.get_object());
    object_list.push_back(alias_obj.get_object());

    // Set endpoint registration object
    mbed_client.set_register_object(register_object);

    // Register with mbed Device Connector
    mbed_client.test_register(register_object, object_list);
    registered = true;
    bool sensor_monitor = false;
    bool gps_monitor = false;
    while (true) {
        updates.wait(25000);
        if (mbed_client.is_registered()){
            if (!sensor_monitor){
                //Calls handle_edimax data on serial RX event
                edimax.listen(handle_edimax_data);
                sensor_monitor = true;
            }
            if (!gps_monitor){
                add_gps_event();
                gps_queue.dispatch();
                gps_monitor = true;
            }

        }
    }

    mbed_client.test_unregister();
}
