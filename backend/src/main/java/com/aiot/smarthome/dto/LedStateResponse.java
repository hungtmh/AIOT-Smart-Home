package com.aiot.smarthome.dto;

import com.aiot.smarthome.model.DeviceState;
import java.time.OffsetDateTime;

public record LedStateResponse(
    String deviceId,
    boolean desiredState,
    boolean reportedState,
    boolean mqttPublished,
    OffsetDateTime updatedAt,
    OffsetDateTime reportedAt) {
  public static LedStateResponse from(DeviceState state, boolean mqttPublished) {
    return new LedStateResponse(
        state.deviceId(),
        state.desiredState(),
        state.reportedState(),
        mqttPublished,
        state.updatedAt(),
        state.reportedAt());
  }
}
