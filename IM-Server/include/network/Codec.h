#ifndef CODEC_H
#define CODEC_H
#include <string>

#include "network/Buffer.h"
#include "network/Protocol.h"

class Codec {
public:
    static bool ParseMessage(Buffer *buffer, uint32_t &out_msg_type,
                             std::string &out_msg_body);
    static std::string PackMessage(uint32_t msg_type,
                                   const std::string &msg_body);
};
#endif