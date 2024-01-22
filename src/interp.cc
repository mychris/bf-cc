#include "interp.h"

#include <stdint.h>

#include <cstdio>

#include "heap.h"
#include "instr.h"

void Interpreter::Run(Heap &heap, Instr *code) {
  while (code) {
    switch (code->OpCode()) {
    case Instr::Code::NOP: {
    } break;
    case Instr::Code::INCR_CELL: {
      heap.IncrementCell(code->Operand1());
    } break;
    case Instr::Code::DECR_CELL: {
      heap.DecrementCell(code->Operand1());
    } break;
    case Instr::Code::SET_CELL: {
      heap.SetCell((uint8_t) code->Operand1());
    } break;
    case Instr::Code::INCR_PTR: {
      heap.IncrementDataPointer(code->Operand1());
    } break;
    case Instr::Code::DECR_PTR: {
      heap.DecrementDataPointer(code->Operand1());
    } break;
    case Instr::Code::READ: {
      const uint8_t input = (uint8_t) std::getchar();
      heap.SetCell(input);
    } break;
    case Instr::Code::WRITE: {
      const uint8_t output = heap.GetCell();
      std::putchar((int) output);
    } break;
    case Instr::Code::JUMP_ZERO: {
      if (heap.GetCell() == 0) {
        code = (Instr *) code->Operand1();
      }
    } break;
    case Instr::Code::JUMP_NON_ZERO: {
      if (heap.GetCell() != 0) {
        code = (Instr *) code->Operand1();
      }
    } break;
    case Instr::Code::FIND_CELL_HIGH: {
      const uint8_t val = (uint8_t) code->Operand1();
      while (heap.GetCell() != val) {
        heap.IncrementDataPointer(code->Operand2());
      }
    } break;
    case Instr::Code::FIND_CELL_LOW: {
      const uint8_t val = (uint8_t) code->Operand1();
      while (heap.GetCell() != val) {
        heap.DecrementDataPointer(code->Operand2());
      }
    } break;
    }
    code = code->Next();
  }
}
