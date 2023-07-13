#pragma once

#include <memory>

namespace Graphick::History {

  class Command {
  public:
    Command() = default;
    Command(const Command&) = delete;
    Command(Command&&) = delete;

    virtual ~Command() {}

    virtual void execute() = 0;
    virtual void undo() = 0;

    virtual bool merge_with(std::unique_ptr<Command>& command) = 0;
    virtual uintptr_t pointer() { return 0; }

    inline virtual void disable_merge() { m_can_merge = false; };
    inline bool can_merge() const { return m_can_merge; };

  protected:
    bool m_can_merge = true;
  };

}
