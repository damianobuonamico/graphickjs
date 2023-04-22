#pragma once

#include "../component.h"
#include "../../../values/int_value.h"

class LayerComponent: public Component {
public:
  enum BlendingMode {
    BlendNormal = 0,
    BlendAverage,
    BlendColorBurn,
    BlendColorDodge,
    BlendDarken,
    BlendDifference,
    BlendExclusion,
    BlendGlow,
    BlendHardLight,
    BlendHardMix,
    BlendLighten,
    BlendLinearBurn,
    BlendLinearDodge,
    BlendLinearLight,
    BlendMultiply,
    BlendNegation,
    BlendOverlay,
    BlendPhoenix,
    BlendPinLight,
    BlendReflect,
    BlendScreen,
    BlendSoftLight,
    BlendSubtract,
    BlendVividLight
  };
public:
  LayerComponent(Entity* entity): Component(entity) {};
  LayerComponent(const LayerComponent&) = default;
  LayerComponent(LayerComponent&&) = default;

  ~LayerComponent() = default;

  inline IntValue& blending_mode() { return m_blending_mode; };
  inline const IntValue& blending_mode() const { return m_blending_mode; };

  inline IntValue& opacity() { return m_opacity; };
  inline const IntValue& opacity() const { return m_opacity; };
private:
  IntValue m_blending_mode = BlendNormal;
  IntValue m_opacity = 100;
};
