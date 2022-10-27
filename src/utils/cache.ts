export class Cache {
  private m_map: Map<string, any>;

  pause: boolean = false;

  constructor() {
    this.m_map = new Map();
  }

  public cached<T>(id: string, callback: () => T, zoom?: number): T {
    if (this.pause === true) {
      this.pause = false;
      this.m_map.clear();
    }

    if (this.m_map.has(id) && (!zoom || zoom === this.m_map.get(id + '-zoom'))) {
      // if (id === 'unrotatedBoundingBox') console.log('hit');
      return this.m_map.get(id);
    }
    // if (id === 'unrotatedBoundingBox') console.log('miss');

    const value = callback();
    this.m_map.set(id, value);

    if (zoom) this.m_map.set(id + 'zoom', zoom);

    return value;
  }

  public clear() {
    this.m_map.clear();
  }

  public get zoom(): number | undefined {
    return this.m_map.get('zoom');
  }
}
// export class Cache implements CacheComponent {
//   public m_levels: CacheLevel[];

//   constructor(levels: number = 1) {
//     this.m_levels = [];

//     for (let i = 0; i < Math.max(levels, 1); i++) {
//       this.m_levels.push(new CacheLevel(this, i));
//     }
//   }

//   public getCacheAtLevel(level: number) {
//     return this.m_levels[level] || this.m_levels[this.m_levels.length - 1];
//   }

//   public cached<T>(
//     id: string,
//     callback: () => T,
//     level: number = this.m_levels.length - 1,
//     zoom?: number
//   ): T {
//     return this.m_levels[level]?.cached(id, callback, zoom);
//   }

//   public clear(level: number = 0) {
//     for (let i = level; i < this.m_levels.length; i++) {
//       this.m_levels[i].empty();
//     }
//   }
// }

// export class CacheLevel implements CacheLevelComponent {
//   private m_parent: Cache;
//   private m_level: number;
//   private m_map: Map<string, any>;

//   constructor(parent: Cache, level: number) {
//     this.m_parent = parent;
//     this.m_level = level;
//     this.m_map = new Map();
//   }

//   public cached<T>(id: string, callback: () => T, zoom?: number): T {
//     if (this.m_map.has(id) && (!zoom || zoom === this.m_map.get(id + '-zoom'))) {
//       // console.log(id, 'hit');
//       return this.m_map.get(id);
//     }

//     // console.log(id, 'miss');

//     const value = callback();
//     this.m_map.set(id, value);

//     if (zoom) this.m_map.set(id + 'zoom', zoom);

//     return value;
//   }

//   public clear() {
//     this.m_parent.clear(this.m_level);
//   }

//   public empty() {
//     this.m_map.clear();
//   }
// }

// /**
//  * Cache Class
//  *
//  * Lower order levels affect higher order levels (level 0 clears all of the other levels)
//  */
// class Cachee {
//   public m_levels: CacheeLevel[];

//   pause: boolean = false;

//   constructor(levels: number = 1) {
//     this.m_levels = [];

//     for (let i = 0; i < Math.max(levels, 1); i++) {
//       this.m_levels.push(new CacheeLevel(this, i));
//     }
//   }

//   public getCacheAtLevel(level: number) {
//     return this.m_levels[level] || this.m_levels[this.m_levels.length - 1];
//   }

//   public cached<T>(
//     id: string,
//     callback: () => T,
//     level: number = this.m_levels.length - 1,
//     zoom?: number
//   ): T {
//     return this.m_levels[level]?.cached(id, callback, zoom);
//   }

//   public shouldMiss(level: number = 0) {
//     let index = -1;

//     for (let i = 0; i <= level; i++) {
//       if (this.m_levels[i].pause === true) {
//         index = i;
//         break;
//       }
//     }

//     if (index !== -1) {
//       for (let i = index; i < this.m_levels.length; i++) {
//         this.m_levels[i].pause = false;
//         this.m_levels[i].clear();
//       }
//       return true;
//     } else return false;
//   }

//   public clear(level: number = 0) {
//     // for (let i = 0; i <= level; i++) {
//     //   this.m_levels[i].pause = false;
//     //   this.m_levels[i].clear();
//     // }
//   }
// }

// class CacheeLevel {
//   private m_parent: Cachee;
//   private m_map: Map<string, any>;
//   private m_level: number;

//   pause: boolean = false;

//   constructor(parent: Cachee, level: number) {
//     this.m_parent = parent;
//     this.m_map = new Map();
//     this.m_level = level;
//   }

//   public cached<T>(id: string, callback: () => T, zoom?: number): T {
//     if (this.m_parent.shouldMiss(this.m_level)) {
//       this.m_parent.clear(this.m_level);
//     }

//     if (this.m_map.has(id) && (!zoom || zoom === this.m_map.get(id + '-zoom'))) {
//       // console.log(id, 'hit');
//       return this.m_map.get(id);
//     }

//     // console.log(id, 'miss');

//     const value = callback();
//     this.m_map.set(id, value);

//     if (zoom) this.m_map.set(id + 'zoom', zoom);

//     return value;
//   }

//   public clear() {
//     this.m_map.clear();
//   }

//   public get zoom(): number | undefined {
//     return this.m_map.get('zoom');
//   }
// }
