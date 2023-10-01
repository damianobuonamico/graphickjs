#include "svg.h"

#include "../../utils/console.h"

#include "../../math/vec2.h"
#include "../../math/vec4.h"

#include "../../editor/editor.h"
#include "../../editor/scene/entity.h"

#include "../../renderer/geometry/path.h"

#include <iostream>

#define IS_ALPHA(c) ((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z')
#define IS_NUM(c) ((c) >= '0' && (c) <= '9')
#define IS_WS(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')

#define IS_STARTNAMECHAR(c) (IS_ALPHA(c) ||  (c) == '_' || (c) == ':')
#define IS_NAMECHAR(c) (IS_STARTNAMECHAR(c) || IS_NUM(c) || (c) == '-' || (c) == '.')

namespace Graphick::io::svg {

  static const char* rtrim(const char* start, const char* end) {
    while (end > start && IS_WS(end[-1])) {
      end--;
    }

    return end;
  }

  static bool skip_desc(const char*& ptr, const char* end, char ch) {
    if (ptr >= end || *ptr != ch) return false;

    ptr++;
    return true;
  }

  static bool skip_desc(const char*& ptr, const char* end, const char* data) {
    int read = 0;
    while (data[read]) {
      if (ptr >= end || *ptr != data[read]) {
        ptr -= read;
        return false;
      }

      read++;
      ptr++;
    }

    return true;
  }

  static bool skip_until(const char*& ptr, const char* end, char ch) {
    while (ptr < end && *ptr != ch) {
      ptr++;
    }

    return ptr < end;
  }

  static bool skip_until(const char*& ptr, const char* end, const char* data) {
    while (ptr < end) {
      const char* start = ptr;
      if (skip_desc(start, end, data)) break;
      ptr++;
    }

    return ptr < end;
  }

  static bool skip_ws(const char*& ptr, const char* end) {
    while (ptr < end && IS_WS(*ptr)) {
      ptr++;
    }

    return ptr < end;
  }

  static bool skip_ws_delimiter(const char*& ptr, const char* end, const char delimiter) {
    if (ptr < end && !IS_WS(*ptr) && *ptr != delimiter) return false;

    if (skip_ws(ptr, end)) {
      if (ptr < end && *ptr == delimiter) {
        ++ptr;
        skip_ws(ptr, end);
      }
    }

    return ptr < end;
  }

  static bool skip_ws_comma(const char*& ptr, const char* end) {
    return skip_ws_delimiter(ptr, end, ',');
  }

  static bool read_identifier(const char*& ptr, const char* end, std::string& value) {
    if (ptr >= end || !IS_STARTNAMECHAR(*ptr)) return false;

    const char* start = ptr;
    ptr++;

    while (ptr < end && IS_NAMECHAR(*ptr)) {
      ptr++;
    }

    value.assign(start, ptr);
    return true;
  }

  inline bool is_integral_digit(char ch, int base) {
    if (IS_NUM(ch)) {
      return ch - '0' < base;
    }

    if (IS_ALPHA(ch)) {
      return (ch >= 'a' && ch < 'a' + std::min(base, 36) - 10) || (ch >= 'A' && ch < 'A' + std::min(base, 36) - 10);
    }

    return false;
  }

  template <typename I>
  static bool parse_integer(const char*& ptr, const char* end, I& integer, int base = 10)
  {
    bool is_negative = 0;
    I value = 0;

    static const I int_max = std::numeric_limits<I>::max();
    static const bool is_signed = std::numeric_limits<I>::is_signed;
    using signed_t = typename std::make_signed<I>::type;
    const I max_multiplier = int_max / static_cast<I>(base);

    if (ptr < end && *ptr == '+') {
      ++ptr;
    } else if (ptr < end && is_signed && *ptr == '-') {
      ++ptr;
      is_negative = true;
    }

    if (ptr >= end || !is_integral_digit(*ptr, base)) return false;

    while (ptr < end && is_integral_digit(*ptr, base)) {
      const char ch = *ptr++;
      int digit_value;

      if (IS_NUM(ch)) {
        digit_value = ch - '0';
      } else if (ch >= 'a') {
        digit_value = ch - 'a' + 10;
      } else {
        digit_value = ch - 'A' + 10;
      }

      if (value > max_multiplier || (value == max_multiplier && static_cast<I>(digit_value) > (int_max % static_cast<I>(base)) + is_negative)) return false;
      value = static_cast<I>(base) * value + static_cast<I>(digit_value);

      if (is_negative) {
        integer = -static_cast<signed_t>(value);
      } else {
        integer = value;
      }
    }

    return true;
  }

