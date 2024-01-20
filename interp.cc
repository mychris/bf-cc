#include "interp.h"
#include "machine.h"
#include "instr.h"


void Interpreter::Run(Instr* op) {
  auto machine = MachineState::Create();
  while (op) {
    op = op->Exec(machine);
  }
}
