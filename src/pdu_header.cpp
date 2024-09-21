#include <cfdp/pdu_header.hpp>

#include "cfdp/pdu_enums.hpp"
#include "internal_exceptions.hpp"
#include "utils.hpp"

namespace cfdp::pdu::header
{
// First header byte related bitmasks.
constexpr uint8_t VERSION_BITMASK           = 0b11100000;
constexpr uint8_t PDU_TYPE_BITMASK          = 0b00010000;
constexpr uint8_t DIRECTION_BITMASK         = 0b00001000;
constexpr uint8_t TRANSMISSION_MODE_BITMASK = 0b00000100;
constexpr uint8_t CRC_FLAG_BITMASK          = 0b00000010;
constexpr uint8_t LARGE_FILE_FLAG_BITMASK   = 0b00000001;

// Fourth header byte related bitmasks.
constexpr uint8_t SEGMENTATION_CONTROL_BITMASK  = 0b10000000;
constexpr uint8_t ENTITY_ID_LENGTH_BITMASK      = 0b01110000;
constexpr uint8_t SEGMENT_METADATA_FLAG_BITMASK = 0b00001000;
constexpr uint8_t TRANSACTION_LENGTH_BITMASK    = 0b00000111;

constexpr uint8_t MAX_THREE_BIT_NUM     = 0b111;
constexpr uint8_t MIN_HEADER_SIZE_BYTES = CONST_HEADER_SIZE_BYTES + 3;
} // namespace cfdp::pdu::header

cfdp::pdu::header::PduHeader::PduHeader(const std::span<uint8_t>& memoryView)
{
    // This is the minumum amount of bytes, the header has to contain.
    if (memoryView.size_bytes() < MIN_HEADER_SIZE_BYTES)
    {
        throw cfdp::exception::BytesDecodeException{
            "Passed buffer view is too small to contain a PDU header"};
    }
    auto firstByte = memoryView[0];

    version = (firstByte & VERSION_BITMASK) >> 5;

    if (version > MAX_THREE_BIT_NUM)
    {
        throw cfdp::exception::BytesDecodeException{"Passed version is larger than 7."};
    }

    pduType          = PduType((firstByte & PDU_TYPE_BITMASK) >> 4);
    direction        = Direction((firstByte & DIRECTION_BITMASK) >> 3);
    transmissionMode = TransmissionMode((firstByte & TRANSMISSION_MODE_BITMASK) >> 2);
    crcFlag          = CrcFlag((firstByte & CRC_FLAG_BITMASK) >> 1);
    largeFileFlag    = LargeFileFlag((firstByte & LARGE_FILE_FLAG_BITMASK) >> 0);

    const auto secondAndThirdByte = memoryView.subspan(1, 2);
    auto rawPduDataFieldLength    = utils::bigEndianBytesToInt<uint16_t>(secondAndThirdByte);

    pduDataFieldLength =
        rawPduDataFieldLength - 4 * (static_cast<uint8_t>(crcFlag == CrcFlag::CrcPresent));

    auto fourthByte = memoryView[3];

    segmentationControl = SegmentationControl((fourthByte & SEGMENTATION_CONTROL_BITMASK) >> 7);
    segmentMetadataFlag = SegmentMetadataFlag((fourthByte & SEGMENT_METADATA_FLAG_BITMASK) >> 3);

    // To fit in 3 bits, CFDP standard specifies that the size is
    // encoded as a size - 1.
    lengthOfEntityIDs   = ((fourthByte & ENTITY_ID_LENGTH_BITMASK) >> 4) + 1;
    lengthOfTransaction = ((fourthByte & TRANSACTION_LENGTH_BITMASK) >> 0) + 1;

    sourceEntityID =
        utils::bigEndianBytesToIntValidated<uint64_t>(memoryView, 4, lengthOfEntityIDs);
    transactionSequenceNumber = utils::bigEndianBytesToIntValidated<uint64_t>(
        memoryView, 4 + lengthOfEntityIDs, lengthOfTransaction);
    destinationEntityID = utils::bigEndianBytesToIntValidated<uint64_t>(
        memoryView, 4 + lengthOfEntityIDs + lengthOfTransaction, lengthOfEntityIDs);
};

std::vector<uint8_t> cfdp::pdu::header::PduHeader::encodeToBytes() const
{
    auto headerSize    = getRawSize();
    auto encodedHeader = std::vector<uint8_t>{};

    encodedHeader.reserve(headerSize);

    uint16_t realPduDataFieldLength =
        pduDataFieldLength + 4 * (static_cast<uint8_t>(crcFlag == CrcFlag::CrcPresent));

    uint8_t firstByte =
        (version << 5) | (utils::toUnderlying(pduType) << 4) |
        (utils::toUnderlying(direction) << 3) | (utils::toUnderlying(transmissionMode) << 2) |
        (utils::toUnderlying(crcFlag) << 1) | (utils::toUnderlying(largeFileFlag) << 0);

    encodedHeader.push_back(firstByte);

    auto pduDataFieldLengthBytes =
        utils::intToBigEndianBytes(realPduDataFieldLength, sizeof(uint16_t));

    utils::concatenateVectorsInplace(pduDataFieldLengthBytes, encodedHeader);

    // To fit in 3 bits, CFDP standard specifies that the size is
    // encoded as a size - 1.
    uint8_t fourthByte =
        (utils::toUnderlying(segmentationControl) << 7) | ((lengthOfEntityIDs - 1) << 4) |
        (utils::toUnderlying(segmentMetadataFlag) << 3) | ((lengthOfTransaction - 1) << 0);

    encodedHeader.push_back(fourthByte);

    auto sourceEntityBytes = utils::intToBigEndianBytes(sourceEntityID, lengthOfEntityIDs);

    utils::concatenateVectorsInplace(sourceEntityBytes, encodedHeader);

    auto transactionBytes =
        utils::intToBigEndianBytes(transactionSequenceNumber, lengthOfTransaction);

    utils::concatenateVectorsInplace(transactionBytes, encodedHeader);

    auto destinationEntityBytes =
        utils::intToBigEndianBytes(destinationEntityID, lengthOfEntityIDs);

    utils::concatenateVectorsInplace(destinationEntityBytes, encodedHeader);

    return encodedHeader;
};
