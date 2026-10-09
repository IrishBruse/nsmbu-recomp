// Formatting library for C++ - color support
//
// Copyright (c) 2018 - present, Victor Zverovich and fmt contributors
// All rights reserved.
//
// For the license information refer to format.h.
#ifndef FMT_COLOR_H_
#define FMT_COLOR_H_

#include "format.h"

FMT_BEGIN_NAMESPACE
FMT_BEGIN_EXPORT

enum class color : uint32_t {
  alice_blue = 0xF0F8FF,
  antique_white = 0xFAEBD7,
  aqua = 0x00FFFF,
  aquamarine = 0x7FFFD4,
  azure = 0xF0FFFF,
  beige = 0xF5F5DC,
  bisque = 0xFFE4C4,
  black = 0x000000,
  blanched_almond = 0xFFEBCD,
  blue = 0x0000FF,
  blue_violet = 0x8A2BE2,
  brown = 0xA52A2A,
  burly_wood = 0xDEB887,
  cadet_blue = 0x5F9EA0,
  chartreuse = 0x7FFF00,
  chocolate = 0xD2691E,
  coral = 0xFF7F50,
  cornflower_blue = 0x6495ED,
  cornsilk = 0xFFF8DC,
  crimson = 0xDC143C,
  cyan = 0x00FFFF,
  dark_blue = 0x00008B,
  dark_cyan = 0x008B8B,
  dark_golden_rod = 0xB8860B,
  dark_gray = 0xA9A9A9,
  dark_green = 0x006400,
  dark_khaki = 0xBDB76B,
  dark_magenta = 0x8B008B,
  dark_olive_green = 0x556B2F,
  dark_orange = 0xFF8C00,
  dark_orchid = 0x9932CC,
  dark_red = 0x8B0000,
  dark_salmon = 0xE9967A,
  dark_sea_green = 0x8FBC8F,
  dark_slate_blue = 0x483D8B,
  dark_slate_gray = 0x2F4F4F,
  dark_turquoise = 0x00CED1,
  dark_violet = 0x9400D3,
  deep_pink = 0xFF1493,
  deep_sky_blue = 0x00BFFF,
  dim_gray = 0x696969,
  dodger_blue = 0x1E90FF,
  fire_brick = 0xB22222,
  floral_white = 0xFFFAF0,
  forest_green = 0x228B22,
  fuchsia = 0xFF00FF,
  gainsboro = 0xDCDCDC,
  ghost_white = 0xF8F8FF,
  gold = 0xFFD700,
  golden_rod = 0xDAA520,
  gray = 0x808080,
  green = 0x008000,
  green_yellow = 0xADFF2F,
  honey_dew = 0xF0FFF0,
  hot_pink = 0xFF69B4,
  indian_red = 0xCD5C5C,
  indigo = 0x4B0082,
  ivory = 0xFFFFF0,
  khaki = 0xF0E68C,
  lavender = 0xE6E6FA,
  lavender_blush = 0xFFF0F5,
  lawn_green = 0x7CFC00,
  lemon_chiffon = 0xFFFACD,
  light_blue = 0xADD8E6,
  light_coral = 0xF08080,
  light_cyan = 0xE0FFFF,
  light_golden_rod_yellow = 0xFAFAD2,
  light_gray = 0xD3D3D3,
  light_green = 0x90EE90,
  light_pink = 0xFFB6C1,
  light_salmon = 0xFFA07A,
  light_sea_green = 0x20B2AA,
  light_sky_blue = 0x87CEFA,
  light_slate_gray = 0x778899,
  light_steel_blue = 0xB0C4DE,
  light_yellow = 0xFFFFE0,
  lime = 0x00FF00,
  lime_green = 0x32CD32,
  linen = 0xFAF0E6,
  magenta = 0xFF00FF,
  maroon = 0x800000,
  medium_aquamarine = 0x66CDAA,
  medium_blue = 0x0000CD,
  medium_orchid = 0xBA55D3,
  medium_purple = 0x9370DB,
  medium_sea_green = 0x3CB371,
  medium_slate_blue = 0x7B68EE,
  medium_spring_green = 0x00FA9A,
  medium_turquoise = 0x48D1CC,
  medium_violet_red = 0xC71585,
  midnight_blue = 0x191970,
  mint_cream = 0xF5FFFA,
  misty_rose = 0xFFE4E1,
  moccasin = 0xFFE4B5,
  navajo_white = 0xFFDEAD,
  navy = 0x000080,
  old_lace = 0xFDF5E6,
  olive = 0x808000,
  olive_drab = 0x6B8E23,
  orange = 0xFFA500,
  orange_red = 0xFF4500,
  orchid = 0xDA70D6,
  pale_golden_rod = 0xEEE8AA,
  pale_green = 0x98FB98,
  pale_turquoise = 0xAFEEEE,
  pale_violet_red = 0xDB7093,
  papaya_whip = 0xFFEFD5,
  peach_puff = 0xFFDAB9,
  peru = 0xCD853F,
  pink = 0xFFC0CB,
  plum = 0xDDA0DD,
  powder_blue = 0xB0E0E6,
  purple = 0x800080,
  rebecca_purple = 0x663399,
  red = 0xFF0000,
  rosy_brown = 0xBC8F8F,
  royal_blue = 0x4169E1,
  saddle_brown = 0x8B4513,
  salmon = 0xFA8072,
  sandy_brown = 0xF4A460,
  sea_green = 0x2E8B57,
  sea_shell = 0xFFF5EE,
  sienna = 0xA0522D,
  silver = 0xC0C0C0,
  sky_blue = 0x87CEEB,
  slate_blue = 0x6A5ACD,
  slate_gray = 0x708090,
  snow = 0xFFFAFA,
  spring_green = 0x00FF7F,
  steel_blue = 0x4682B4,
  tan = 0xD2B48C,
  teal = 0x008080,
  thistle = 0xD8BFD8,
  tomato = 0xFF6347,
  turquoise = 0x40E0D0,
  violet = 0xEE82EE,
  wheat = 0xF5DEB3,
  white = 0xFFFFFF,
  white_smoke = 0xF5F5F5,
  yellow = 0xFFFF00,
  yellow_green = 0x9ACD32
};

