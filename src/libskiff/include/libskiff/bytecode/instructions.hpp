#ifndef LIBSKIFF_INSTRUCTIONS_HPP
#define LIBSKIFF_INSTRUCTIONS_HPP

#include <cstdint>

namespace libskiff {
namespace bytecode {
namespace instructions {

constexpr uint8_t INS_SIZE_BYTES = 8;
constexpr uint8_t NOP = 0x00;
constexpr uint8_t EXIT = 0x01;
constexpr uint8_t BLT = 0x02;
constexpr uint8_t BGT = 0x03;
constexpr uint8_t BEQ = 0x04;
constexpr uint8_t JMP = 0x05;
constexpr uint8_t CALL = 0x06;
constexpr uint8_t RET = 0x07;
constexpr uint8_t MOV = 0x08;
constexpr uint8_t ADD = 0x09;
constexpr uint8_t SUB = 0x0A;
constexpr uint8_t DIV = 0x0B;
constexpr uint8_t MUL = 0x0C;
constexpr uint8_t ADDF = 0x0D;
constexpr uint8_t SUBF = 0x0E;
constexpr uint8_t DIVF = 0x0F;
constexpr uint8_t MULF = 0x10;
constexpr uint8_t LSH = 0x11;
constexpr uint8_t RSH = 0x12;
constexpr uint8_t AND = 0x13;
constexpr uint8_t OR = 0x14;
constexpr uint8_t XOR = 0x15;
constexpr uint8_t NOT = 0x16;
constexpr uint8_t BLTF = 0x17;
constexpr uint8_t BGTF = 0x18;
constexpr uint8_t BEQF = 0x19;
constexpr uint8_t ASEQ = 0x1A;
constexpr uint8_t ASNE = 0x1B;
constexpr uint8_t PUSH_W = 0x1C;
constexpr uint8_t PUSH_DW = 0x1D;
constexpr uint8_t PUSH_QW = 0x1E;
constexpr uint8_t POP_W = 0x1F;
constexpr uint8_t POP_DW = 0x20;
constexpr uint8_t POP_QW = 0x21;

constexpr uint8_t ALLOC = 0x22;
constexpr uint8_t FREE = 0x23;
constexpr uint8_t SW = 0x24;
constexpr uint8_t SDW = 0x25;
constexpr uint8_t SQW = 0x26;
constexpr uint8_t LW = 0x27;
constexpr uint8_t LDW = 0x28;
constexpr uint8_t LQW = 0x29;

} // namespace instructions
} // namespace bytecode
} // namespace libskiff

#endif
