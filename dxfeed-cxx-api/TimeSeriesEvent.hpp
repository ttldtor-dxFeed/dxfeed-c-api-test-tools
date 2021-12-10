#pragma once

#include "IndexedEvent.hpp"

namespace dxf {

template <typename SymbolType>
struct TimeSeriesEvent : public virtual IndexedEvent<SymbolType> {
  IndexedEventSource getSource() override { return IndexedEventSource::DEFAULT; }
};

}