  template<typename I>
  static  bool parse_number(const char*& ptr, const char* end, I& number) {
    I integer, fraction;
    int sign, expsign, exponent;

    static const I number_max = std::numeric_limits<I>::max();
    fraction = 0;
    integer = 0;
    exponent = 0;
    sign = 1;
    expsign = 1;

    if (ptr < end && *ptr == '+') {
      ++ptr;
    } else if (ptr < end && *ptr == '-') {
      ++ptr;
      sign = -1;
    }

    if (ptr >= end || !(IS_NUM(*ptr) || *ptr == '.')) return false;

    if (*ptr != '.') {
      while (ptr < end && IS_NUM(*ptr)) {
        integer = static_cast<I>(10) * integer + (*ptr - '0');
        ++ptr;
      }
    }

    if (ptr < end && *ptr == '.') {
      ++ptr;
      if (ptr >= end || !IS_NUM(*ptr)) return false;

      I divisor = 1;

      while (ptr < end && IS_NUM(*ptr)) {
        fraction = static_cast<I>(10) * fraction + (*ptr - '0');
        divisor *= static_cast<I>(10);
        ++ptr;
      }

      fraction /= divisor;
    }

    if (
      ptr < end && (*ptr == 'e' || *ptr == 'E')
      && (ptr[1] != 'x' && ptr[1] != 'm')
      ) {
      ++ptr;
      if (ptr < end && *ptr == '+') {
        ++ptr;
      } else if (ptr < end && *ptr == '-') {
        ++ptr;
        expsign = -1;
      }

      if (ptr >= end || !IS_NUM(*ptr)) return false;

      while (ptr < end && IS_NUM(*ptr)) {
        exponent = 10 * exponent + (*ptr - '0');
        ++ptr;
      }
    }

    number = sign * (integer + fraction);
    if (exponent) {
      number *= static_cast<I>(pow(10.0, expsign * exponent));
    }

    return number >= -number_max && number <= number_max;
  }

  static bool parse_number_list(const char*& ptr, const char* end, float* values, int count) {
    for (int i = 0; i < count; i++) {
      if (!parse_number(ptr, end, values[i])) return false;
      skip_ws_comma(ptr, end);
    }

    return true;
  }

  static bool parse_arc_flag(const char*& ptr, const char* end, bool& flag) {
    if (ptr < end && *ptr == '0') {
      flag = false;
    } else if (ptr < end && *ptr == '1') {
      flag = true;
    } else {
      return false;
    }

    ptr++;
    skip_ws_comma(ptr, end);
    return true;
  }

