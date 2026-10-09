

#ifndef FMT_OS_H_
#define FMT_OS_H_

#include "format.h"

#ifndef FMT_MODULE
#  include <cerrno>
#  include <cstddef>
#  include <cstdio>
#  include <system_error>

#  if FMT_HAS_INCLUDE(<xlocale.h>)
#    include <xlocale.h>
#  endif
#endif

#ifndef FMT_USE_FCNTL

#  if FMT_HAS_INCLUDE("winapifamily.h")
#    include <winapifamily.h>
#  endif
#  if (FMT_HAS_INCLUDE(<fcntl.h>) || defined(__APPLE__) || \
       defined(__linux__)) &&                              \
      (!defined(WINAPI_FAMILY) ||                          \
       (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)) &&    \
      !defined(__wasm__)
#    include <fcntl.h>
#    define FMT_USE_FCNTL 1
#  else
#    define FMT_USE_FCNTL 0
#  endif
#endif

#ifndef FMT_POSIX
#  if defined(_WIN32) && !defined(__MINGW32__)

#    define FMT_POSIX(call) _##call
#  else
#    define FMT_POSIX(call) call
#  endif
#endif

#ifdef FMT_SYSTEM
#  define FMT_HAS_SYSTEM
#  define FMT_POSIX_CALL(call) FMT_SYSTEM(call)
#else
#  define FMT_SYSTEM(call) ::call
#  ifdef _WIN32

#    define FMT_POSIX_CALL(call) ::_##call
#  else
#    define FMT_POSIX_CALL(call) ::call
#  endif
#endif

#ifndef _WIN32
#  define FMT_RETRY_VAL(result, expression, error_result) \
    do {                                                  \
      (result) = (expression);                            \
    } while ((result) == (error_result) && errno == EINTR)
#else
#  define FMT_RETRY_VAL(result, expression, error_result) result = (expression)
#endif

#define FMT_RETRY(result, expression) FMT_RETRY_VAL(result, expression, -1)

FMT_BEGIN_NAMESPACE
FMT_BEGIN_EXPORT

template <typename Char> class basic_cstring_view {
 private:
  const Char* data_;

 public:

  basic_cstring_view(const Char* s) : data_(s) {}

  basic_cstring_view(const std::basic_string<Char>& s) : data_(s.c_str()) {}

  auto c_str() const -> const Char* { return data_; }
};

using cstring_view = basic_cstring_view<char>;
using wcstring_view = basic_cstring_view<wchar_t>;

#ifdef _WIN32
FMT_API const std::error_category& system_category() noexcept;

namespace detail {
FMT_API void format_windows_error(buffer<char>& out, int error_code,
                                  const char* message) noexcept;
}

FMT_API std::system_error vwindows_error(int error_code, string_view fmt,
                                         format_args args);

template <typename... T>
auto windows_error(int error_code, string_view message, const T&... args)
    -> std::system_error {
  return vwindows_error(error_code, message, vargs<T...>{{args...}});
}

FMT_API void report_windows_error(int error_code, const char* message) noexcept;
#else
inline auto system_category() noexcept -> const std::error_category& {
  return std::system_category();
}
#endif

#ifdef __OSX__
template <typename S, typename... Args, typename Char = char_t<S>>
void say(const S& fmt, Args&&... args) {
  std::system(format("say \"{}\"", format(fmt, args...)).c_str());
}
#endif

class buffered_file {
 private:
  FILE* file_;

  friend class file;

  inline explicit buffered_file(FILE* f) : file_(f) {}

 public:
  buffered_file(const buffered_file&) = delete;
  void operator=(const buffered_file&) = delete;

  inline buffered_file() noexcept : file_(nullptr) {}

  FMT_API ~buffered_file() noexcept;

 public:
  inline buffered_file(buffered_file&& other) noexcept : file_(other.file_) {
    other.file_ = nullptr;
  }

  inline auto operator=(buffered_file&& other) -> buffered_file& {
    close();
    file_ = other.file_;
    other.file_ = nullptr;
    return *this;
  }

  FMT_API buffered_file(cstring_view filename, cstring_view mode);

  FMT_API void close();

