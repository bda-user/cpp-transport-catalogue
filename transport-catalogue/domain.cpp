#include "domain.h"

namespace transport {

auto StopsPtr::AsTuple() const {
    return std::tie(from_ptr, to_ptr);
}

bool StopsPtr::operator==(const StopsPtr& other) const {
    return AsTuple() == other.AsTuple();
}

size_t StopsPtrHash::operator() (const StopsPtr& sp) const {
    return static_cast<size_t>(ptr_hasher_(sp.from_ptr) +
                               ptr_hasher_(sp.to_ptr) * 37);
}

} // namespace transport
