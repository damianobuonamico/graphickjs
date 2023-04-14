#pragma once

class Texture {
public:
  Texture(const unsigned char* buffer, int width, int height);
  ~Texture() {
    delete[] m_buffer;
  }

  void bind() const;
private:
  unsigned int m_id;
  unsigned char* m_buffer;
};
