/**
 * @file keys.h
 * @brief Contains the keymap for the supported platforms.
 *
 * @todo add the GLFW keymap.
 */

#pragma once

namespace graphick::editor::input {

  /**
   * @brief The KeyboardKey enum translates platform-specific keycodes to a platform-agnostic set of keynames.
  */
  enum class KeyboardKey {
    Undefined = -1,
    Backspace = 8,
    Shift = 16,
    Ctrl = 17,
    Alt = 18,
    Escape = 27,
    Space = 32,
    Delete = 46,
    A = 97,
    B = 98,
    C = 99,
    D = 100,
    E = 101,
    F = 102,
    G = 103,
    H = 104,
    I = 105,
    J = 106,
    K = 107,
    L = 108,
    M = 109,
    N = 110,
    O = 111,
    P = 112,
    Q = 113,
    R = 114,
    S = 115,
    T = 116,
    U = 117,
    V = 118,
    W = 119,
    X = 120,
    Y = 121,
    Z = 122,
  };

}