  static bool decode_text(const char* ptr, const char* end, std::string& value) {
    value.clear();

    while (ptr < end) {
      char ch = *ptr;
      ptr++;

      if (ch != '&') {
        value.push_back(ch);
        continue;
      }

      if (skip_desc(ptr, end, '#')) {
        int base = 10;
        if (skip_desc(ptr, end, 'x')) {
          base = 16;
        }

        unsigned int cp;
        if (!parse_integer(ptr, end, cp, base)) return false;

        char c[5] = { 0, 0, 0, 0, 0 };
        if (cp < 0x80) {
          c[1] = 0;
          c[0] = cp;
        } else if (cp < 0x800) {
          c[2] = 0;
          c[1] = (cp & 0x3F) | 0x80;
          cp >>= 6;
          c[0] = cp | 0xC0;
        } else if (cp < 0x10000) {
          c[3] = 0;
          c[2] = (cp & 0x3F) | 0x80;
          cp >>= 6;
          c[1] = (cp & 0x3F) | 0x80;
          cp >>= 6;
          c[0] = cp | 0xE0;
        } else if (cp < 0x200000) {
          c[4] = 0;
          c[3] = (cp & 0x3F) | 0x80;
          cp >>= 6;
          c[2] = (cp & 0x3F) | 0x80;
          cp >>= 6;
          c[1] = (cp & 0x3F) | 0x80;
          cp >>= 6;
          c[0] = cp | 0xF0;
        }

        value.append(c);
      } else {
        if (skip_desc(ptr, end, "amp")) {
          value.push_back('&');
        } else if (skip_desc(ptr, end, "lt")) {
          value.push_back('<');
        } else if (skip_desc(ptr, end, "gt")) {
          value.push_back('>');
        } else if (skip_desc(ptr, end, "quot")) {
          value.push_back('\"');
        } else if (skip_desc(ptr, end, "apos")) {
          value.push_back('\'');
        } else {
          return false;
        }
      }

      if (!skip_desc(ptr, end, ';'))
        return false;
    }

    return true;
  }

