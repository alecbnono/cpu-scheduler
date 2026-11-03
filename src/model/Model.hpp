#pragma once

#include "model/SystemSnapshot.hpp"

class Model : public ISnapshotProvider {
public:
    SystemSnapshot getSnapshot() const override;
};