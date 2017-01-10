#include "simpleclient.h"
#include "edimax.h"
#ifndef READ_ONLY_INTERFACE_H
#define READ_ONLY_INTERFACE_H

class AirBoxResource {
public:
    AirBoxResource() 
    {
       
        strcpy(edimax_sensors, "t:-1, h:-1, p10:-1, p25:-1, p100:-1");
        resource_object = M2MInterfaceFactory::create_object("3303");
        M2MObjectInstance* resource_inst = resource_object->create_object_instance();
        M2MResource* res = resource_inst->create_dynamic_resource("5700", "AirBoxData", M2MResourceInstance::STRING, true);
        res->set_operation(M2MBase::GET_ALLOWED);
        update_res_value();
    }

    
    ~AirBoxResource(){}
    
    M2MObject* get_object() {
        return resource_object;
    }

    void update_sensors(struct edimax_data data){
        sprintf(edimax_sensors, "t:%f, h:%f, p10:%lu, p25:%lu, p100:%lu", data.temp, data.humidity, data.pm10, data.pm25, data.pm100);           
        update_res_value();
    }
    
    void update_gps(char* lat, char* longi){
        strcpy(latitude, lat);
        strcpy(longitude, longi);
        update_res_value();
    }
    
    void update_res_value(){
        M2MObjectInstance* inst = resource_object->object_instance();
        M2MResource* res = inst->resource("5700");
        char buffer[115];
        int size = sprintf(buffer, "%s, lat:%s, lon:%s", edimax_sensors, latitude, longitude);
        res->set_value((uint8_t*)buffer, size);
    }
private:
    M2MObject* resource_object;
    char latitude[25];
    char longitude[25];
    char edimax_sensors[50];
};

#endif
