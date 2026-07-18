package com.aiot.smarthome.model;

import java.time.OffsetDateTime;

public record SensorTelemetry(
    double temperature,
    double humidity,
    int smokePpm,
    OffsetDateTime measuredAt) {
}
