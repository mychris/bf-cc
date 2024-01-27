// SPDX-License-Identifier: MIT License
#include <initializer_list>
#ifndef BF_CC_INSTR_H
#define BF_CC_INSTR_H 1

#include <cassert>
#include <cstdint>

#include <utility>

#include "error.h"

enum class InstrCode {
  NOP = 1 << 0,
  INCR_CELL = 1 << 1,
  DECR_CELL = 1 << 2,
  SET_CELL = 1 << 3,
  INCR_PTR = 1 << 4,
  DECR_PTR = 1 << 5,
  // SET_PTR = 1 << 6,
  READ = 1 << 7,
  WRITE = 1 << 8,
  JUMP_ZERO = 1 << 9,
  JUMP_NON_ZERO = 1 << 10,
  FIND_CELL_LOW = 1 << 11,
  FIND_CELL_HIGH = 1 << 12,
  ANY = 1 << 13,
};

class Instr final {
public:
  typedef intptr_t operand_type;

private:
  struct M {
    InstrCode op_code{InstrCode::NOP};
    Instr *next{nullptr};
    Instr *prev{nullptr};
    intptr_t operand1{0};
    intptr_t operand2{0};
  } m;

  Instr(const Instr &) = delete;
  Instr &operator=(const Instr &) = delete;

  explicit Instr(M m) : m(std::move(m)) {
  }

  static Instr Create(enum InstrCode op_code, intptr_t op1 = 0, intptr_t op2 = 0) {
    return Instr(M{
        .op_code = op_code,
        .next = nullptr,
        .prev = nullptr,
        .operand1 = op1,
        .operand2 = op2,
    });
  }

  static Instr *Allocate(enum InstrCode op_code, intptr_t op1 = 0, intptr_t op2 = 0) {
    Instr *instr = new (std::nothrow) Instr(M{
        .op_code = op_code,
        .next = nullptr,
        .prev = nullptr,
        .operand1 = op1,
        .operand2 = op2,
    });
    if (!instr) {
      Error(Err::OutOfMemory());
    }
    return instr;
  }

  inline void SetNext(Instr *next) {
    m.next = next;
  }

  inline void SetPrev(Instr *prev) {
    m.prev = prev;
  }

public:
  Instr(Instr &&other) : m(std::exchange(other.m, {InstrCode::NOP, nullptr, nullptr, 0, 0})) {
  }

  Instr &operator=(Instr &&other) noexcept {
    std::swap(m, other.m);
    return *this;
  }

  ~Instr() = default;

  inline InstrCode OpCode() const {
    return m.op_code;
  }

  inline void SetOpCode(enum InstrCode cmd) {
    m.op_code = cmd;
  }

  inline bool IsJump() const {
    return m.op_code == InstrCode::JUMP_ZERO || m.op_code == InstrCode::JUMP_NON_ZERO;
  }

  inline bool IsIO() const {
    return m.op_code == InstrCode::READ || m.op_code == InstrCode::WRITE;
  }

  inline intptr_t Operand1() const {
    return m.operand1;
  }

  inline void SetOperand1(intptr_t val) {
    m.operand1 = val;
  }

  inline intptr_t Operand2() const {
    return m.operand2;
  }

  inline void SetOperand2(intptr_t val) {
    m.operand2 = val;
  }

  inline Instr *Next() {
    return m.next;
  }

  inline Instr *Prev() {
    return m.prev;
  }

  class Stream final {
  private:
    struct M {
      Instr *head;
      Instr *tail;
      std::size_t length;
    } m;

    Stream(const Stream &) = delete;
    Stream &operator=(const Stream &) = delete;

    explicit Stream(M m) : m(std::move(m)) {
    }

  public:
    Stream(Stream &&other) noexcept : m(std::exchange(other.m, {})) {
    }

    Stream &operator=(Stream &&other) noexcept {
      std::swap(m, other.m);
      return *this;
    }

    static Stream Create() noexcept {
      return Stream(M{});
    }

    ~Stream() {
      Instr *op = m.head;
      while (op) {
        Instr *next = op->Next();
        delete op;
        op = next;
      }
      m.head = nullptr;
      m.tail = nullptr;
      m.length = 0;
    }

