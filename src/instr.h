// SPDX-License-Identifier: MIT License
#ifndef BF_CC_INSTR_H
#define BF_CC_INSTR_H 1

#include <cstdint>
#include <initializer_list>
#include <utility>

#include "error.h"

/**
 * Instructions.
 *
 * See the Operation class for the actual operation, Instruction is
 * only the OpCode of the Operation.
 *
 * Each operation can have up to two operands.  The operands for each
 * Instruction are (NULL means ignored and should be 0):
 *
 *   NOP             [NULL, NULL]          Do nothing
 *   INCR_CELL       [AMOUNT, PTR OFFSET]  Increment the cell at PTR OFFSET by AMOUNT
 *   DECR_CELL       [AMOUNT, PTR OFFSET]  Decrement the cell at PTR OFFSET by AMOUNT
 *   IMUL_CELL       [AMOUNT, PTR OFFSET]  Increment cell at PTR OFFSET by a multiple of the current cell
 *   DMUL_CELL       [AMOUNT, PTR OFFSET]  Decrement cell at PTR OFFSET by a multiple of the current cell
 *   SET_CELL        [VALUE, PTR OFFSET]   Set the cell at PTR OFFSET to VALUE
 *   INCR_PTR        [AMOUNT, NULL]        Increment the cell pointer by AMOUNT
 *   DECR_PTR        [AMOUNT, NULL]        Decrement the cell pointer by AMOUNT
 *   READ            [NULL, PTR OFFSET]    Read from STDIN into the cell at PTR OFFSET
 *   WRITE           [NULL, PTR OFFSET]    Write to STDOUT the value from cell at PTR OFFSET
 *   JZ              [ADDR, NULL]          Jump to the label at ADDR if the cell value is 0
 *   JNZ             [ADDR, NULL]          Jump to the label at ADDR if the cell value is not 0
 *   LABEL           [ADDR, NULL]          Destination for jumps, ADDR is the jump which jumps to this label
 *   FIND_CELL_LOW   [VALUE, MOVE AMOUNT]  Find cell with VALUE, move the cell pointer downwards by MOVE AMOUNT
 *   FIND_CELL_HIGH  [VALUE, MOVE AMOUNT]  Find cell with VALUE, move the cell pointer upwards by MOVE AMOUNT
 */
enum class Instruction : uint32_t {
  NOP = 1 << 0,
  INCR_CELL = 1 << 1,
  DECR_CELL = 1 << 2,
  IMUL_CELL = 1 << 3,
  DMUL_CELL = 1 << 4,
  SET_CELL = 1 << 5,
  INCR_PTR = 1 << 6,
  DECR_PTR = 1 << 7,
  // SET_PTR = 1 << 8,
  READ = 1 << 9,
  WRITE = 1 << 10,
  JZ = 1 << 11,
  JNZ = 1 << 12,
  LABEL = 1 << 13,
  FIND_CELL_LOW = 1 << 14,
  FIND_CELL_HIGH = 1 << 15,
};

enum class EOFMode {
  KEEP,
  ZERO,
  NEG_ONE,
};

class Operation final {
  friend class OperationStream;

public:
  typedef intptr_t operand_type;

private:
  struct M {
    Instruction code{Instruction::NOP};
    Operation *next{nullptr};
    Operation *prev{nullptr};
    intptr_t operands[2]{0, 0};
  } m;

  Operation(const Operation &) = delete;
  Operation &operator=(const Operation &) = delete;

  explicit Operation(M m) : m(std::move(m)) {
  }

  static Operation Create(enum Instruction code, intptr_t op1 = 0, intptr_t op2 = 0) {
    return Operation(M{
        .code = code,
        .next = nullptr,
        .prev = nullptr,
        .operands = {op1, op2},
    });
  }

