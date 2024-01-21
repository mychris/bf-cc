#include "interp.h"
#include "heap.h"
#include "instr.h"

#include <cstdio>
#include <stdint.h>

void Interpreter::Run(Heap &heap, Instr *code) {
  while (code) {
    switch (code->OpCode()) {
    case OpCode::NOP: {
    } break;
    case OpCode::INCR_CELL: {
      heap.IncrementCell(code->Operand1());
    } break;
    case OpCode::DECR_CELL: {
      heap.DecrementCell(code->Operand1());
    } break;
    case OpCode::SET_CELL: {
      heap.SetCell((uint8_t)code->Operand1());
    } break;
    case OpCode::INCR_PTR: {
      heap.IncrementDataPointer(code->Operand1());
    } break;
    case OpCode::DECR_PTR: {
      heap.DecrementDataPointer(code->Operand1());
    } break;
    case OpCode::READ: {
      uint8_t input = (uint8_t)std::getchar();
      heap.SetCell(input);
    } break;
    case OpCode::WRITE: {
      uint8_t output = heap.GetCell();
      std::putchar((int)output);
    } break;
    case OpCode::JUMP_ZERO: {
      if (heap.GetCell() == 0) {
        code = (Instr *)code->Operand1();
      }
    } break;
    case OpCode::JUMP_NON_ZERO: {
      if (heap.GetCell() != 0) {
        code = (Instr *)code->Operand1();
      }
    } break;
    case OpCode::FIND_CELL_HIGH: {
      uint8_t val = (uint8_t)code->Operand1();
      while (heap.GetCell() != val) {
        heap.IncrementDataPointer(code->Operand2());
      }
    } break;
    case OpCode::FIND_CELL_LOW: {
      uint8_t val = (uint8_t)code->Operand1();
      while (heap.GetCell() != val) {
        heap.DecrementDataPointer(code->Operand2());
      }
    } break;
    }
    code = code->Next();
  }
}
