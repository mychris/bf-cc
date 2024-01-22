// SPDX-License-Identifier: MIT License
#ifndef BF_CC_ERROR_H
#define BF_CC_ERROR_H 1

#include <utility>
#include <variant>

extern const char *program_name;

class Err final {
public:
  enum class Code {
    OK = 0,
    OUT_OF_MEMORY,
    HEAP_MMAP,
    HEAP_MPROTECT,
    CODE_MMAP,
    CODE_MPROTECT,
    CODE_INVALID_OFFSET,
    IO,
  };

private:
  struct M {
    Err::Code code;
    int native;
  } m;

  explicit Err(M m) noexcept : m(std::move(m)) {
  }

public:
  static Err Ok() noexcept {
    return Err(M{.code = Err::Code::OK, .native = 0});
  }
  static Err OutOfMemory() noexcept {
    return Err(M{.code = Err::Code::OUT_OF_MEMORY, .native = 0});
  }
  static Err HeapMmap(const int err) noexcept {
    return Err(M{.code = Err::Code::HEAP_MMAP, .native = err});
  }
  static Err HeapMprotect(const int err) noexcept {
    return Err(M{.code = Err::Code::HEAP_MPROTECT, .native = err});
  }
  static Err CodeMmap(const int err) noexcept {
    return Err(M{.code = Err::Code::CODE_MMAP, .native = err});
  }
  static Err CodeMprotect(const int err) noexcept {
    return Err(M{.code = Err::Code::CODE_MPROTECT, .native = err});
  }
  static Err CodeInvalidOffset() noexcept {
    return Err(M{.code = Err::Code::CODE_INVALID_OFFSET, .native = 0});
  }
  static Err IO(const int err) noexcept {
    return Err(M{.code = Err::Code::IO, .native = err});
  }

  inline bool IsOk() const noexcept {
    return m.code == Err::Code::OK;
  }

  inline Err::Code Code() const noexcept {
    return m.code;
  }

  inline int NativeErrno() const noexcept {
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
