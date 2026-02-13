#pragma once

#include "Types.h"

namespace npp::platform {

class IScalingService {
public:
    virtual ~IScalingService() = default;

    virtual float GetGlobalScaleFactor() const = 0;
    virtual float GetScaleFactorForPoint(const Point& screenPoint) const = 0;
    virtual int ScalePx(int logicalPx) const = 0;
    virtual int UnscalePx(int physicalPx) const = 0;
};

}  // namespace npp::platform
