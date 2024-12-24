/**
* @file CANSerialBridge.cpp
* @brief Communication library for communicating binary packet structures between CAN FD devices.
* @author testusuke
* @date 2023/07/20
*/

#include "../CANSerialBridge.hpp"

/**
* @brief CANSerialBridge class constructor.
* @param[in] dev (ACAN2517FD class pointer) An argument that indicates a CAN FD device class object.
*/
CANSerialBridge::CANSerialBridge(ACAN2517FD *dev)
        : _dev(dev), _id_list() {}

/**
* @brief A function that registers the message frame to be used.
* @param[in] id (Message identification number) A number to identify the message.
* The number can be specified from 0x00 to 0xFFFFFFFF.
* However, you cannot add more messages than specified by CANSerialBridge::STRUCT_MAX_NUM.
* @param[in] frame (sb::MessageInterface class pointer) A data structure object used to send and receive data.
* @return int Whether the message frame was added successfully.
* @retval  0 : Success.
* @retval -1 : Failure. The argument frame specified was NULL.
* @retval -2 : Failure. The number of registered message frames has reached the upper limit.
*/
int CANSerialBridge::add_frame(CANSerialBridge::frame_id id, sb::MessageInterface *frame) {
    if (frame == NULL) {
        return -1;
    }

    //  check if structure has already registered.
    int order = _id_2_order(id);
    if (order >= 0) {
        //  update structure
        _str[order] = frame;
        return 0;
    }

    //  find available order and register them.
    for (int i = 0; i < STRUCT_MAX_NUM; i++) {
        if (_str[i] == NULL) {
            _str[i] = frame;
            _id_list[i] = id;
            return 0;
        }
    }

    //  failed to add message due to fail to find available order.
    return -2;
}

/**
* @brief A function to remove registered message frames.
* @param[in] id (Message identification number)
* The identifier of the message frame you want to remove.
* @return int Whether the remove was successful.
* @retval  0 : Success.
* @retval -1 : Unregistered message id.
*/
int CANSerialBridge::rm_frame(frame_id id) {
    //  find order by frame id
    int order = _id_2_order(id);

    //  check if structure not exists
    if (order < 0) {
        return -1;
    }

    //  swap elements which located after removed structure.
    for (int i = order; _str[i] != NULL; i++) {
        _str[i] = _str[i + 1];
        _id_list[i] = _id_list[i + 1];
    }

    return 0;
}

/**
* @brief Sends the message registered with the specified id.
* @param[in] id (Message identification number)ID of the message you want to send.
* @return int Number of written data(byte) or error code.
* @retval -1 : Unregistered message id.
* @retval -2 : Failed to send message.
*/
int CANSerialBridge::write(CANSerialBridge::frame_id id) {
    int order = _id_2_order(id);

    //  check if structure not exists
    if (order < 0) {
        return -1;
    }

    //  pack message with frame_id
    _str[order]->packing();

    acan2517fd::CANFDMessage canfdMessage;
    //  id
    canfdMessage.id = id;
    //  find optimal message length
    canfdMessage.len = find_optimal_size(_str[order]->size());
    //  copy data
    memcpy(&canfdMessage.data, _str[order]->ptr(), _str[order]->size());

    //  write message
    bool result = _dev->tryToSend(canfdMessage);

    //  error handling
    if (!result) {
        return -2;
    }

    return 0;
}

/**
* @brief
* Update the received data from the serial device.
* (Note that this function must be called every time in a processing loop to get the data.)
* @return int Data acquisition status.
* @retval UpdateFrameStatus::UPDATE_SUCCESS : Updated message.
* @retval UpdateFrameStatus::NO_MESSAGE : Message not received.
* @retval UpdateFrameStatus::UNREGISTERED_ID : The id of the received message is unregistered.
*/
int CANSerialBridge::update() {
    //  update frame
    return _update_frame();
}

/**
* @brief Returns the array index of the frame stored in the array.(private)
* @param[in] id Message identification number.
* @return int Index of element or error.
* @retval -1 : Its id is not included in the array.
*/
int CANSerialBridge::_id_2_order(frame_id id) {
    for (int i = 0; _str[i] != NULL; i++) {
        if (id == _id_list[i]) {
            return i;
        }
    }
    return -1;
}

/**
* @brief Update the message from the packet data obtained from the CAN FD device.
* @return int The status of the update operation.
* @retval UpdateFrameStatus::UPDATE_SUCCESS : Updated message.
* @retval UpdateFrameStatus::NO_MESSAGE : Message not received.
* @retval UpdateFrameStatus::UNREGISTERED_ID : The id of the received message is unregistered.
*/
int CANSerialBridge::_update_frame() {
    //  Update one frame by one call for improving data integrity
    if (_dev->available()) {
        acan2517fd::CANFDMessage canfdMessage;

        //  receive message
        bool is_received = _dev->receive(canfdMessage);

        //  failed to receive
        if (!is_received) {
            return UpdateFrameStatus::NO_MESSAGE;
        }

        //  get order by frame_id
        int order = _id_2_order(canfdMessage.id);

        if (order < 0) {
            //  failed to find message
            return UpdateFrameStatus::UNREGISTERED_ID;
        }

        //  insert data to message
        memcpy(_str[order]->ptr(), &canfdMessage.data, _str[order]->size());

        //  unpack message
        _str[order]->unpacking();

        return UpdateFrameStatus::UPDATE_SUCCESS;
    }
    return UpdateFrameStatus::NO_MESSAGE;
}

/**
* @brief Returns the optimal packet size for sending.
* @param[in] size (uint8_t) The size of the data to be sent.
* @return int The optimal packet size.
*/
int CANSerialBridge::find_optimal_size(uint8_t size) {
    uint8_t optimal_size = 8;

    if ((size > 8) && (size < 12)) {
        optimal_size = 12;
    } else if ((size > 12) && (size < 16)) {
        optimal_size = 16;
    } else if ((size > 16) && (size < 20)) {
        optimal_size = 20;
    } else if ((size > 20) && (size < 24)) {
        optimal_size = 24;
    } else if ((size > 24) && (size < 32)) {
        optimal_size = 32;
    } else if ((size > 32) && (size < 48)) {
        optimal_size = 48;
    } else if ((size > 48) && (size < 64)) {
        optimal_size = 64;
    }

    return optimal_size;
}
