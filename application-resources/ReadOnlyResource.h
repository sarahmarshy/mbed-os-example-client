#ifndef READ_ONLY_INTERFACE_H
#define READ_ONLY_INTERFACE_H
#include "simpleclient.h"
#include "edimax.h"
#include <cstdarg>

class AirBoxResource {
public:
    AirBoxResource(char* object_id, char* resource_id, char* resource_name, M2MResourceInstance::ResourceType type) 
    {
       
        resource_object = M2MInterfaceFactory::create_object(object_id);
        M2MObjectInstance* resource_inst = resource_object->create_object_instance();
        M2MResource* res = resource_inst->create_dynamic_resource(resource_id, resource_name, type, true);
        res->set_operation(M2MBase::GET_ALLOWED);
        strcpy(obj_id, object_id);
        strcpy(res_id, resource_id);
        update_value("%s","None");
    }

    
    ~AirBoxResource(){}
    
    M2MObject* get_object() {
        return resource_object;
    }

    void update_value(const char* format, ...){
        M2MObjectInstance* inst = resource_object->object_instance();
        M2MResource* res = inst->resource(res_id);
        char* buffer;
        buffer = (char *)malloc(sizeof(char) * 256);
        va_list args;
        va_start(args, format);
        int size = vsprintf(buffer, format, args);
        va_end(args);
        res->set_value((uint8_t*)buffer, size);
        free(buffer);
    }
private:
    char obj_id[5];
    char res_id[5];
    M2MObject* resource_object;
};

#endif
