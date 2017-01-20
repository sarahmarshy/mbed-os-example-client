#ifndef READ_ONLY_INTERFACE_H
#define READ_ONLY_INTERFACE_H
#include "simpleclient.h"
#include "edimax.h"
#include <cstdarg>

class AirBoxResource {
public:
    AirBoxResource(const char* object_id, const char* resource_id, const char* resource_name, M2MResourceInstance::ResourceType type) 
    {
       
        resource_object = M2MInterfaceFactory::create_object(object_id);
        M2MObjectInstance* resource_inst = resource_object->create_object_instance();
        M2MResource* res = resource_inst->create_dynamic_resource(resource_id, resource_name, type, true);
        res->set_operation(M2MBase::GET_ALLOWED);
        strcpy(res_id, resource_id);
        update_value("%s","None");
    }

    
    ~AirBoxResource(){}
    
    M2MObject* get_object() {
        return resource_object;
    }

    void update_value(const char* format, ...){
        M2MObjectInstance* inst = resource_object->object_instance();
        MBED_ASSERT(inst != NULL);
        M2MResource* res = inst->resource(res_id);
        MBED_ASSERT(res != NULL);
        char* buffer;
        buffer = (char *)malloc(sizeof(char) * 256);
        MBED_ASSERT(buffer != NULL);
        va_list args;
        va_start(args, format);
        int size = vsnprintf(buffer, 256, format, args);
        va_end(args);
        res->set_value((uint8_t*)buffer, size);
        free(buffer);
    }
private:
    char res_id[5];
    M2MObject* resource_object;
};

#endif
