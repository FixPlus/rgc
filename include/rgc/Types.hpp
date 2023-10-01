#ifndef RENDERGRAPHCOMPILER_TYPES_HPP
#define RENDERGRAPHCOMPILER_TYPES_HPP

#include "rgc/Types.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <ranges>

namespace rgc {

/**
 * @class ImageType
 *
 * Represents type of Image resource.
 *
 * Image is always a Device owned resource.
 *
 * There are 3 kinds of Image:
 * 1) Allocated - image that is allocated on device using
 * general allocation routine. It may have static or dynamic extents.
 * PixelFormat and ExtentType must not be set to auto. mipLevels must be at
 * least 1 and not greater than log2(min(extent)). If image has dynamic extents
 * mipLevels must be set to 1.
 * 2) ScreenBuffer - image that is a screen buffer(SwapChain image). Extents of
 * such image are unknown at compile time, that way it always has dynamic
 * extents. PixelFormat and ExtentType must be set to auto. mipLevel must be
 * set to 1.
 * 3) TiedToScreenBuffer - much like kind 1) except it's extents are set to
 * track screen buffer image. That way, only this kind of Image is allowed to be
 * in same framebuffer with ScreenBuffer image. ExtentType must be set to auto.
 * mipLevel must be set to 1.
 *
 */
class ImageType : public ScalarType {
public:
  enum class ImageKind { Allocated, ScreenBuffer, TiedToScreenBuffer };
  enum class PixelFormat {
#include "rgc/PixelFormat.inc"
    Auto
  };
  enum class ExtentType { T1D, T2D, T3D, Auto };

  ImageType(ImageKind ik, PixelFormat pf, ExtentType et, unsigned mipLevels = 1,
            std::span<const size_t, 3> extents = std::array<size_t, 3>{0ull,
                                                                       0ull,
                                                                       0ull})
      : ScalarType(ScalarType::Kind::Image, ScalarType::OwnerType::Device),
        m_kind(ik), m_pixelFormat(pf), m_type(et), m_mipLevels(mipLevels) {
    std::copy(extents.begin(), extents.end(), m_extents.begin());
  }

  auto imageKind() const { return m_kind; }

  auto pixelFormat() const { return m_pixelFormat; }

  auto extentType() const { return m_type; }

  auto mipLevels() const { return m_mipLevels; }

  bool hasDynamicExtents() const {
    return std::ranges::all_of(m_extents, [](auto e) { return e == 0u; });
  }
  std::span<const size_t, 3> extents() const { return m_extents; }

  size_t hash() const {
    auto extentHash = std::accumulate(
        m_extents.begin(), m_extents.end(), (size_t)0u, [](auto s, auto &&e) {
          return std::hash<size_t>{}(std::hash<size_t>{}(s) +
                                     std::hash<size_t>{}(e));
        });
    return std::hash<size_t>{}(
        extentHash + std::hash<size_t>{}((unsigned long long)m_kind) +
        std::hash<size_t>{}((unsigned long long)m_pixelFormat) +
        std::hash<size_t>{}((unsigned long long)m_type) +
        std::hash<unsigned>{}(m_mipLevels));
  }
  bool equal(Type *another) const {
    if (auto *i = dynamic_cast<ImageType *>(another)) {
      return m_kind == i->m_kind && m_pixelFormat == i->m_pixelFormat &&
             m_type == i->m_type && m_mipLevels == i->m_mipLevels &&
             std::ranges::equal(m_extents, i->m_extents);
    }
    return false;
  }

private:
  ImageKind m_kind;
  PixelFormat m_pixelFormat;
  ExtentType m_type;

  unsigned m_mipLevels;
  std::array<size_t, 3> m_extents;
};

class AllocatedImageType : public ImageType {
public:
  AllocatedImageType(PixelFormat pf, ExtentType et, unsigned mipLevels,
                     std::span<size_t, 3> extents)
      : ImageType(ImageType::ImageKind::Allocated, pf, et, mipLevels, extents) {
  }
};

class ScreenBufferImage : public ImageType {
public:
  explicit ScreenBufferImage(unsigned swapChainID)
      : ImageType(ImageType::ImageKind::ScreenBuffer,
                  ImageType::PixelFormat::Auto, ImageType::ExtentType::Auto),
        m_swapChainID(swapChainID) {}
  auto getSwapChainID() const { return m_swapChainID; }

  size_t hash() const {
    return std::hash<size_t>{}((size_t)m_swapChainID + ImageType::hash());
  }
  bool equal(Type *another) const {
    if (!ImageType::equal(another))
      return false;
    if (auto *i = dynamic_cast<ScreenBufferImage *>(another)) {
      return m_swapChainID == i->m_swapChainID;
    }
    return false;
  }

private:
  unsigned m_swapChainID;
};

class TiedToScreenBufferImage : public ImageType {
public:
  explicit TiedToScreenBufferImage(PixelFormat pf, unsigned swapChainID)
      : ImageType(ImageType::ImageKind::TiedToScreenBuffer, pf,
                  ImageType::ExtentType::Auto),
        m_swapChainID(swapChainID) {}
  auto getSwapChainID() const { return m_swapChainID; }

  size_t hash() const {
    return std::hash<size_t>{}((size_t)m_swapChainID + ImageType::hash());
  }
  bool equal(Type *another) const {
    if (!ImageType::equal(another))
      return false;
    if (auto *i = dynamic_cast<TiedToScreenBufferImage *>(another)) {
      return m_swapChainID == i->m_swapChainID;
    }
    return false;
  }

private:
  unsigned m_swapChainID;
};

/**
 * @class BufferType
 *
 * Represents type of generic buffer resource.
 *
 * Buffers can either be host or device owned.
 *
 */
class BufferType : public ScalarType {
public:
  BufferType(ScalarType::OwnerType ownerType, size_t es, size_t ec)
      : ScalarType(ScalarType::Kind::Buffer, ownerType), m_elementSize(es),
        m_elementCount(ec) {
    assert(m_elementSize != 0 && "Element size can't be zero");
  }
  size_t extent() const { return m_elementCount; }

  bool hasDynamicExtents() const { return m_elementCount == 0; }

  size_t elementSize() const { return m_elementSize; }

  size_t hash() const {
    return std::hash<size_t>{}(std::hash<size_t>{}(m_elementSize) +
                               std::hash<size_t>{}(m_elementCount));
  }
  bool equal(Type *another) const {
    if (auto *i = dynamic_cast<BufferType *>(another)) {
      return ownerType() == i->ownerType() &&
             m_elementSize == i->m_elementSize &&
             m_elementCount == i->m_elementCount;
    }
    return false;
  }

private:
  size_t m_elementSize;
  size_t m_elementCount;
};

} // namespace rgc
#endif // RENDERGRAPHCOMPILER_TYPES_HPP
