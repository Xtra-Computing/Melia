#ifndef ERROR_HANDLING
#define ERROR_HANDLING

#include <CL/cl.h>
#include <string.h>

char *print_cl_errstring(cl_int err) {
    switch (err) {
        case CL_SUCCESS:                          return _strdup("Success!");
        case CL_DEVICE_NOT_FOUND:                 return _strdup("Device not found.");
        case CL_DEVICE_NOT_AVAILABLE:             return _strdup("Device not available");
        case CL_COMPILER_NOT_AVAILABLE:           return _strdup("Compiler not available");
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return _strdup("Memory object allocation failure");
        case CL_OUT_OF_RESOURCES:                 return _strdup("Out of resources");
        case CL_OUT_OF_HOST_MEMORY:               return _strdup("Out of host memory");
        case CL_PROFILING_INFO_NOT_AVAILABLE:     return _strdup("Profiling information not available");
        case CL_MEM_COPY_OVERLAP:                 return _strdup("Memory copy overlap");
        case CL_IMAGE_FORMAT_MISMATCH:            return _strdup("Image format mismatch");
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return _strdup("Image format not supported");
        case CL_BUILD_PROGRAM_FAILURE:            return _strdup("Program build failure");
        case CL_MAP_FAILURE:                      return _strdup("Map failure");
        case CL_INVALID_VALUE:                    return _strdup("Invalid value");
        case CL_INVALID_DEVICE_TYPE:              return _strdup("Invalid device type");
        case CL_INVALID_PLATFORM:                 return _strdup("Invalid platform");
        case CL_INVALID_DEVICE:                   return _strdup("Invalid device");
        case CL_INVALID_CONTEXT:                  return _strdup("Invalid context");
        case CL_INVALID_QUEUE_PROPERTIES:         return _strdup("Invalid queue properties");
        case CL_INVALID_COMMAND_QUEUE:            return _strdup("Invalid command queue");
        case CL_INVALID_HOST_PTR:                 return _strdup("Invalid host pointer");
        case CL_INVALID_MEM_OBJECT:               return _strdup("Invalid memory object");
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return _strdup("Invalid image format descriptor");
        case CL_INVALID_IMAGE_SIZE:               return _strdup("Invalid image size");
        case CL_INVALID_SAMPLER:                  return _strdup("Invalid sampler");
        case CL_INVALID_BINARY:                   return _strdup("Invalid binary");
        case CL_INVALID_BUILD_OPTIONS:            return _strdup("Invalid build options");
        case CL_INVALID_PROGRAM:                  return _strdup("Invalid program");
        case CL_INVALID_PROGRAM_EXECUTABLE:       return _strdup("Invalid program executable");
        case CL_INVALID_KERNEL_NAME:              return _strdup("Invalid kernel name");
        case CL_INVALID_KERNEL_DEFINITION:        return _strdup("Invalid kernel definition");
        case CL_INVALID_KERNEL:                   return _strdup("Invalid kernel");
        case CL_INVALID_ARG_INDEX:                return _strdup("Invalid argument index");
        case CL_INVALID_ARG_VALUE:                return _strdup("Invalid argument value");
        case CL_INVALID_ARG_SIZE:                 return _strdup("Invalid argument size");
        case CL_INVALID_KERNEL_ARGS:              return _strdup("Invalid kernel arguments");
        case CL_INVALID_WORK_DIMENSION:           return _strdup("Invalid work dimension");
        case CL_INVALID_WORK_GROUP_SIZE:          return _strdup("Invalid work group size");
        case CL_INVALID_WORK_ITEM_SIZE:           return _strdup("Invalid work item size");
        case CL_INVALID_GLOBAL_OFFSET:            return _strdup("Invalid global offset");
        case CL_INVALID_EVENT_WAIT_LIST:          return _strdup("Invalid event wait list");
        case CL_INVALID_EVENT:                    return _strdup("Invalid event");
        case CL_INVALID_OPERATION:                return _strdup("Invalid operation");
        case CL_INVALID_GL_OBJECT:                return _strdup("Invalid OpenGL object");
        case CL_INVALID_BUFFER_SIZE:              return _strdup("Invalid buffer size");
        case CL_INVALID_MIP_LEVEL:                return _strdup("Invalid mip-map level");
        default:                                  return _strdup("Unknown");
    }
}
#endif