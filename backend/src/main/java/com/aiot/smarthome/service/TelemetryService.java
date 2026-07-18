package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.TelemetryResponse;
import com.aiot.smarthome.model.SensorTelemetry;
import com.aiot.smarthome.realtime.RealtimeHub;
import com.aiot.smarthome.repository.TelemetryRepository;
import org.springframework.stereotype.Service;

@Service
public class TelemetryService {
  private final TelemetryRepository repository;
  private final RealtimeHub realtimeHub;

  public TelemetryService(TelemetryRepository repository, RealtimeHub realtimeHub) {
    this.repository = repository;
    this.realtimeHub = realtimeHub;
  }

  public TelemetryResponse getLatestTelemetry() {
    return TelemetryResponse.from(repository.findLatest());
  }

  public void handleTelemetry(double temperature, double humidity, int smokePpm) {
    SensorTelemetry telemetry = repository.save(temperature, humidity, smokePpm);
    realtimeHub.broadcastTelemetry(TelemetryResponse.from(telemetry));
  }
}