  static Renderer::Geometry::Path parse_path(const std::string& string) {
    const char* ptr = string.data();
    const char* end = ptr + string.size();

    Renderer::Geometry::Path path{ 0 };

    if (ptr >= end || !(*ptr == 'M' || *ptr == 'm')) return path;

    char command = *ptr++;
    char last_command = command;
    float c[6];
    bool f[2];

    vec2 start_point{ 0.0f }, current_point{ 0.0f }, control_point{ 0.0f };
    // std::vector<std::pair<UUID, std::shared_ptr<VertexEntity>>> vertices;

    while (true) {
      skip_ws(ptr, end);

      if (command == 'M' || command == 'm') {
        if (!parse_number_list(ptr, end, c, 2)) return path;

        if (command == 'm') {
          c[0] += current_point.x;
          c[1] += current_point.y;
        }

        // path.moveTo(c[0], c[1]);
        path.move_to({ c[0], c[1] });
        // auto vertex = std::make_shared<VertexEntity>(vec2{ c[0], c[1] });
        // vertices.push_back(std::make_pair(vertex->id, vertex));
        start_point.x = current_point.x = c[0];
        start_point.y = current_point.y = c[1];
        command = command == 'm' ? 'l' : 'L';
      } else if (command == 'L' || command == 'l') {
        if (!parse_number_list(ptr, end, c, 2)) return path;

        if (command == 'l')
        {
          c[0] += current_point.x;
          c[1] += current_point.y;
        }

        // path.lineTo(c[0], c[1]);
        path.line_to({ c[0], c[1] });
        // auto vertex = std::make_shared<VertexEntity>(vec2{ c[0], c[1] });
        // vertices.push_back(std::make_pair(vertex->id, vertex));
        current_point.x = c[0];
        current_point.y = c[1];
      } else if (command == 'H' || command == 'h') {
        if (!parse_number_list(ptr, end, c, 1)) return path;

        if (command == 'h')
          c[0] += current_point.x;

        // path.lineTo(c[0], current_point.y);
        path.line_to({ c[0], current_point.y });
        // auto vertex = std::make_shared<VertexEntity>(vec2{ c[0], current_point.y });
        // vertices.push_back(std::make_pair(vertex->id, vertex));
        current_point.x = c[0];
      } else if (command == 'V' || command == 'v') {
        if (!parse_number_list(ptr, end, c + 1, 1)) return path;

        if (command == 'v')
          c[1] += current_point.y;

        // path.lineTo(current_point.x, c[1]);
        path.line_to({ current_point.x, c[1] });
        // auto vertex = std::make_shared<VertexEntity>(vec2{ current_point.x, c[1] });
        // vertices.push_back(std::make_pair(vertex->id, vertex));
        current_point.y = c[1];
      } else if (command == 'Q' || command == 'q') {
        if (!parse_number_list(ptr, end, c, 4)) return path;

        if (command == 'q')
        {
          c[0] += current_point.x;
          c[1] += current_point.y;
          c[2] += current_point.x;
          c[3] += current_point.y;
        }

        path.quadratic_to({ c[0], c[1] }, { c[2], c[3] });
        // float cx1 = 2.0 / 3.0 * c[0] + 1.0 / 3.0 * current_point.x;
        // float cy1 = 2.0 / 3.0 * c[1] + 1.0 / 3.0 * current_point.y;
        // float cx2 = 2.0 / 3.0 * c[0] + 1.0 / 3.0 * c[2];
        // float cy2 = 2.0 / 3.0 * c[1] + 1.0 / 3.0 * c[3];
        // path.quadTo(current_point.x, current_point.y, c[0], c[1], c[2], c[3]);
        // vertices.back().second->set_right(vec2{ cx1, cy1 } - vec2{ current_point.x, current_point.y });
        // auto vertex = std::make_shared<VertexEntity>(vec2{ c[2], c[3] }, vec2{ cx2, cy2 } - vec2{ c[2], c[3] }, true);
        // vertices.push_back(std::make_pair(vertex->id, vertex));
        control_point.x = c[0];
        control_point.y = c[1];
        current_point.x = c[2];
        current_point.y = c[3];
      } else if (command == 'C' || command == 'c') {
        if (!parse_number_list(ptr, end, c, 6)) return path;

        if (command == 'c')
        {
          c[0] += current_point.x;
          c[1] += current_point.y;
          c[2] += current_point.x;
          c[3] += current_point.y;
          c[4] += current_point.x;
          c[5] += current_point.y;
        }

        path.cubic_to({ c[0], c[1] }, { c[2], c[3] }, { c[4], c[5] });
        // path.cubicTo(c[0], c[1], c[2], c[3], c[4], c[5]);
        // vertices.back().second->set_right(vec2{ c[0], c[1] } - vec2{ current_point.x, current_point.y });
        // auto vertex = std::make_shared<VertexEntity>(vec2{ c[4], c[5] }, vec2{ c[2], c[3] } - vec2{ c[4], c[5] }, true);
        // vertices.push_back(std::make_pair(vertex->id, vertex));
        control_point.x = c[2];
        control_point.y = c[3];
        current_point.x = c[4];
        current_point.y = c[5];
      } else if (command == 'T' || command == 't') {
        if (last_command != 'Q' && last_command != 'q' && last_command != 'T' && last_command != 't') {
          c[0] = current_point.x;
          c[1] = current_point.y;
        } else {
          c[0] = 2 * current_point.x - control_point.x;
          c[1] = 2 * current_point.y - control_point.y;
        }

        if (!parse_number_list(ptr, end, c + 2, 2)) return path;

        if (command == 't')
        {
          c[2] += current_point.x;
          c[3] += current_point.y;
        }

        path.quadratic_to({ c[0], c[1] }, { c[2], c[3] });
        // float cx1 = 2.0 / 3.0 * c[0] + 1.0 / 3.0 * current_point.x;
        // float cy1 = 2.0 / 3.0 * c[1] + 1.0 / 3.0 * current_point.y;
        // float cx2 = 2.0 / 3.0 * c[0] + 1.0 / 3.0 * c[2];
        // float cy2 = 2.0 / 3.0 * c[1] + 1.0 / 3.0 * c[3];
        // path.quadTo(current_point.x, current_point.y, c[0], c[1], c[2], c[3]);
        // vertices.back().second->set_right(vec2{ cx1, cy1 } - vec2{ current_point.x, current_point.y });
        // auto vertex = std::make_shared<VertexEntity>(vec2{ c[2], c[3] }, vec2{ cx2, cy2 } - vec2{ c[2], c[3] }, true);
        // vertices.push_back(std::make_pair(vertex->id, vertex));
        control_point.x = c[0];
        control_point.y = c[1];
        current_point.x = c[2];
        current_point.y = c[3];
      } else if (command == 'S' || command == 's') {
        if (last_command != 'C' && last_command != 'c' && last_command != 'S' && last_command != 's') {
          c[0] = current_point.x;
          c[1] = current_point.y;
        } else {
          c[0] = 2 * current_point.x - control_point.x;
          c[1] = 2 * current_point.y - control_point.y;
        }

        if (!parse_number_list(ptr, end, c + 2, 4)) return path;

        if (command == 's')
        {
          c[2] += current_point.x;
          c[3] += current_point.y;
          c[4] += current_point.x;
          c[5] += current_point.y;
        }

        path.cubic_to({ c[0], c[1] }, { c[2], c[3] }, { c[4], c[5] });
        // path.cubicTo(c[0], c[1], c[2], c[3], c[4], c[5]);
        // vertices.back().second->set_right(vec2{ c[0], c[1] } - vec2{ current_point.x, current_point.y });
        // auto vertex = std::make_shared<VertexEntity>(vec2{ c[4], c[5] }, vec2{ c[2], c[3] } - vec2{ c[4], c[5] }, true);
        // vertices.push_back(std::make_pair(vertex->id, vertex));
        control_point.x = c[2];
        control_point.y = c[3];
        current_point.x = c[4];
        current_point.y = c[5];
      } else if (command == 'A' || command == 'a') {
        if (!parse_number_list(ptr, end, c, 3)
          || !parse_arc_flag(ptr, end, f[0])
          || !parse_arc_flag(ptr, end, f[1])
          || !parse_number_list(ptr, end, c + 3, 2)) return path;
        // if (!parse_number_list(ptr, end, c, 3)) return path;

        if (command == 'a')
        {
          c[3] += current_point.x;
          c[4] += current_point.y;
        }

        path.arc_to({ current_point.x, current_point.y }, { c[0], c[1] }, c[2], f[0], f[1], { c[3], c[4] });
        // path.arcTo(current_point.x, current_point.y, c[0], c[1], c[2], f[0], f[1], c[3], c[4]);
        current_point.x = c[3];
        current_point.y = c[4];
      } else if (command == 'Z' || command == 'z') {
        path.close();
        current_point.x = start_point.x;
        current_point.y = start_point.y;
      } else {
        return path;
      }

      skip_ws_comma(ptr, end);
      if (ptr >= end) break;

      last_command = command;
      if (IS_ALPHA(*ptr)) {
        command = *ptr++;
      }
    }

    return path;
  }

