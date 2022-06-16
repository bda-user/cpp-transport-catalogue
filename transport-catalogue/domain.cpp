#include "domain.h"

namespace transport {

auto StopsPtr::AsTuple() const {
    return std::tie(ptr1, ptr2);
}

bool StopsPtr::operator==(const StopsPtr& other) const {
    return AsTuple() == other.AsTuple();
}

size_t StopsPtrHash::operator() (const StopsPtr& sp) const {
    return static_cast<size_t>(ptr_hasher_(sp.ptr1) +
                               ptr_hasher_(sp.ptr2) * 37);
}

} // namespace transport
