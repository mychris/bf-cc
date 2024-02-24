// SPDX-License-Identifier: MIT License
#ifndef BF_CC_ERROR_H
#define BF_CC_ERROR_H 1

#include <cstdint>
#include <utility>
#include <variant>

extern const char *program_name;

class Err final {
public:
  enum class Code {
    OK = 0,
    UNMATCHED_JUMP,
    OUT_OF_MEMORY,
    MEM_ALLOCATE,
    MEM_PROTECT,
    CODE_INVALID_OFFSET,
    IO,
  };

private:
  struct M {
    Err::Code code;
    int64_t native;
  } m;

  explicit Err(M m) noexcept : m(std::move(m)) {
  }

public:
  static Err Ok() noexcept {
    return Err(M{.code = Err::Code::OK, .native = 0});
  }
  static Err UnmatchedJump() noexcept {
    return Err(M{.code = Err::Code::UNMATCHED_JUMP, .native = 0});
  }
  static Err OutOfMemory() noexcept {
    return Err(M{.code = Err::Code::OUT_OF_MEMORY, .native = 0});
  }
  static Err MemAllocate(const int64_t err) noexcept {
    return Err(M{.code = Err::Code::MEM_ALLOCATE, .native = err});
  }
  static Err MemProtect(const int64_t err) noexcept {
    return Err(M{.code = Err::Code::MEM_PROTECT, .native = err});
  }
  static Err CodeInvalidOffset() noexcept {
    return Err(M{.code = Err::Code::CODE_INVALID_OFFSET, .native = 0});
  }
  static Err IO(const int64_t err) noexcept {
    return Err(M{.code = Err::Code::IO, .native = err});
  }

  inline bool IsOk() const noexcept {
    return m.code == Err::Code::OK;
  }

  inline Err::Code Code() const noexcept {
    return m.code;
  }

  inline int64_t NativeErrno() const noexcept {
    return m.native;
  }

  template <class F>
  constexpr auto and_then(F &&f) const & {
    if (IsOk()) {
      return f();
    }
    return *this;
  }

  template <class F>
  constexpr auto and_then(F &&f) const && {
    if (IsOk()) {
      return f();
    }
    return *this;
  }
};

void Error(const char *fmt, ...);

void Error(const Err &);

inline void Ensure(const Err &err) {
  if (!err.IsOk()) {
    Error(err);
  }
}

template <class T>
inline T Ensure(std::variant<T, Err> &&err) noexcept {
  if (0 != err.index()) {
    Error(std::get<Err>(std::move(err)));
  }
  return std::get<T>(std::move(err));
}

#endif /* BF_CC_ERROR_H */
