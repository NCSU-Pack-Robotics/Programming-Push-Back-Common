#include "SerialHandler.hpp"

SerialHandler::SerialHandler(DeviceType device_type) : device_type(device_type)
{
    if (device_type == DeviceType::PI)
    {
        // TODO: Open PI connection to /dev/ttyACM1
    }
}
