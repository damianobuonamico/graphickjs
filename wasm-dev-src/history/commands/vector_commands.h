#pragma once

#include "../command.h"

#include <vector>

namespace Graphick::History {

  template <typename T>
  class InsertInVectorCommand : public Command {
    using vector = std::vector<T>;
  public:
    InsertInVectorCommand(vector* vector, const T& value)
      : m_vector(vector), m_values({ value }), m_indices({ -1 }) {}

    InsertInVectorCommand(vector* vector, const T& value, int index)
      : m_vector(vector), m_values({ value }), m_indices({ index }) {}

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
      InsertInVectorCommand* casted_command = dynamic_cast<InsertInVectorCommand*>(command.get());

      if (
        casted_command == nullptr ||
        casted_command->m_vector != this->m_vector
        ) {
        return false;
      }

      casted_command->m_values.insert(casted_command->m_values.end(), m_values.begin(), m_values.end());
      casted_command->m_indices.insert(casted_command->m_indices.end(), m_indices.begin(), m_indices.end());

      return true;
    }

    virtual uintptr_t pointer() override {
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
      : m_vector(vector), m_values({ value }), m_indices({ }) {
      auto it = std::find(m_vector->begin(), m_vector->end(), value);

      if (it != m_vector->end()) {
        m_indices.push_back(static_cast<int>(it - m_vector->begin()));
      } else {
        m_indices.push_back(-1);
      }
    }

    EraseFromVectorCommand(vector* vector, const T& value, int index)
      : m_vector(vector), m_values({ value }), m_indices({ index }) {}

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

    virtual bool merge_with(std::unique_ptr<Command>& command) override {
      EraseFromVectorCommand* casted_command = dynamic_cast<EraseFromVectorCommand*>(command.get());

      if (
        casted_command == nullptr ||
        casted_command->m_vector != this->m_vector
        ) {
        return false;
      }

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
