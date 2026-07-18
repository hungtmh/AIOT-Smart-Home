package com.aiot.smarthome.dto;

import com.aiot.smarthome.model.DeviceState;
import java.time.OffsetDateTime;

public record DeviceStateResponse(
    String deviceId,
    String name,
    String type,
    boolean desiredState,
    boolean reportedState,
    boolean mqttPublished,
    OffsetDateTime updatedAt,
    OffsetDateTime reportedAt) {
  public static DeviceStateResponse from(DeviceState state, String name, String type, boolean mqttPublished) {
    return new DeviceStateResponse(
        state.deviceId(),
        name,
        type,
        state.desiredState(),
        state.reportedState(),
        mqttPublished,
        state.updatedAt(),
        state.reportedAt());
  }
}