  bool parse_svg(const char* svg) {
    const char* ptr = svg;
    const char* end = svg + strlen(svg);

    std::string name;
    std::string value;
    int ignoring = 0;
    std::vector<vec4> colors = { { 1.0f, 0.0f, 0.0f, 1.0f } };

    auto remove_comments = [](std::string& value) {
      size_t start = value.find("/*");
      while (start != std::string::npos) {
        size_t end = value.find("*/", start + 2);
        value.erase(start, end - start + 2);
        start = value.find("/*");
      }
      };

    auto handle_text = [&](const char* start, const char* end, bool in_cdata) {
      if (ignoring > 0) return;

      if (in_cdata) {
        value.assign(start, end);
      } else {
        // decode_text(start, end, value);
      }

      remove_comments(value);
      // style_sheet.parse(value);
      };

    while (ptr < end) {
      const char* start = ptr;
      if (!skip_until(ptr, end, '<')) break;

      handle_text(start, ptr, false);
      ptr++;

      if (ptr < end && *ptr == '/') {
        // if (current == nullptr && ignoring == 0) return false;

        ptr++;
        if (!read_identifier(ptr, end, name)) return false;

        if (name == "g") {
          colors.pop_back();
        }

        skip_ws(ptr, end);
        if (ptr >= end || *ptr != '>') return false;

        if (ignoring > 0) {
          ignoring--;
        } else {
          // current = current->parent;
        }

        ptr++;
        continue;
      }

      if (ptr < end && *ptr == '?') {
        ptr++;

        if (!read_identifier(ptr, end, name)) return false;
        if (!skip_until(ptr, end, "?>")) return false;

        ptr += 2;
        continue;
      }

      if (ptr < end && *ptr == '!') {
        ptr++;

        if (skip_desc(ptr, end, "--")) {
          start = ptr;
          if (!skip_until(ptr, end, "-->")) return false;

          handle_text(start, ptr, false);
          ptr += 3;
          continue;
        }

        if (skip_desc(ptr, end, "[CDATA[")) {
          start = ptr;

          if (!skip_until(ptr, end, "]]>")) return false;

          handle_text(start, ptr, true);
          ptr += 3;
          continue;
        }

        if (skip_desc(ptr, end, "DOCTYPE")) {
          while (ptr < end && *ptr != '>') {
            if (*ptr == '[') {
              ptr++;
              int depth = 1;

              while (ptr < end && depth > 0) {
                if (*ptr == '[') depth++;
                if (*ptr == ']') depth--;
                ptr++;
              }
            } else {
              ptr++;
            }
          }

          if (ptr >= end || *ptr != '>') return false;

          ptr++;
          continue;
        }

        return false;
      }

      if (!read_identifier(ptr, end, name)) return false;
      // TODO: Implement

      if (name == "g") {
        colors.push_back({ 0.0f, 0.0f, 0.0f, 0.0f });
      }
      std::string temp_name = name;

      skip_ws(ptr, end);
      while (ptr < end && read_identifier(ptr, end, name)) {
        skip_ws(ptr, end);
        if (ptr >= end || *ptr != '=') return false;

        ptr++;
        skip_ws(ptr, end);

        if (ptr >= end || !(*ptr == '\"' || *ptr == '\'')) return false;

        char quote = *ptr;
        ptr++;

        skip_ws(ptr, end);
        start = ptr;

        while (ptr < end && *ptr != quote) {
          ptr++;
        }

        if (ptr >= end || *ptr != quote) return false;

        if (!skip_until(ptr, end, quote)) return false;

        if (name == "fill") {
          decode_text(start, rtrim(start, ptr), value);
          // TOOD: refactor
          int r = 0, g = 0, b = 0;

          if (value.size() == 4) {
            std::stringstream ss;
            ss << std::hex << value.substr(1, 1) << value.substr(1, 1);
            ss >> r;
            ss.clear();

            ss << std::hex << value.substr(2, 1) << value.substr(2, 1);
            ss >> g;
            ss.clear();

            ss << std::hex << value.substr(3, 1) << value.substr(3, 1);
            ss >> b;
            ss.clear();
          } else if (value.size() == 7) {
            std::stringstream ss;
            ss << std::hex << value.substr(1, 2);
            ss >> r;
            ss.clear();

            ss << std::hex << value.substr(3, 2);
            ss >> g;
            ss.clear();

            ss << std::hex << value.substr(5, 2);
            ss >> b;
            ss.clear();
          }

          colors.back() = { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f };

        } if (name == "d") {
          // if (colors.back() != vec4{ 0.0f, 0.0f, 0.0f, 1.0f }) {
          decode_text(start, rtrim(start, ptr), value);
          // TODO: optimize copy
          Renderer::Geometry::Path path = parse_path(value);

          if (!path.empty()) {
            // TODO: reimplement svg creation, element creation, path optimization
            Editor::Entity element = Editor::Editor::scene().create_element();
            element.add_component<Editor::FillComponent>(colors.back());
          }
        }

        ptr++;
        skip_ws(ptr, end);
      }
    }

    return true;
  }

  bool parse_svg(const std::string& svg) {
    return parse_svg(svg.c_str());
  }

}
