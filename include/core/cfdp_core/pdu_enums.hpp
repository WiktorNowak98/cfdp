#pragma once

#include <cstdint>

namespace cfdp::pdu::header
{
enum class PduType : uint8_t
{
    FileDirective = 0b0,
    FileData      = 0b1,
};

enum class Direction : uint8_t
{
    TowardsReceiver = 0b0,
    TowardsSender   = 0b1,
};

enum class TransmissionMode : uint8_t
{
    Acknowledged   = 0b0,
    Unacknowledged = 0b1,
};

enum class CrcFlag : uint8_t
{
    CrcNotPresent = 0b0,
    CrcPresent    = 0b1,
};

enum class LargeFileFlag : uint8_t
{
    SmallFile = 0b0,
    LargeFile = 0b1,
};

enum class SegmentationControl : uint8_t
{
    BoundariesNotPreserved = 0b0,
    BoundariesPreserved    = 0b1,
};

enum class SegmentMetadataFlag : uint8_t
{
    NotPresent = 0b0,
    Present    = 0b1,
};
} // namespace cfdp::pdu::header

namespace cfdp::pdu::directive
{

enum class Directive : uint8_t
{
    Eof       = 0x4,
    Finished  = 0x5,
    Ack       = 0x6,
    Metadata  = 0x7,
    Nak       = 0x8,
    Promt     = 0x9,
    KeepAlive = 0xC,

};

enum class Condition : uint8_t
{
    NoError                    = 0b0000,
    PositiveAckLimitReached    = 0b0001,
    KeepAliveLimitReached      = 0b0010,
    InvalidTransmissionReached = 0b0011,
    FilestoreRejection         = 0b0100,
    FileChecksumFailure        = 0b0101,
    FileSizeError              = 0b0110,
    NakLimitReached            = 0b0111,
    InactivityDetected         = 0b1000,
    InvalidFileStructure       = 0b1001,
    CheckLimitReached          = 0b1010,
    UnsupportedChecksumType    = 0b1011,
    SuspendRequestReceived     = 0b1110,
    CancelRequestReceived      = 0b1111,
};

} // namespace cfdp::pdu::directive
