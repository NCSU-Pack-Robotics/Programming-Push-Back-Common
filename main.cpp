#include "SerialHandler.hpp"

int main()
{
    SerialHandler handler(DeviceType::BRAIN);

    handler.handlers.emplace(PacketId::Hello, [](const uint8_t* data) {

    });
}
