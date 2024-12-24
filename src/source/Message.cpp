/**
* @file Message.cpp
* @brief The data format handled by SerialBridge is defined here.
* @author Taiyou Komazawa
* @date 2021/8/18
*/
#include "../Message.hpp"

/**
* @brief Returns a pointer to the current packet.
* @return uint8_t* Pointer indicating the packet array.
*/
uint8_t *sb::MessageInterface::ptr() {
    return _all_packet;
}

/**
* @brief Stores the data structure in a packet.
* This function is used internally by SerialBridge when sending.
* @param[in] id Message ID assigned when sending.
* @return None
*/
void sb::MessageInterface::packing() {
    //  copy message data
    memcpy(_all_packet, _data_ptr(), size());
}

/**
* @brief Extract the data structure from the packet.
* This function is used internally by SerialBridge on reception.
* @return None
*/
void sb::MessageInterface::unpacking() {
    memcpy(_data_ptr(), _all_packet, size());
    _unpacked = true;
}