  static Operation *Allocate(enum Instruction code, intptr_t op1 = 0, intptr_t op2 = 0) {
    Operation *instr = new (std::nothrow) Operation(M{
        .code = code,
        .next = nullptr,
        .prev = nullptr,
        .operands = {op1, op2},
    });
    if (!instr) {
      Error(Err::OutOfMemory());
    }
    return instr;
  }

public:
  Operation(Operation &&other) : m(std::exchange(other.m, {Instruction::NOP, nullptr, nullptr, {0, 0}})) {
  }

  Operation &operator=(Operation &&other) noexcept {
    std::swap(m, other.m);
    return *this;
  }

  ~Operation() = default;

  inline Instruction OpCode() const noexcept {
    return m.code;
  }

  inline bool Is(Instruction code) const noexcept {
    return m.code == code;
  }

  inline bool IsAny(std::initializer_list<Instruction> codes) const noexcept {
    for (const auto &c : codes) {
      if (c == m.code) {
        return true;
      }
    }
    return false;
  }

  inline void SetOpCode(enum Instruction cmd) {
    m.code = cmd;
  }

  inline bool IsJump() const {
    return m.code == Instruction::JZ || m.code == Instruction::JNZ;
  }

  inline intptr_t Operand1() const {
    return m.operands[0];
  }

  inline void SetOperand1(intptr_t val) {
    m.operands[0] = val;
  }

  inline intptr_t Operand2() const {
    return m.operands[1];
  }

  inline void SetOperand2(intptr_t val) {
    m.operands[1] = val;
  }

  void Dump() const;
};

class OperationStream final {
private:
  struct M {
    Operation *head;
    Operation *tail;
    std::size_t length;
  } m;

  OperationStream(const OperationStream &) = delete;
  OperationStream &operator=(const OperationStream &) = delete;

  explicit OperationStream(M m) : m(std::move(m)) {
  }

public:
  OperationStream(OperationStream &&other) noexcept : m(std::exchange(other.m, {})) {
  }

  OperationStream &operator=(OperationStream &&other) noexcept {
    std::swap(m, other.m);
    return *this;
  }

  static OperationStream Create() noexcept {
    return OperationStream(M{});
  }

  ~OperationStream() {
    Operation *op = m.head;
    while (op) {
      Operation *next = op->m.next;
      delete op;
      op = next;
    }
    m.head = nullptr;
    m.tail = nullptr;
    m.length = 0;
  }

  inline Operation *First() {
    return m.head;
  }

  inline Operation *Last() {
    return m.tail;
  }

  inline void Append(Instruction code, intptr_t op1 = 0, intptr_t op2 = 0) {
    Operation *instr = Operation::Allocate(code, op1, op2);
    ++m.length;
    if (!m.head) {
      m.head = instr;
      m.tail = instr;
      instr->m.next = nullptr;
      instr->m.prev = nullptr;
    } else {
      m.tail->m.next = instr;
      instr->m.prev = m.tail;
      m.tail = instr;
    }
  }

  inline void Prepend(Instruction code, intptr_t op1 = 0, intptr_t op2 = 0) {
    Operation *instr = Operation::Allocate(code, op1, op2);
    ++m.length;
    if (!m.head) {
      m.head = instr;
      m.tail = instr;
      instr->m.next = nullptr;
      instr->m.prev = nullptr;
    } else {
      instr->m.next = m.head;
      m.head->m.prev = instr;
      m.head = instr;
    }
  }

  inline void InsertBefore(Operation *instr, Instruction code, intptr_t op1 = 0, intptr_t op2 = 0) {
    if (nullptr == instr) {
      Append(code, op1, op2);
    } else if (nullptr == instr->m.prev) {
      Prepend(code, op1, op2);
    } else {
      Operation *prev = instr->m.prev;
      Operation *next = instr;
      Operation *new_instr = Operation::Allocate(code, op1, op2);
      prev->m.next = new_instr;
      next->m.prev = new_instr;
      new_instr->m.next = next;
      new_instr->m.prev = prev;
    }
  }

