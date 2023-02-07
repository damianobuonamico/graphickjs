#pragma once

#include "command.h"

#include <unordered_map>
#include <vector>
#include <iterator>

class CommandBatch: public Command {
  using CommandPointer = std::unique_ptr<Command>;
public:
  CommandBatch(CommandPointer& command) {
    uintptr_t pointer = command->pointer();
    m_commands.push_back(std::move(command));

    if (pointer != 0) {
      m_commands_indices.insert({ pointer, static_cast<int>(m_commands.size()) - 1 });
    }
  }


  inline size_t size() const { return m_commands.size(); }
  inline CommandPointer& front() { return m_commands.front(); }

  void add(CommandPointer& command) {
    bool merged = false;

    if (command->can_merge()) {
      uintptr_t pointer = command->pointer();

      if (pointer != 0) {
        auto it = m_commands_indices.find(pointer);

        if (it != m_commands_indices.end()) {
          CommandPointer& last_command = m_commands[it->second];

          if (last_command->can_merge() && command->merge_with(last_command)) {
            merged = true;
          }
        }
      }
    }

    if (!merged) {
      uintptr_t pointer = command->pointer();
      m_commands.push_back(std::move(command));

      if (pointer != 0) {
        m_commands_indices.insert({ pointer, static_cast<int>(m_commands.size()) - 1 });
      }
    }
  }

  virtual void execute() override {
    for (auto& command : m_commands) {
      command->execute();
    }
  }

  virtual void undo() override {
    for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
      (*it)->undo();
    }
  }

  inline virtual void disable_merge() override {
    m_commands_indices.clear();
    m_can_merge = false;
  }

  virtual bool merge_with(CommandPointer& command) override {
    CommandBatch* casted_command = dynamic_cast<CommandBatch*>(command.get());
    if (casted_command == nullptr) return false;

    std::move(this->m_commands.begin(), this->m_commands.end(), std::back_inserter(casted_command->m_commands));
    this->m_commands.clear();

    return true;
  }
private:
  std::vector<CommandPointer> m_commands;
  std::unordered_map<uintptr_t, int> m_commands_indices;
};