  inline auto get() const noexcept -> FILE* { return file_; }

  FMT_API auto descriptor() const -> int;

  template <typename... T>
  inline void print(string_view fmt, const T&... args) {
    fmt::vargs<T...> vargs = {{args...}};
    detail::is_locking<T...>() ? fmt::vprint_buffered(file_, fmt, vargs)
                               : fmt::vprint(file_, fmt, vargs);
  }
};

#if FMT_USE_FCNTL

class FMT_API file {
 private:
  int fd_;

  explicit file(int fd) : fd_(fd) {}

  friend struct pipe;

 public:

  enum {
    RDONLY = FMT_POSIX(O_RDONLY),
    WRONLY = FMT_POSIX(O_WRONLY),
    RDWR = FMT_POSIX(O_RDWR),
    CREATE = FMT_POSIX(O_CREAT),
    APPEND = FMT_POSIX(O_APPEND),
    TRUNC = FMT_POSIX(O_TRUNC)
  };

  inline file() noexcept : fd_(-1) {}

  file(cstring_view path, int oflag);

 public:
  file(const file&) = delete;
  void operator=(const file&) = delete;

  inline file(file&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }

  inline auto operator=(file&& other) -> file& {
    close();
    fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
  }

  ~file() noexcept;

  inline auto descriptor() const noexcept -> int { return fd_; }

  void close();

  auto size() const -> long long;

  auto read(void* buffer, size_t count) -> size_t;

  auto write(const void* buffer, size_t count) -> size_t;

  static auto dup(int fd) -> file;

  void dup2(int fd);

  void dup2(int fd, std::error_code& ec) noexcept;

  auto fdopen(const char* mode) -> buffered_file;

#  if defined(_WIN32) && !defined(__MINGW32__)

  static file open_windows_file(wcstring_view path, int oflag);
#  endif
};

struct FMT_API pipe {
  file read_end;
  file write_end;

  pipe();
};

auto getpagesize() -> long;

namespace detail {

struct buffer_size {
  constexpr buffer_size() = default;
  size_t value = 0;
  FMT_CONSTEXPR auto operator=(size_t val) const -> buffer_size {
    auto bs = buffer_size();
    bs.value = val;
    return bs;
  }
};

struct ostream_params {
  int oflag = file::WRONLY | file::CREATE | file::TRUNC;
  size_t buffer_size = BUFSIZ > 32768 ? BUFSIZ : 32768;

  constexpr ostream_params() {}

  template <typename... T>
  ostream_params(T... params, int new_oflag) : ostream_params(params...) {
    oflag = new_oflag;
  }

  template <typename... T>
  ostream_params(T... params, detail::buffer_size bs)
      : ostream_params(params...) {
    this->buffer_size = bs.value;
  }

#  if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 2000
  ostream_params(int new_oflag) : oflag(new_oflag) {}
  ostream_params(detail::buffer_size bs) : buffer_size(bs.value) {}
#  endif
};

}

FMT_INLINE_VARIABLE constexpr auto buffer_size = detail::buffer_size();

class ostream : private detail::buffer<char> {
 private:
  file file_;

  FMT_API ostream(cstring_view path, const detail::ostream_params& params);

  FMT_API static void grow(buffer<char>& buf, size_t);

 public:
  FMT_API ostream(ostream&& other) noexcept;
  FMT_API ~ostream();

  operator writer() {
    detail::buffer<char>& buf = *this;
    return buf;
  }

  inline void flush() {
    if (size() == 0) return;
    file_.write(data(), size() * sizeof(data()[0]));
    clear();
  }

  template <typename... T>
  friend auto output_file(cstring_view path, T... params) -> ostream;

  inline void close() {
    flush();
    file_.close();
  }

  template <typename... T> void print(format_string<T...> fmt, T&&... args) {
    vformat_to(appender(*this), fmt.str, vargs<T...>{{args...}});
  }
};

template <typename... T>
inline auto output_file(cstring_view path, T... params) -> ostream {
  return {path, detail::ostream_params(params...)};
}
#endif

FMT_END_EXPORT
FMT_END_NAMESPACE

#endif
