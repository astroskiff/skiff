#ifndef LIBSKIFF_INSTRUCTIONS_HPP
#define LIBSKIFF_INSTRUCTIONS_HPP

#include <cstdint>

namespace libskiff
{
namespace instructions
{

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

}
}

#endif
