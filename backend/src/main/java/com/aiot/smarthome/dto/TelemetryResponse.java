package com.aiot.smarthome.dto;

import com.aiot.smarthome.model.SensorTelemetry;
import java.time.OffsetDateTime;

public record TelemetryResponse(
    double temperature,
    double humidity,
    int smokePpm,
    OffsetDateTime measuredAt) {
  public static TelemetryResponse from(SensorTelemetry telemetry) {
    return new TelemetryResponse(
        telemetry.temperature(),
        telemetry.humidity(),
        telemetry.smokePpm(),
        telemetry.measuredAt());
  }
}
