#include "device.h"

#include "../../utils/console.h"

namespace Graphick::Renderer::GPU {

  DeviceBackend* Device::s_device = nullptr;

  void Device::init(const DeviceVersion version, const unsigned int default_framebuffer) {
    if (s_device != nullptr) {
      console::error("Device already initialized, call shutdown() before reinitializing!");
      return;
    }

#if defined(GK_GLES3) || defined(GK_GL3)
    if (version != DeviceVersion::GL3 && version != DeviceVersion::GLES3) {
      console::error("Invalid device version, try using a different version!");
      return;
    }
#endif

    s_device = new DeviceBackend(version, default_framebuffer);
  }

  void Device::shutdown() {
    if (s_device == nullptr) {
      console::error("Device already shutdown, call init() before shutting down!");
      return;
    }

    delete s_device;
  }

}
