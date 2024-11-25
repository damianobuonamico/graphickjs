/**
 * @file io/text/parser.h
 * @brief The file contains a basic text parser.
 */

#pragma once

#include "codepoint.h"
#include "unicode_.h"

#include <string>
#include <vector>

#define MAX_CLUSER_SIZE 32

namespace graphick::io::text {

/**
 * @brief Character input to the cluster parser.
 */
struct Token {
  char ch;          // The character.
  uint32_t offset;  // Offset of the character in code units.
  uint8_t len;      // Length of the character in code units.
  CharInfo info;    // Character information.
};

class Parser {
 public:
  void parse(const std::string& text);

 private:
  inline bool emoji()
  {
    return cur_emoji;
  }

  inline ClusterBreak kind()
  {
    return cur_kind;
  }

  inline bool accept(ClusterBreak kind)
  {
    if (cur_kind == kind) {
      accept_any();
      return true;
    } else {
      return false;
    }
  }

  inline bool accept_as(ClusterBreak kind, ShapeClass as_kind)
  {
    if (cur_kind == kind) {
      accept_any_as(as_kind);
      return true;
    } else {
      return false;
    }
  }

  inline void accept_any()
  {
    push_cur();
    advance();
  }

  inline void accept_any_as(ShapeClass as_kind)
  {
    // cluster.push_back(cur, as_kind);
    advance();
  }

  inline void advance()
  {
    // if (cluster.len() == MAX_CLUSTER_SIZE) {
    //   return;
    // }

    // if (chars.)
    //   Some(input) = self.s.chars.next()
    //   {
    //     let(kind, emoji) = input.info.cluster_class();
    //     self.s.cur = input;
    //     self.s.cur_emoji = emoji;
    //     self.s.cur_kind = kind;
    //     Some(())
    //   }
    // else {
    //   self.s.done = true;
    //   None
    // }
  }

  inline void push_cur()
  {
    // cluster.push_back(cur, ShapeClass::Base);
  }

 private:
  std::vector<Token> chars;  // The tokenized input.
  Token cur;
  ClusterBreak cur_kind;
  bool cur_emoji;
  bool done;
};

}  // namespace graphick::io::text
