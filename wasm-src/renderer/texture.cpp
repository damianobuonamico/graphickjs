#include "texture.h"

#ifdef EMSCRIPTEN
#include <GLES3/gl32.h>
#include <emscripten/html5.h>
#else
#include <glad/glad.h>
#endif

static bool isPowerOf2(int x) {
  return (x & (x - 1)) == 0;
}

static int next_p2(int a) {
  int rval = 1;
  // rval<<=1 is a prettier way of writing rval*=2; 
  while (rval < a) rval <<= 1;
  return rval;
}

Texture::Texture(const unsigned char* buffer, int width, int height) {
  // Allocate Memory For The Texture Data.
  // int w = next_p2(width);
  // int h = next_p2(height);
  // m_buffer = new GLubyte[2 * w * h];

  // Here We Fill In The Data For The Expanded Bitmap.
  // Notice That We Are Using A Two Channel Bitmap (One For
  // Channel Luminosity And One For Alpha), But We Assign
  // Both Luminosity And Alpha To The Value That We
  // Find In The FreeType Bitmap.
  // We Use The ?: Operator To Say That Value Which We Use
  // Will Be 0 If We Are In The Padding Zone, And Whatever
  // Is The FreeType Bitmap Otherwise.
  // for (int j = 0; j < h;j++) {
  //   for (int i = 0; i < w; i++) {
  //     m_buffer[2 * (i + j * w)] = m_buffer[2 * (i + j * w) + 1] =
  //       (i >= width || j >= height) ?
  //       0 : buffer[i + width * j];
  //   }
  // }

  // for (int j = 0; j < h; j++) {
  //   for (int i = 0; i < w; i++) {
  //     m_buffer[2 * (i + j * w)] = 200;
  //   }
  // }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  m_buffer = new GLubyte[1];

  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_2D, m_id);
  // const unsigned char data[] = { 0, 0, 255, 255 };
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);

  if (isPowerOf2(width) && isPowerOf2(height)) {
    // Yes, it's a power of 2. Generate mips.
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    // No, it's not a power of 2. Turn off mips and set
    //   wrapping to clamp to edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
}

void Texture::bind() const {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_id);
}
