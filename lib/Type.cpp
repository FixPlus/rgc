#include "rgc/Type.hpp"
#include <cassert>

namespace rgc {

namespace {
std::string_view typeKindToName(Type::Kind kind) {
  switch (kind) {
#define RDC_CASE(X)                                                            \
  case Type::Kind::X:                                                          \
    return #X;
    RDC_CASE(Scalar)
    RDC_CASE(Aggregate)
#undef RDC_CASE
  }
  assert(0 && "unreachable");
  return "";
}

} // namespace
void Type::dump(std::ostream &os) const {
  os << this << " " << typeKindToName(m_kind)
     << "{ mapped type: " << typeIndex().name() << "}";
}

} // namespace rgc