enum class terminal_color : uint8_t {
  black = 30,
  red,
  green,
  yellow,
  blue,
  magenta,
  cyan,
  white,
  bright_black = 90,
  bright_red,
  bright_green,
  bright_yellow,
  bright_blue,
  bright_magenta,
  bright_cyan,
  bright_white
};

enum class emphasis : uint8_t {
  bold = 1,
  faint = 1 << 1,
  italic = 1 << 2,
  underline = 1 << 3,
  blink = 1 << 4,
  reverse = 1 << 5,
  conceal = 1 << 6,
  strikethrough = 1 << 7,
};

struct rgb {
  constexpr rgb() : r(0), g(0), b(0) {}
  constexpr rgb(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) {}
  constexpr rgb(uint32_t hex)
      : r((hex >> 16) & 0xFF), g((hex >> 8) & 0xFF), b(hex & 0xFF) {}
  constexpr rgb(color hex)
      : r((uint32_t(hex) >> 16) & 0xFF),
        g((uint32_t(hex) >> 8) & 0xFF),
        b(uint32_t(hex) & 0xFF) {}
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

namespace detail {

struct color_type {
  constexpr color_type() noexcept = default;
  constexpr color_type(color rgb_color) noexcept
      : value_(static_cast<uint32_t>(rgb_color) | (1 << 24)) {}
  constexpr color_type(rgb rgb_color) noexcept
      : color_type(static_cast<color>(
            (static_cast<uint32_t>(rgb_color.r) << 16) |
            (static_cast<uint32_t>(rgb_color.g) << 8) | rgb_color.b)) {}
  constexpr color_type(terminal_color term_color) noexcept
      : value_(static_cast<uint32_t>(term_color) | (3 << 24)) {}

