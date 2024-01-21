#include "interp.h"
#include "instr.h"
#include "machine.h"

#include <cstdio>
#include <stdint.h>

void Interpreter::Run(Instr *code) {
  auto machine = MachineState::Create();
  while (code) {
    switch (code->OpCode()) {
    case OpCode::NOP: {
    } break;
    case OpCode::INCR_CELL: {
      machine.IncrementCell(code->Operand1());
    } break;
    case OpCode::DECR_CELL: {
      machine.DecrementCell(code->Operand1());
    } break;
    case OpCode::SET_CELL: {
      machine.SetCell((uint8_t)code->Operand1());
    } break;
    case OpCode::INCR_PTR: {
      machine.IncrementDataPointer(code->Operand1());
    } break;
    case OpCode::DECR_PTR: {
      machine.DecrementDataPointer(code->Operand1());
    } break;
    case OpCode::READ: {
      uint8_t input = (uint8_t)std::getchar();
      machine.SetCell(input);
    } break;
    case OpCode::WRITE: {
      uint8_t output = machine.GetCell();
      std::putchar((int)output);
    } break;
    case OpCode::JUMP_ZERO: {
      if (machine.GetCell() == 0) {
        code = (Instr *)code->Operand1();
      }
    } break;
    case OpCode::JUMP_NON_ZERO: {
      if (machine.GetCell() != 0) {
        code = (Instr *)code->Operand1();
      }
    } break;
    case OpCode::FIND_CELL_HIGH: {
      uint8_t val = (uint8_t)code->Operand1();
      while (machine.GetCell() != val) {
        machine.IncrementDataPointer(code->Operand2());
      }
    } break;
    case OpCode::FIND_CELL_LOW: {
      uint8_t val = (uint8_t)code->Operand1();
      while (machine.GetCell() != val) {
        machine.DecrementDataPointer(code->Operand2());
      }
    } break;
    }
    code = code->Next();
  }
}
