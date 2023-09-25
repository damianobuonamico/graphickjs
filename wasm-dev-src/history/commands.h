#pragma once

#include "../math/vec2.h"

#include "../utils/uuid.h"

#include <memory>
#include <vector>

namespace Graphick::History {

  class Command {
  public:
    enum class Type {
      Batch,
      ChangePrimitive,
      ChangeVec2,
      // Implemented in scene.cpp
      InsertInRegistry,
      EraseFromRegistry,
      // Implemented in segment.cpp
      CreateHandle,
      InsertInVector,
      EraseFromVector
    };
  public:
    const Type type;
  public:
    Command(const Type type) : type(type) {}
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

  template <typename T>
  class ChangePrimitiveCommand : public Command {
  public:
    ChangePrimitiveCommand(T& old_value, const T new_value)
      : Command(Type::ChangePrimitive), m_value(old_value), m_new_value(new_value), m_old_value(old_value) {}

    inline virtual void execute() override {
      m_old_value = m_value;
      m_value = m_new_value;
    }

    inline virtual void undo() override {
      m_value = m_old_value;
    }

    virtual bool merge_with(std::unique_ptr<Command>& command) override {
      if (command->type != Type::ChangePrimitive) return false;

      ChangePrimitiveCommand<T>* casted_command = static_cast<ChangePrimitiveCommand<T>*>(command.get());

      if (&casted_command->m_value != &this->m_value) return false;
      casted_command->m_new_value = this->m_new_value;

      return true;
    }

    inline virtual uintptr_t pointer() override {
      return reinterpret_cast<uintptr_t>(&m_value);
    }
  private:
    T& m_value;
    T m_new_value;
    T m_old_value;
  };

  class ChangeVec2Command : public Command {
  public:
    ChangeVec2Command(vec2& old_value, const vec2& new_value)
      : Command(Type::ChangeVec2), m_value(old_value), m_new_value(new_value), m_old_value({}) {}

    inline virtual void execute() override {
      m_old_value = m_value;
      m_value = m_new_value;
    }

    inline virtual void undo() override {
      m_value = m_old_value;
    }

    virtual bool merge_with(std::unique_ptr<Command>& command) override {
      if (command->type != Type::ChangeVec2) return false;

      ChangeVec2Command* casted_command = static_cast<ChangeVec2Command*>(command.get());

      if (&casted_command->m_value != &this->m_value) return false;
      casted_command->m_new_value = this->m_new_value;

      return true;
    }

    inline virtual uintptr_t pointer() override {
      return reinterpret_cast<uintptr_t>(&m_value);
    }
  private:
    vec2& m_value;
    vec2 m_new_value;
    vec2 m_old_value;
  };

  template <typename T>
  class InsertInVectorCommand : public Command {
    using vector = std::vector<T>;
  public:
    InsertInVectorCommand(vector* vector, const T& value)
      : Command(Type::InsertInVector), m_vector(vector), m_values({ value }), m_indices({ -1 }) {}

    InsertInVectorCommand(vector* vector, const T& value, int index)
      : Command(Type::InsertInVector), m_vector(vector), m_values({ value }), m_indices({ index }) {}

    virtual void execute() override {
      if (m_vector == nullptr) return;

      for (size_t i = 0; i < m_values.size(); ++i) {
        T& value = m_values[i];
        int index = m_indices[i];

        if (index > -1) {
          auto it = m_vector->begin() + index;
          m_vector->insert(it, value);
        } else {
          m_vector->push_back(value);
        }
      }
    }

    virtual void undo() override {
      if (m_vector == nullptr) return;

      for (int i = static_cast<int>(m_values.size()) - 1; i >= 0; --i) {
        T& value = m_values[i];
        int index = m_indices[i];

        if (index > -1) {
          m_vector->erase(m_vector->begin() + index);
        } else {
          m_vector->erase(std::remove(m_vector->begin(), m_vector->end(), value), m_vector->end());
        }
      }
    }

    virtual bool merge_with(std::unique_ptr<Command>& command) override {
      if (command->type != Type::InsertInVector) return false;

      InsertInVectorCommand* casted_command = static_cast<InsertInVectorCommand*>(command.get());

      if (casted_command->m_vector != this->m_vector) return false;
      casted_command->m_values.insert(casted_command->m_values.end(), m_values.begin(), m_values.end());
      casted_command->m_indices.insert(casted_command->m_indices.end(), m_indices.begin(), m_indices.end());

      return true;
    }

    inline virtual uintptr_t pointer() override {
      return reinterpret_cast<uintptr_t>(m_vector);
    }
  private:
    vector* m_vector;
    std::vector<T> m_values;
    std::vector<int> m_indices;
  };

  template <typename T>
  class EraseFromVectorCommand : public Command {
    using vector = std::vector<T>;
  public:
    EraseFromVectorCommand(vector* vector, const T& value)
      : Command(Type::EraseFromVector), m_vector(vector), m_values({ value }), m_indices({ })
    {
      auto it = std::find(m_vector->begin(), m_vector->end(), value);

      if (it != m_vector->end()) {
        m_indices.push_back(static_cast<int>(it - m_vector->begin()));
      } else {
        m_indices.push_back(-1);
      }
    }

    EraseFromVectorCommand(vector* vector, const T& value, int index)
      : Command(Type::EraseFromVector), m_vector(vector), m_values({ value }), m_indices({ index }) {}

    virtual void execute() override {
      if (m_vector == nullptr) return;

      for (size_t i = 0; i < m_values.size(); ++i) {
        T& value = m_values[i];
        int index = m_indices[i];

        if (index > -1) {
          m_vector->erase(m_vector->begin() + index);
        } else {
          m_vector->erase(std::remove(m_vector->begin(), m_vector->end(), value), m_vector->end());
        }
      }
    }

    virtual void undo() override {
      if (m_vector == nullptr) return;

      for (int i = static_cast<int>(m_values.size()) - 1; i >= 0; --i) {
        T& value = m_values[i];
        int index = m_indices[i];

        if (index > -1) {
          m_vector->insert(m_vector->begin() + index, value);
        } else {
          m_vector->push_back(value);
        }
      }
    }

    inline virtual bool merge_with(std::unique_ptr<Command>& command) override {
      if (command->type != Type::EraseFromVector) return false;

      EraseFromVectorCommand* casted_command = static_cast<EraseFromVectorCommand*>(command.get());

      if (casted_command->m_vector != this->m_vector) return false;

      casted_command->m_values.insert(casted_command->m_values.end(), m_values.begin(), m_values.end());
      casted_command->m_indices.insert(casted_command->m_indices.end(), m_indices.begin(), m_indices.end());

      return true;
    }

    virtual uintptr_t pointer() override {
      return reinterpret_cast<uintptr_t>(m_vector) << 1;
    }
  private:
    vector* m_vector;
    vector m_values;
    std::vector<int> m_indices;
  };

}