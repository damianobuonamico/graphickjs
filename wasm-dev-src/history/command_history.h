#pragma once

#include "command.h"

#include <vector>

namespace Graphick::History {

  class CommandHistory {
  public:
    CommandHistory(const CommandHistory&) = delete;
    CommandHistory(CommandHistory&&) = delete;

    static inline CommandHistory* get() { return s_instance; }

    static void init();
    static void shutdown();

    static void add(std::unique_ptr<Command> command);

    static void undo();
    static void redo();

    static void end_batch();
    static void seal();
    static void pop();
    static void clear();

    static inline void ignore_next() { get()->m_ignore_next = true; }
    static inline void clear_ignore() { get()->m_ignore_next = false; }
  private:
    CommandHistory() = default;
    ~CommandHistory() = default;
  private:
    std::vector<std::unique_ptr<Command>> m_commands;
    int m_index = -1;
    bool m_ignore_next = false;
  private:
    static CommandHistory* s_instance;
  };

}
