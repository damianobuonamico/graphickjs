#pragma once

#include <memory>

class Command {
public:
  virtual ~Command() {}

  virtual void execute() = 0;
  virtual void undo() = 0;

  virtual bool merge_with(std::unique_ptr<Command>& command) = 0;

  inline void disable_merge() { m_can_merge = false; };
  inline bool can_merge() const { return m_can_merge; };
protected:
  bool m_can_merge = true;
};
