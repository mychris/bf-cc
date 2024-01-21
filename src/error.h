#ifndef BF_CC_ERROR_H
#define BF_CC_ERROR_H 1

#include <utility>
#include <variant>

extern const char* program_name;

enum class ErrorCode {
  OK = 0,
  OUT_OF_MEMORY,
  HEAP_MMAP,
  HEAP_MPROTECT,
  CODE_MMAP,
  CODE_MPROTECT,
  IO,
};

class Err final {
private:
  struct M {
    ErrorCode code;
    int native;
  } m;

  explicit Err(M m)
    : m(std::move(m))
  {}

public:
  static Err Ok() { return Err(M{ .code = ErrorCode::OK, .native = 0}); }
  static Err OutOfMemory() { return Err(M{ .code = ErrorCode::OUT_OF_MEMORY, .native = 0}); }
  static Err HeapMmap(int err) { return Err(M{ .code = ErrorCode::HEAP_MMAP, .native = err}); }
  static Err HeapMprotect(int err) { return Err(M{ .code = ErrorCode::HEAP_MPROTECT, .native = err}); }
  static Err CodeMmap(int err) { return Err(M{ .code = ErrorCode::CODE_MMAP, .native = err}); }
  static Err CodeMprotect(int err) { return Err(M{ .code = ErrorCode::CODE_MPROTECT, .native = err}); }
  static Err IO(int err) { return Err(M{ .code = ErrorCode::IO, .native = err}); }

  inline bool IsOk() const {
    return m.code == ErrorCode::OK;
  }

  inline ErrorCode Code() const {
    return m.code;
  }

  inline int NativeErrno() const {
    return m.native;
  }

  template<class F>
  constexpr auto and_then(F&& f) const & {
    if (IsOk()) {
      return f();
    }
    return *this;
  }

  template<class F>
  constexpr auto and_then(F&& f) const && {
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
inline T Ensure(std::variant<T, Err> &&err) noexcept
{
  if (0 != err.index()) {
    Error(std::get<Err>(std::move(err)));
  }
  return std::get<T>(std::move(err));
}

#endif /* BF_CC_ERROR_H */
