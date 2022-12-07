import libtess from 'libtess';

/**
 * Lookup table for error types by number.
 * @enum {string}
 * @const
 * @private
 */
const ERROR_TYPES_ = {
  100900: 'GLU_INVALID_ENUM',
  100901: 'GLU_INVALID_VALUE',
  100151: 'GLU_TESS_MISSING_BEGIN_POLYGON',
  100153: 'GLU_TESS_MISSING_END_POLYGON',
  100152: 'GLU_TESS_MISSING_BEGIN_CONTOUR',
  100154: 'GLU_TESS_MISSING_END_CONTOUR',
  100155: 'GLU_TESS_COORD_TOO_LARGE',
  100156: 'GLU_TESS_NEED_COMBINE_CALLBACK'
};

/**
 * Lookup table for primitive types by number.
 * @enum {string}
 * @const
 * @private
 */
const PRIMITIVE_TYPES_ = {
  2: 'GL_LINE_LOOP',
  4: 'GL_TRIANGLES',
  5: 'GL_TRIANGLE_STRIP',
  6: 'GL_TRIANGLE_FAN'
};

/**
 * Tessellation output types.
 * @private {Array.<{name: string, value: boolean}>}
 * @const
 */
const OUTPUT_TYPES_ = [
  {
    name: 'triangulation',
    value: false
  },
  {
    name: 'boundaries',
    value: true
  }
];

/**
 * Whether to provide a normal to libtess or make it compute one.
 * @private {!Array.<{name: string, value: boolean}>}
 * @const
 */
const PROVIDE_NORMAL_ = [
  {
    name: 'explicitNormal',
    value: true
  },
  {
    name: 'computedNormal',
    value: false
  }
];

/**
 * Set of normals for planes in which to test tessellation.
 * @private {!Array.<{name: string, value: !Array.<number>}>}
 * @const
 */
const NORMALS_ = [
  {
    name: 'xyPlane',
    value: [0, 0, 1]
  },
  {
    name: 'xzPlane',
    value: [0, 1, 0]
  },
  {
    name: 'yzPlane',
    value: [1, 0, 0]
  },
  {
    name: 'tiltedPlane',
    value: [Math.SQRT1_2, Math.SQRT1_2, 0]
  }
  // TODO(bckenny): make this transformations instead, so we can test more than
  // just rotations about the origin
];

export default abstract class Tesselator {
  private static m_tesselator: any;

  static init() {
    var outputType = OUTPUT_TYPES_[0];
    let begun = false;

    function beginCallback(type: keyof typeof PRIMITIVE_TYPES_, vertexArrays: any) {
      // if (!begun) console.error('GLU_TESS_BEGIN without closing the last primitive');
      begun = true;

      if (outputType.value === false) {
        if (type !== libtess.primitiveType.GL_TRIANGLES)
          console.error('GL_TRIANGLES expected but ' + PRIMITIVE_TYPES_[type] + ' emitted instead');
      } else {
        if (type !== libtess.primitiveType.GL_LINE_LOOP)
          console.error('GL_LINE_LOOP expected but ' + PRIMITIVE_TYPES_[type] + ' emitted instead');
      }

      vertexArrays.push([]);
    }

    function vertexCallback(data: any, polyVertArray: any) {
      if (!begun) console.error('GLU_TESS_VERTEX called while not inside a primitive');

      polyVertArray[polyVertArray.length - 1].push(data[0], data[1], data[2]);
    }

    function endCallback() {
      // if (!begun) console.error('GLU_TESS_END called while not inside a primitive');
      begun = false;
    }

    function errorCallback(errorNumber: keyof typeof ERROR_TYPES_) {
      throw new Error(ERROR_TYPES_[errorNumber]);
    }

    function combineCallback(coords: any, data: any, weight: any) {
      // if (!begun) console.error('combine called while returning the vertices of a primitive');
      return [coords[0], coords[1], coords[2]];
    }

    function edgeCallback(flag: any) {}

    const tess = new libtess.GluTesselator();

    tess.gluTessCallback(libtess.gluEnum.GLU_TESS_BEGIN_DATA, beginCallback);
    tess.gluTessCallback(libtess.gluEnum.GLU_TESS_VERTEX_DATA, vertexCallback);
    tess.gluTessCallback(libtess.gluEnum.GLU_TESS_END, endCallback);

    tess.gluTessCallback(libtess.gluEnum.GLU_TESS_ERROR, errorCallback);
    tess.gluTessCallback(libtess.gluEnum.GLU_TESS_COMBINE, combineCallback);
    tess.gluTessCallback(libtess.gluEnum.GLU_TESS_EDGE_FLAG, edgeCallback);

    tess.gluTessProperty(libtess.gluEnum.GLU_TESS_BOUNDARY_ONLY, outputType.value);

    this.m_tesselator = tess;
  }

  static tesselate(points: vec2[]) {
    const tess = this.m_tesselator;

    // winding rule
    console.log(libtess.windingRule);
    tess.gluTessProperty(
      libtess.gluEnum.GLU_TESS_WINDING_RULE,
      libtess.windingRule.GLU_TESS_WINDING_NONZERO
    );

    // // transform function to align plane with desired normal
    // var rotate = exports.createPlaneRotation(normal.value);

    // // provide normal or compute
    // if (provideNormal.value) {
    //   tess.gluTessNormal.apply(tess, normal.value);
    // }

    var resultVerts: any = [];
    tess.gluTessBeginPolygon(resultVerts);

    tess.gluTessBeginContour();
    for (var j = 0; j < points.length; j++) {
      var coords: vec3 = [...points[j], 0];
      tess.gluTessVertex(coords, coords);
    }
    tess.gluTessEndContour();

    tess.gluTessEndPolygon();

    return resultVerts;
  }
}