  constexpr auto is_terminal_color() const noexcept -> bool {
    return (value_ & (1 << 25)) != 0;
  }

  constexpr auto value() const noexcept -> uint32_t {
    return value_ & 0xFFFFFF;
  }

  constexpr color_type(uint32_t value) noexcept : value_(value) {}

  uint32_t value_ = 0;
};
}

class text_style {

 public:
  FMT_CONSTEXPR text_style(emphasis em = emphasis()) noexcept
      : style_(static_cast<uint64_t>(em) << 54) {}

  FMT_CONSTEXPR auto operator|=(text_style rhs) -> text_style& {
    if (((style_ + rhs.style_) & ((1ULL << 26) | (1ULL << 53))) != 0)
      report_error("can't OR a terminal color");
    style_ |= rhs.style_;
    return *this;
  }

  friend FMT_CONSTEXPR auto operator|(text_style lhs, text_style rhs)
      -> text_style {
    return lhs |= rhs;
  }

  FMT_CONSTEXPR auto operator==(text_style rhs) const noexcept -> bool {
    return style_ == rhs.style_;
  }

  FMT_CONSTEXPR auto operator!=(text_style rhs) const noexcept -> bool {
    return !(*this == rhs);
  }

  FMT_CONSTEXPR auto has_foreground() const noexcept -> bool {
    return (style_ & (1 << 24)) != 0;
  }
  FMT_CONSTEXPR auto has_background() const noexcept -> bool {
    return (style_ & (1ULL << 51)) != 0;
  }
  FMT_CONSTEXPR auto has_emphasis() const noexcept -> bool {
    return (style_ >> 54) != 0;
  }
  FMT_CONSTEXPR auto get_foreground() const noexcept -> detail::color_type {
    FMT_ASSERT(has_foreground(), "no foreground specified for this style");
    return style_ & 0x3FFFFFF;
  }
  FMT_CONSTEXPR auto get_background() const noexcept -> detail::color_type {
    FMT_ASSERT(has_background(), "no background specified for this style");
    return (style_ >> 27) & 0x3FFFFFF;
  }
  FMT_CONSTEXPR auto get_emphasis() const noexcept -> emphasis {
    FMT_ASSERT(has_emphasis(), "no emphasis specified for this style");
    return static_cast<emphasis>(style_ >> 54);
  }

 private:
  FMT_CONSTEXPR text_style(uint64_t style) noexcept : style_(style) {}

  friend FMT_CONSTEXPR auto fg(detail::color_type foreground) noexcept
      -> text_style;

  friend FMT_CONSTEXPR auto bg(detail::color_type background) noexcept
      -> text_style;

