package com.aiot.smarthome.dto;

import java.time.OffsetDateTime;

public record ControlLogResponse(
    String deviceId,
    String requestedState,
    String source,
    String result,
    OffsetDateTime createdAt) {
}
