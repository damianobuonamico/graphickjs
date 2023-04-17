#include "font.h"

#include "../../io/woff2/include/woff2/output.h"
#include "../../io/woff2/include/woff2/decode.h"
#include "../../utils/console.h"

#define WOFF2_SIGNATURE 0x774F4632

Font::Font(const uint8_t* buffer, size_t buffer_size) {
  if (buffer_size < 4) {
    console::error("Invalid font buffer size!");
  }

  for (int i = 0; i < 4; i++) {
    console::log(buffer[i]);
  }

  if ((buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3]) == WOFF2_SIGNATURE) {
    m_buffer_size = woff2::ComputeWOFF2FinalSize(buffer, buffer_size);
    if (m_buffer_size == 0) {
      console::error("Failed to compute WOFF2 final size!");
    }

    m_buffer = new uint8_t[m_buffer_size];
    woff2::WOFF2MemoryOut out(m_buffer, m_buffer_size);

    bool success = woff2::ConvertWOFF2ToTTF(buffer, buffer_size, &out);
    if (!success) {
      console::error("Failed to convert WOFF2 to TTF!");
    }

    m_buffer_size = out.Size();
  } else {
    m_buffer_size = buffer_size;
    m_buffer = new uint8_t[m_buffer_size];
    memcpy(m_buffer, buffer, m_buffer_size);
  }

  if (!stbtt_InitFont(&m_font_info, m_buffer, 0)) {
    console::error("Failed to initialize font info!");
  }

  // int line_height = 64;
  // float scale = stbtt_ScaleForPixelHeight(&m_font_info, line_height);

  // m_font = hz_stbtt_font_create(&m_font_info);
  // hz_feature_t features[] = {
  //   HZ_FEATURE_CCMP,
  //   HZ_FEATURE_ISOL,
  //   HZ_FEATURE_INIT,
  //   HZ_FEATURE_MEDI,
  //   HZ_FEATURE_FINA,
  //   HZ_FEATURE_RLIG,
  //   HZ_FEATURE_CALT,
  //   HZ_FEATURE_LIGA,
  //   HZ_FEATURE_DLIG,
  //   HZ_FEATURE_MSET,
  //   HZ_FEATURE_CURS,
  //   HZ_FEATURE_KERN,
  //   HZ_FEATURE_MARK,
  //   HZ_FEATURE_MKMK,
  // };

  // hz_font_data_init(&m_font_data, HZ_DEFAULT_FONT_DATA_ARENA_SIZE);
  // hz_font_data_load(&m_font_data, m_font);

  // hz_shaper_init(&m_shaper);
  // hz_shaper_set_direction(&m_shaper, HZ_DIRECTION_LTR);
  // hz_shaper_set_script(&m_shaper, HZ_SCRIPT_LATIN);
  // hz_shaper_set_language(&m_shaper, HZ_LANGUAGE_ENGLISH);
  // hz_shaper_set_features(&m_shaper, features, sizeof(features) / sizeof(features[0]));

  m_blob = hb_blob_create((const char*)m_buffer, m_buffer_size, HB_MEMORY_MODE_WRITABLE, m_buffer, nullptr);
  m_face = hb_face_create(m_blob, 0);
  m_font = hb_font_create(m_face);

  // FT_Error error = FT_New_Memory_Face(m_library, m_buffer, m_buffer_size, 0, &m_ft_face);
  // if (error) {
  //   console::error("Failed to load font face", error);
  // }

  // error = FT_Set_Pixel_Sizes(m_ft_face, 0, 64);
  // if (error) {
  //   console::error("Failed to set font size", error);
  //   return;
  // }
}

Font::~Font() {
  // hz_font_data_release(&m_font_data);
  // hz_font_destroy(m_font);
  hb_font_destroy(m_font);
  hb_face_destroy(m_face);
  hb_blob_destroy(m_blob);

  delete[] m_buffer;
}
