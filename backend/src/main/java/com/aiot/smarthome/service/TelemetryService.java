package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.TelemetryResponse;
import com.aiot.smarthome.repository.TelemetryRepository;
import org.springframework.stereotype.Service;

@Service
public class TelemetryService {
  private final TelemetryRepository repository;

  public TelemetryService(TelemetryRepository repository) {
    this.repository = repository;
  }

  public TelemetryResponse getLatestTelemetry() {
    return TelemetryResponse.from(repository.findLatest());
  }

  public void handleTelemetry(double temperature, double humidity, int smokePpm) {
    repository.save(temperature, humidity, smokePpm);
  }
}
