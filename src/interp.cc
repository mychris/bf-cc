#include "interp.h"
#include "machine.h"
#include "instr.h"

void Interpreter::Run(Instr* code) {
  auto machine = MachineState::Create();
  while (code) {
    switch (code->OpCode()) {
    case OpCode::NOP: {
      code =  code->Next();
    } break;
    case OpCode::INCR_CELL: {
      machine.IncrementCell(code->Operand1());
      code =  code->Next();
    } break;
    case OpCode::DECR_CELL: {
      machine.DecrementCell(code->Operand1());
      code =  code->Next();
    } break;
    case OpCode::SET_CELL: {
      machine.SetCell((u8)code->Operand1());
      code =  code->Next();
    } break;
    case OpCode::INCR_PTR: {
      machine.IncrementDataPointer(code->Operand1());
      code =  code->Next();
    } break;
    case OpCode::DECR_PTR: {
      machine.DecrementDataPointer(code->Operand1());
      code =  code->Next();
    } break;
    case OpCode::SET_PTR: {
      machine.SetDataPointer(code->Operand1());
      code =  code->Next();
    } break;
    case OpCode::READ: {
      u8 input = (u8)std::getchar();
      machine.SetCell(input);
      code =  code->Next();
    } break;
    case OpCode::WRITE: {
      u8 output = machine.GetCell();
      std::putchar((int)output);
      code =  code->Next();
    } break;
    case OpCode::JUMP_ZERO: {
      if (machine.GetCell() == 0) {
        code =  (Instr *)code->Operand1();
      }
      code =  code->Next();
    } break;
    case OpCode::JUMP_NON_ZERO: {
      if (machine.GetCell() != 0) {
        code =  (Instr *)code->Operand1();
      }
      code =  code->Next();
    } break;
    case OpCode::FIND_CELL_HIGH: {
      u8 val = (u8) code->Operand1();
      while (machine.GetCell() != val) {
        machine.IncrementDataPointer(code->Operand2());
      }
      code =  code->Next();
    } break;
    case OpCode::FIND_CELL_LOW: {
      u8 val = (u8) code->Operand1();
      while (machine.GetCell() != val) {
        machine.DecrementDataPointer(code->Operand2());
      }
      code =  code->Next();
    } break;
    }
  }
}