  inline void Unlink(Operation &instr) {
    --m.length;
    if (m.head == &instr) {
      if (m.tail == &instr) {
        m.head = nullptr;
        m.tail = nullptr;
      } else {
        m.head = instr.m.next;
        m.head->m.prev = nullptr;
      }
    } else if (m.tail == &instr) {
      m.tail = instr.m.prev;
      m.tail->m.next = nullptr;
    } else {
      instr.m.prev->m.next = instr.m.next;
      instr.m.next->m.prev = instr.m.prev;
    }
    instr.m.next = nullptr;
    instr.m.prev = nullptr;
  }

  inline void Unlink(Operation *instr) {
    Unlink(*instr);
  }

  inline void Delete(Operation *instr) {
    Unlink(*instr);
    delete instr;
  }

  void Swap(Operation *left, Operation *right);

  class Iterator final {
  private:
    struct M {
      OperationStream *stream;
      Operation *current;
    } m;

    explicit Iterator(M m) noexcept : m(std::move(m)) {
    }

  public:
    static Iterator Create(OperationStream *stream, Operation *instr) {
      return Iterator(M{
          .stream = stream,
          .current = instr,
      });
    }

    Iterator(const Iterator &other) : m(M{.stream = other.m.stream, .current = other.m.current}) {
    }

    Iterator &operator=(const Iterator &other) {
      m.stream = other.m.stream;
      m.current = other.m.current;
      return *this;
    }

    inline Iterator &operator++() {
      if (m.current) {
        m.current = m.current->m.next;
      }
      return *this;
    }

    inline Iterator operator++(int) {
      Iterator iter = *this;
      ++*this;
      return iter;
    }

    inline Iterator &operator--() {
      if (m.current) {
        m.current = m.current->m.prev;
      } else {
        m.current = m.stream->m.tail;
      }
      return *this;
    }

    inline Iterator operator--(int) {
      Iterator iter = *this;
      --*this;
      return iter;
    }

    inline Iterator &operator+=(intptr_t amount) {
      if (amount < 0) {
        return *this -= (-amount);
      }
      while (m.current && amount > 0) {
        m.current = m.current->m.next;
        --amount;
      }
      return *this;
    }

    inline Iterator &operator-=(intptr_t amount) {
      if (amount < 0) {
        return *this += (-amount);
      }
      while (m.current && amount > 0) {
        m.current = m.current->m.prev;
        --amount;
      }
      return *this;
    }

    inline Iterator operator+(intptr_t amount) const {
      Iterator iter = *this;
      iter += amount;
      return iter;
    }

    inline Iterator operator-(intptr_t amount) const {
      Iterator iter = *this;
      iter -= amount;
      return iter;
    }

    inline bool operator==(const Iterator &iter) const noexcept {
      return m.current == iter.m.current;
    }

    inline bool operator!=(const Iterator &iter) const noexcept {
      return !(*this == iter);
    }

    inline Operation *operator*() {
      return m.current;
    }

    inline const Operation *operator*() const {
      return m.current;
    }

    inline Operation *operator->() {
      return m.current;
    }

    inline const Operation *operator->() const {
      return m.current;
    }

    inline Iterator &JumpTo(Operation *instr) {
      m.current = instr;
      return *this;
    }

    bool LookingAt(const std::initializer_list<Instruction> pattern);
  };

  Iterator Begin() {
    return Iterator::Create(this, m.head);
  }

  Iterator begin() {
    return Begin();
  }

  Iterator End() {
    return Iterator::Create(this, nullptr);
  }

  Iterator end() {
    return End();
  }

  Iterator From(Operation *instr) {
    return Iterator::Create(this, instr);
  }

  inline void Delete(Iterator &iter) {
    Delete(*iter);
  }

  inline void Delete(Iterator &&iter) {
    Delete(*iter);
  }

  void Dump();
};

#endif /* BF_CC_INSTR_H */
