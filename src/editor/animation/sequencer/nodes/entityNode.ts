import { vec2 } from '@/math';
import Node from './node';

class EntityNode extends Node {
  entity: Entity;
  duration: number = 3000;

  constructor(entity: Entity, position: vec2) {
    super(position, entity.id);

    this.entity = entity;
  }
}

export default EntityNode;
