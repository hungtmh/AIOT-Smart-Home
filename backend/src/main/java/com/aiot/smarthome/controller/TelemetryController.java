package com.aiot.smarthome.controller;

import com.aiot.smarthome.dto.TelemetryResponse;
import com.aiot.smarthome.service.TelemetryService;
import java.util.List;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/telemetry")
public class TelemetryController {
  private final TelemetryService telemetryService;

  public TelemetryController(TelemetryService telemetryService) {
    this.telemetryService = telemetryService;
  }

  @GetMapping("/latest")
  public TelemetryResponse getLatestTelemetry() {
    return telemetryService.getLatestTelemetry();
  }

  @GetMapping("/history")
  public List<TelemetryResponse> getTelemetryHistory(
      @org.springframework.web.bind.annotation.RequestParam(defaultValue = "30") int limit) {
    return telemetryService.getTelemetryHistory(limit);
  }
}
