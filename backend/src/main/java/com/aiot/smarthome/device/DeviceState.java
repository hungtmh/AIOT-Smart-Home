package com.aiot.smarthome.device;

import java.time.OffsetDateTime;

public record DeviceState(
    String deviceId,
    boolean desiredState,
    boolean reportedState,
    OffsetDateTime updatedAt,
    OffsetDateTime reportedAt) {
}
