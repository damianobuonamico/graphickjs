class HoverState {
  public entity: Entity | undefined = undefined;

  public get element() {
    let entity = this.entity;
    if (!entity) return undefined;
    while (entity.type !== 'element' && entity.parent) {
      entity = entity.parent;
    }
    if (entity.type === 'element') return entity;
    return undefined;
  }
}

export default HoverState;
