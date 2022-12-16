#include "command_history.h"

#include <assert.h>

CommandHistory* CommandHistory::s_instance = nullptr;

void CommandHistory::init() {
  assert(!s_instance);
  s_instance = new CommandHistory();
}

void CommandHistory::shutdown() {
  delete s_instance;
}

void CommandHistory::add(std::unique_ptr<Command> command) {
  command->execute();

  if (get()->m_ignore_next) {
    clear_ignore();
    return;
  }

  seal();

  CommandHistory* instance = get();

  bool merged = false;

  if (!instance->m_commands.empty()) {
    std::unique_ptr<Command>& last_command = instance->m_commands.back();

    if (last_command->can_merge() && command->can_merge()) {
      merged = command->merge_with(last_command);
    }
  }

  if (!merged) {
    instance->m_commands.push_back(std::move(command));
    instance->m_index++;
  }
}

void CommandHistory::undo() {
  CommandHistory* instance = get();

  if (instance->m_index >= 0 && !instance->m_commands.empty()) {
    instance->m_commands[instance->m_index]->undo();
    instance->m_index--;
  }
}

void CommandHistory::redo() {
  CommandHistory* instance = get();
  size_t next_index = instance->m_index + 1;

  if (next_index < instance->m_commands.size() && next_index >= 0) {
    instance->m_commands[next_index]->execute();
    instance->m_index++;
  }
}

void CommandHistory::seal() {
  CommandHistory* instance = get();

  if (instance->m_index < static_cast<int>(instance->m_commands.size()) - 1) {
    instance->m_commands.erase(std::next(instance->m_commands.begin(), instance->m_index), instance->m_commands.end());
    instance->m_index = static_cast<int>(instance->m_commands.size()) - 1;
  }
}

void CommandHistory::pop() {
  CommandHistory* instance = get();

  instance->m_commands.erase(std::next(instance->m_commands.begin(), instance->m_index - 1), instance->m_commands.end());
  instance->m_index = instance->m_commands.size() - 1;
}

void CommandHistory::clear() {
  CommandHistory* instance = get();

  instance->m_commands.clear();
  instance->m_index = -1;
}
