
#pragma once

#include <new>
#include <cstring>

#include "../cubic_bezier.h"
#include "../line.h"

#include <vector>

namespace graphick::geom {

#define ARRAY_SIZE(a) \
    ((sizeof(a) / sizeof(*(a))) / \
      static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#define SIZE_OF(a) \
    (static_cast<int>(sizeof(a)))

#define DISABLE_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &) = delete; \
    void operator=(const TypeName &) = delete;

#define ASSERT(x)

  /**
   * Used for parallel curve construction.
   */
  class CubicCurveBuilder final {
  public:
    CubicCurveBuilder() {
    }
  public:
    ~CubicCurveBuilder();
  public:

    /**
     * Adds line.
     */
    void AddLine(const dvec2 p0, const dvec2 p1);


    /**
     * Adds cubic curve.
     */
    void AddCubic(const dvec2 p0, const dvec2 cp1,
        const dvec2 cp2, const dvec2 to);


    /**
     * Returns the first point.
     */
    dvec2 GetFirstPoint() const;


    /**
     * Returns the last point.
     */
    dvec2 GetLastPoint() const;


    /**
     * Returns start tangent.
     */
    dline GetStartTangent() const;


    /**
     * Returns end tangent.
     */
    dline GetEndTangent() const;


    /**
     * Returns the number of curves.
     */
    int GetSegmentCount() const;


    /**
     * Returns pointer to the output segment at a given index.
     *
     * @param index Segment index. Must be equal or greater than zero and less
     * than the value returned by GetSegmentCount.
     */
    const dcubic_bezier* GetSegmentAt(const int index) const;


    /**
     * Clears all segments in this builder.
     */
    void Reset();

  private:
    void MakeRoomForCurve();
  private:
    std::vector<dcubic_bezier> mCurves;
    // static constexpr int EmbeddedCurveCount = 32;

    // char mEmbeddedMemory[SIZE_OF(dcubic_bezier) * EmbeddedCurveCount];
    // dcubic_bezier* mCurves = reinterpret_cast<dcubic_bezier*>(mEmbeddedMemory);

    // // How many curves there are.
    // int mCurveCount = 0;

    // // For how many curves memory was allocated.
    // int mCurveCapacity = EmbeddedCurveCount;
  private:
    DISABLE_COPY_AND_ASSIGN(CubicCurveBuilder);
  };


  inline CubicCurveBuilder::~CubicCurveBuilder() {
    // if (mCurves != reinterpret_cast<dcubic_bezier*>(mEmbeddedMemory)) {
    //   free(mCurves);
    // }
  }


  inline void CubicCurveBuilder::AddLine(const dvec2 p0, const dvec2 p1) {
    mCurves.emplace_back(p0, p1);

    // MakeRoomForCurve();
    // void* p = mCurves + mCurveCount;
    // new (p) dcubic_bezier(p0, p1);
    // mCurveCount++;
  }


  inline void CubicCurveBuilder::AddCubic(const dvec2 p0, const dvec2 cp1, const dvec2 cp2, const dvec2 to) {
    mCurves.emplace_back(p0, cp1, cp2, to);
    // MakeRoomForCurve();
    // void* p = mCurves + mCurveCount;
    // new (p) dcubic_bezier(p0, cp1, cp2, to);
    // mCurveCount++;
  }


  inline dvec2 CubicCurveBuilder::GetFirstPoint() const {
    // ASSERT(mCurveCount > 0);

    // return mCurves->p0;

    return mCurves.front().p0;
  }


  inline dvec2 CubicCurveBuilder::GetLastPoint() const {
    // ASSERT(mCurveCount > 0);

    // return mCurves[mCurveCount - 1].p3;

    return mCurves.back().p3;
  }


  inline dline CubicCurveBuilder::GetStartTangent() const {
    // ASSERT(mCurveCount > 0);

    // return mCurves->start_tangent();

    return mCurves.front().start_tangent();
  }


  inline dline CubicCurveBuilder::GetEndTangent() const {
    // ASSERT(mCurveCount > 0);

    // return mCurves[mCurveCount - 1].end_tangent();

    return mCurves.back().end_tangent();
  }


  inline void CubicCurveBuilder::MakeRoomForCurve() {
    // if (mCurveCapacity == mCurveCount) {
    //   const int newCapacity = mCurveCapacity * 2;

    //   dcubic_bezier* curves = reinterpret_cast<dcubic_bezier*>(
    //       malloc(SIZE_OF(dcubic_bezier) * newCapacity));

    //   memcpy(curves, mCurves, SIZE_OF(dcubic_bezier) * mCurveCount);

    //   if (mCurves != reinterpret_cast<dcubic_bezier*>(mEmbeddedMemory)) {
    //     free(mCurves);
    //   }

    //   mCurves = curves;
    //   mCurveCapacity = newCapacity;
    // }
  }


  inline int CubicCurveBuilder::GetSegmentCount() const {
    // return mCurveCount;
    return mCurves.size();
  }


  inline const dcubic_bezier* CubicCurveBuilder::GetSegmentAt(const int index) const {
    // ASSERT(index >= 0);
    // ASSERT(index < mCurveCount);

    // return mCurves + index;

    return &mCurves[index];
  }


  inline void CubicCurveBuilder::Reset() {
    // mCurveCount = 0;
    mCurves.clear();
  }


  /**
   * Find a set of segments that approximate parallel curve.
   *
   * @param curve Input curve.
   *
   * @param offset Offset amount. If it is zero, resulting curve will be
   * identical to input curve. Can be negative.
   *
   * @param maximumError Maximum error. Lower value means better precision and
   * more output segments. Larger value means worse precision, but fewer output
   * segments.
   *
   * @param builder Output receiver.
   */
  extern void OffsetCurve(const dcubic_bezier& curve, const double offset,
      const double maximumError, CubicCurveBuilder& builder);

}