    inline Instr *First() {
      return m.head;
    }

    inline Instr *Last() {
      return m.tail;
    }

    inline void Append(InstrCode code, intptr_t op1 = 0, intptr_t op2 = 0) {
      Instr *instr = Instr::Allocate(code, op1, op2);
      ++m.length;
      if (!m.head) {
        m.head = instr;
        m.tail = instr;
        instr->SetNext(nullptr);
        instr->SetPrev(nullptr);
      } else {
        m.tail->SetNext(instr);
        instr->SetPrev(m.tail);
        m.tail = instr;
      }
    }

    inline void Prepend(InstrCode code, intptr_t op1 = 0, intptr_t op2 = 0) {
      Instr *instr = Instr::Allocate(code, op1, op2);
      ++m.length;
      if (!m.head) {
        m.head = instr;
        m.tail = instr;
        instr->SetNext(nullptr);
        instr->SetPrev(nullptr);
      } else {
        instr->SetNext(m.head);
        m.head->SetPrev(instr);
        m.head = instr;
      }
    }

    inline void InsertBefore(Instr *instr, InstrCode code, intptr_t op1 = 0, intptr_t op2 = 0) {
      if (nullptr == instr) {
        Append(code, op1, op2);
      } else if (nullptr == instr->Prev()) {
        Prepend(code, op1, op2);
      } else {
        Instr *prev = instr->Prev();
        Instr *next = instr;
        Instr *new_instr = Instr::Allocate(code, op1, op2);
        prev->SetNext(new_instr);
        next->SetPrev(new_instr);
        new_instr->SetPrev(prev);
        new_instr->SetNext(next);
      }
    }

    inline void Unlink(Instr &instr) {
      --m.length;
      if (m.head == &instr) {
        if (m.tail == &instr) {
          m.head = nullptr;
          m.tail = nullptr;
        } else {
          m.head = instr.Next();
          m.head->SetPrev(nullptr);
        }
      } else if (m.tail == &instr) {
        m.tail = instr.Prev();
        m.tail->SetNext(nullptr);
      } else {
        instr.Prev()->SetNext(instr.Next());
        instr.Next()->SetPrev(instr.Prev());
      }
      instr.SetNext(nullptr);
      instr.SetPrev(nullptr);
    }

    inline void Unlink(Instr *instr) {
      Unlink(*instr);
    }

    inline void Delete(Instr *instr) {
      Unlink(*instr);
      delete instr;
    }

    class Iterator final {
    private:
      struct M {
        Stream *stream;
        Instr *current;
      } m;

      explicit Iterator(M m) noexcept : m(std::move(m)) {
      }

    public:

      static Iterator Create(Stream *stream, Instr *instr) {
        return Iterator(M{
            .stream = stream,
            .current = instr,
        });
      }

      Iterator(const Iterator &other)
        : m(M{ .stream = other.m.stream, .current = other.m.current} ) {
      }

      Iterator &operator=(const Iterator &other) {
        m.stream = other.m.stream;
        m.current = other.m.current;
        return *this;
      }

      inline Iterator &operator++() {
        if (m.current) {
          m.current = m.current->Next();
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
          m.current = m.current->Prev();
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
          m.current = m.current->Next();
          --amount;
        }
        return *this;
      }

      inline Iterator &operator-=(intptr_t amount) {
        if (amount < 0) {
          return *this += (-amount);
        }
        while (m.current && amount > 0) {
          m.current = m.current->Prev();
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

      inline Instr *operator*() {
        return m.current;
      }

      inline Instr *operator->() {
        return m.current;
      }

      inline Iterator &JumpTo(Instr *instr) {
        m.current = instr;
        return *this;
      }

      bool LookingAt(const std::initializer_list<InstrCode> pattern);
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

    Iterator From(Instr *instr) {
      return Iterator::Create(this, instr);
    }

    inline void Delete(Iterator &iter) {
      Delete(*iter);
    }

    inline void Delete(Iterator &&iter) {
      Delete(*iter);
    }

    void Dump();

    void VisitPattern(std::initializer_list<InstrCode> pattern, void (*fun)(Instr::Stream &, Instr::Stream::Iterator &));

  };
};

#endif /* BF_CC_INSTR_H */
