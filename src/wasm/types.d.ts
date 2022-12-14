interface Api {
  _init(): void;
  _resize(width: number, height: number): void;

  _to_heap(array: Float32Array): Pointer;
  _free(pointer: Pointer): void;

  _begin_frame(position: Pointer, zoom: number): void;
  _end_frame(): void;
  _draw(
    vertices: number,
    verticesLength: number,
    indices: number,
    indicesLength: number
  ): void;
}

type Pointer = number;