  uint64_t style_ = 0;
};

FMT_CONSTEXPR inline auto fg(detail::color_type foreground) noexcept
    -> text_style {
  return foreground.value_;
}

FMT_CONSTEXPR inline auto bg(detail::color_type background) noexcept
    -> text_style {
  return static_cast<uint64_t>(background.value_) << 27;
}

FMT_CONSTEXPR inline auto operator|(emphasis lhs, emphasis rhs) noexcept
    -> text_style {
  return text_style(lhs) | rhs;
}

namespace detail {

template <typename Char> struct ansi_color_escape {
  FMT_CONSTEXPR ansi_color_escape(color_type text_color,
                                  const char* esc) noexcept {

    if (text_color.is_terminal_color()) {
      bool is_background = esc == string_view("\x1b[48;2;");
      uint32_t value = text_color.value();

      if (is_background) value += 10u;

      buffer[size++] = static_cast<Char>('\x1b');
      buffer[size++] = static_cast<Char>('[');

      if (value >= 100u) {
        buffer[size++] = static_cast<Char>('1');
        value %= 100u;
      }
      buffer[size++] = static_cast<Char>('0' + value / 10u);
      buffer[size++] = static_cast<Char>('0' + value % 10u);

      buffer[size++] = static_cast<Char>('m');
      return;
    }

    for (int i = 0; i < 7; i++) {
      buffer[i] = static_cast<Char>(esc[i]);
    }
    rgb color(text_color.value());
    to_esc(color.r, buffer + 7, ';');
    to_esc(color.g, buffer + 11, ';');
    to_esc(color.b, buffer + 15, 'm');
    size = 19;
  }
  FMT_CONSTEXPR ansi_color_escape(emphasis em) noexcept {
    uint8_t em_codes[num_emphases] = {};
    if (has_emphasis(em, emphasis::bold)) em_codes[0] = 1;
    if (has_emphasis(em, emphasis::faint)) em_codes[1] = 2;
    if (has_emphasis(em, emphasis::italic)) em_codes[2] = 3;
    if (has_emphasis(em, emphasis::underline)) em_codes[3] = 4;
    if (has_emphasis(em, emphasis::blink)) em_codes[4] = 5;
    if (has_emphasis(em, emphasis::reverse)) em_codes[5] = 7;
    if (has_emphasis(em, emphasis::conceal)) em_codes[6] = 8;
    if (has_emphasis(em, emphasis::strikethrough)) em_codes[7] = 9;

    buffer[size++] = static_cast<Char>('\x1b');
    buffer[size++] = static_cast<Char>('[');

    for (size_t i = 0; i < num_emphases; ++i) {
      if (!em_codes[i]) continue;
      buffer[size++] = static_cast<Char>('0' + em_codes[i]);
      buffer[size++] = static_cast<Char>(';');
    }

    buffer[size - 1] = static_cast<Char>('m');
  }
  FMT_CONSTEXPR operator const Char*() const noexcept { return buffer; }

  FMT_CONSTEXPR auto begin() const noexcept -> const Char* { return buffer; }
  FMT_CONSTEXPR auto end() const noexcept -> const Char* {
    return buffer + size;
  }

 private:
  static constexpr size_t num_emphases = 8;
  Char buffer[7u + 4u * num_emphases] = {};
  size_t size = 0;

  static FMT_CONSTEXPR void to_esc(uint8_t c, Char* out,
                                   char delimiter) noexcept {
    out[0] = static_cast<Char>('0' + c / 100);
    out[1] = static_cast<Char>('0' + c / 10 % 10);
    out[2] = static_cast<Char>('0' + c % 10);
    out[3] = static_cast<Char>(delimiter);
  }
  static FMT_CONSTEXPR auto has_emphasis(emphasis em, emphasis mask) noexcept
      -> bool {
    return static_cast<uint8_t>(em) & static_cast<uint8_t>(mask);
  }
};

template <typename Char>
FMT_CONSTEXPR auto make_foreground_color(color_type foreground) noexcept
    -> ansi_color_escape<Char> {
  return ansi_color_escape<Char>(foreground, "\x1b[38;2;");
}

template <typename Char>
FMT_CONSTEXPR auto make_background_color(color_type background) noexcept
    -> ansi_color_escape<Char> {
  return ansi_color_escape<Char>(background, "\x1b[48;2;");
}

template <typename Char>
FMT_CONSTEXPR auto make_emphasis(emphasis em) noexcept
    -> ansi_color_escape<Char> {
  return ansi_color_escape<Char>(em);
}

template <typename Char> inline void reset_color(buffer<Char>& buffer) {
  auto reset_color = string_view("\x1b[0m");
  buffer.append(reset_color.begin(), reset_color.end());
}

template <typename T> struct styled_arg : view {
  const T& value;
  text_style style;
  styled_arg(const T& v, text_style s) : value(v), style(s) {}
};

template <typename Char>
void vformat_to(buffer<Char>& buf, text_style ts, basic_string_view<Char> fmt,
                basic_format_args<buffered_context<Char>> args) {
  if (ts.has_emphasis()) {
    auto emphasis = make_emphasis<Char>(ts.get_emphasis());
    buf.append(emphasis.begin(), emphasis.end());
  }
  if (ts.has_foreground()) {
    auto foreground = make_foreground_color<Char>(ts.get_foreground());
    buf.append(foreground.begin(), foreground.end());
  }
  if (ts.has_background()) {
    auto background = make_background_color<Char>(ts.get_background());
    buf.append(background.begin(), background.end());
  }
  vformat_to(buf, fmt, args);
  if (ts != text_style()) reset_color<Char>(buf);
}
}

inline void vprint(FILE* f, text_style ts, string_view fmt, format_args args) {
  auto buf = memory_buffer();
  detail::vformat_to(buf, ts, fmt, args);
  print(f, FMT_STRING("{}"), string_view(buf.begin(), buf.size()));
}

template <typename... T>
void print(FILE* f, text_style ts, format_string<T...> fmt, T&&... args) {
  vprint(f, ts, fmt.str, vargs<T...>{{args...}});
}

template <typename... T>
void print(text_style ts, format_string<T...> fmt, T&&... args) {
  return print(stdout, ts, fmt, std::forward<T>(args)...);
}

inline auto vformat(text_style ts, string_view fmt, format_args args)
    -> std::string {
  auto buf = memory_buffer();
  detail::vformat_to(buf, ts, fmt, args);
  return fmt::to_string(buf);
}

template <typename... T>
inline auto format(text_style ts, format_string<T...> fmt, T&&... args)
    -> std::string {
  return fmt::vformat(ts, fmt.str, vargs<T...>{{args...}});
}

template <typename OutputIt,
          FMT_ENABLE_IF(detail::is_output_iterator<OutputIt, char>::value)>
auto vformat_to(OutputIt out, text_style ts, string_view fmt, format_args args)
    -> OutputIt {
  auto&& buf = detail::get_buffer<char>(out);
  detail::vformat_to(buf, ts, fmt, args);
  return detail::get_iterator(buf, out);
}

template <typename OutputIt, typename... T,
          FMT_ENABLE_IF(detail::is_output_iterator<OutputIt, char>::value)>
inline auto format_to(OutputIt out, text_style ts, format_string<T...> fmt,
                      T&&... args) -> OutputIt {
  return vformat_to(out, ts, fmt.str, vargs<T...>{{args...}});
}

template <typename T, typename Char>
struct formatter<detail::styled_arg<T>, Char> : formatter<T, Char> {
  template <typename FormatContext>
  auto format(const detail::styled_arg<T>& arg, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    const auto& ts = arg.style;
    auto out = ctx.out();

    bool has_style = false;
    if (ts.has_emphasis()) {
      has_style = true;
      auto emphasis = detail::make_emphasis<Char>(ts.get_emphasis());
      out = detail::copy<Char>(emphasis.begin(), emphasis.end(), out);
    }
    if (ts.has_foreground()) {
      has_style = true;
      auto foreground =
          detail::make_foreground_color<Char>(ts.get_foreground());
      out = detail::copy<Char>(foreground.begin(), foreground.end(), out);
    }
    if (ts.has_background()) {
      has_style = true;
      auto background =
          detail::make_background_color<Char>(ts.get_background());
      out = detail::copy<Char>(background.begin(), background.end(), out);
    }
    out = formatter<T, Char>::format(arg.value, ctx);
    if (has_style) {
      auto reset_color = string_view("\x1b[0m");
      out = detail::copy<Char>(reset_color.begin(), reset_color.end(), out);
    }
    return out;
  }
};

template <typename T>
FMT_CONSTEXPR auto styled(const T& value, text_style ts)
    -> detail::styled_arg<remove_cvref_t<T>> {
  return detail::styled_arg<remove_cvref_t<T>>{value, ts};
}

FMT_END_EXPORT
FMT_END_NAMESPACE

#endif
