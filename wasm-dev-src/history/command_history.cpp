#include "command_history.h"

#include "../utils/console.h"
#include "command_batch.h"

#include <assert.h>

// TODO: fix commands casting without rtti
namespace Graphick::History {

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
        if (CommandBatch* command_batch = static_cast<CommandBatch*>(last_command.get())) {
          command_batch->add(command);
          merged = true;
        } else {
          merged = command->merge_with(last_command);
        }
      }
    }

    if (!merged) {
      instance->m_commands.push_back(std::make_unique<CommandBatch>(command));
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
      instance->m_commands.erase(instance->m_commands.begin() + (instance->m_index + 1), instance->m_commands.end());
      instance->m_index = static_cast<int>(instance->m_commands.size()) - 1;
    }
  }

  void CommandHistory::end_batch() {
    CommandHistory* instance = get();

    if (instance->m_index < 0) return;

    auto& command = instance->m_commands[instance->m_index];

    if (CommandBatch* command_batch = static_cast<CommandBatch*>(command.get())) {
      size_t batch_size = command_batch->size();

      if (batch_size == 0) {
        instance->m_commands.erase(instance->m_commands.begin() + instance->m_index);
        instance->m_commands[instance->m_index--]->disable_merge();

        return;
      } else if (batch_size == 1) {
        auto& first_command = command_batch->front();

        first_command->disable_merge();
        instance->m_commands[instance->m_index] = std::move(first_command);

        return;
      }
    }

    command->disable_merge();
  }

  void CommandHistory::pop() {
    CommandHistory* instance = get();

    instance->m_commands.erase(std::next(instance->m_commands.begin(), instance->m_index - 1), instance->m_commands.end());
    instance->m_index = (int)instance->m_commands.size() - 1;
  }

  void CommandHistory::clear() {
    CommandHistory* instance = get();

    instance->m_commands.clear();
    instance->m_index = -1;
  }

}
