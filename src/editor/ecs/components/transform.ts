import { mat4, vec2, vec3, vec4 } from '@math';

class SimpleTransform {
  private m_translation: vec2 = vec2.create();

  public get vec2() {
    return vec2.clone(this.m_translation);
  }

  public clear() {
    this.m_translation = vec2.create();
  }

  public translate(delta: vec2 | null) {
    if (delta) vec2.add(this.m_translation, delta, true);
    else this.m_translation = vec2.create();
  }
}

class Transform {
  private m_translation: vec2 = vec2.create();
  private m_scaling: vec2 = vec2.create();
  private m_rotation: number = 0;

  constructor() {}

  public get mat4() {
    // TODO: rotation and scaling
    return mat4.fromTranslation(vec3.fromValues(this.m_translation[0], this.m_translation[1], 0));
  }

  public clear() {
    this.m_translation = vec2.create();
    this.m_scaling = vec2.create();
    this.m_rotation = 0;
  }

  public translate(delta: vec2 | null) {
    if (delta) vec2.add(this.m_translation, delta, true);
    else this.m_translation = vec2.create();
  }
}

export { SimpleTransform };
export default Transform;
