/**
* @file Message.hpp
* @brief The data format handled by SerialBridge is defined here.
* @author Taiyou Komazawa
* @date 2021/8/11
*
* @details
* @par Example of data format declaration:
*       //Define the structure of the data.
*       typedef struct{
*           int32_t x;
*           int32_t y;
*           int32_t z;
*       } Vector3_t;
 *
*       //Put the data format in the message type.
*       //(Since the name is long, you can rename it with typedef as follows.)
*       typedef sb::Message<Vector3_t> Vector3;
*
* @par How to use:
*       Vector3 msg; //Declare a message.
*
*       //Register this message frame in the SerialBridge instance
*       // (assuming you declared it with the name obj this time).
*       //The msg_id passed to the argument together has the role of
*       // associating the data communicated with the msg declared earlier.
*       obj.add_frame(0, &msg); //msg_id = 0
*
*       //The data to be communicated conforms
*       // to the contents of the object member's data structure.
*       msg.data.x;
*       msg.data.y;
*       msg.data.z;
*/

#ifndef _SERIAL_BRIDGE_MESSAGE_HPP_
#define _SERIAL_BRIDGE_MESSAGE_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
* @namespace sb
* @brief Namespace to indicate SerialBridge messages.
*/
namespace sb
{

/**
* @brief sb::MessageInterface interface class.
* @details
* This is for abstracting various sb::Messages so that they can be handled
*  with a unified interface when used within SerialBridge.
*/
class MessageInterface
{
public:
    uint8_t *ptr();

    void packing();
    void unpacking();

    /**
    * @brief Returns the length of the current packet.
    * @return int Current packet array length.
    */
    virtual int size() = 0;

    /**
    * @brief Returns whether the message has been updated.
    *
    * @return true  Message has been updated.
    * @return false Message has not been updated since the last function call.
    */
    virtual bool was_updated() = 0;

protected:
    /**
    * @brief A function that returns a data structure.
    * @return *void Pointer to the data structure of the derived class.
    */
    virtual void *_data_ptr() = 0;

    uint8_t *_all_packet;
    volatile bool _unpacked;
};


/**
* @brief A class that stores the data processed by the SerialBridge class.
* @tparam DataStruct Specify the type of data structure you want to handle with sb::Message.
*/
template <class DataStruct>
class Message : public MessageInterface
{
public:
    /**
    * @brief Data is exchanged through this member variable.
    * You can specify any structure type here with template parameters.
    */
    DataStruct data;

    /**
    * @brief Message class constructor.
    */
    Message()
    : _error(false)
    {
        if (sizeof(DataStruct) > 64) {
            _error = true;
        }
        _all_packet = new uint8_t[sizeof(DataStruct)];
    }

    /**
    * @brief Message class destructor.
    */
    ~Message()
    {
        delete[] _all_packet;
    }

    /**
    * @brief Returns the size of the current packet.
    * @return int Current packet array length.
    */
    virtual int size()
    {
        return sizeof(DataStruct);
    }

    /**
    * @brief Returns whether the message has been updated.
    * @return bool Whether the message has been updated.
    */
    virtual bool was_updated()
    {
        bool tmp = _unpacked;
        _unpacked = false;
        return tmp;
    }

    /**
    * @brief Returns whether the message has an error.
    * @return bool Whether the message has an error.
    */
    virtual bool is_error()
    {
        return _error;
    }

private:
    bool _error;

    /**
    * @brief Returns a pointer to the data structure.
    * @return void* Pointer to the data structure.
    */
    virtual void *_data_ptr()
    {
        return &data;
    }
};
}

#endif //#ifndef _SERIAL_BRIDGE_MESSAGE_HPP_
