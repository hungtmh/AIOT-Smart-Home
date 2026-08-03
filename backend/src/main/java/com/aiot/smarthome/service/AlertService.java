package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.FireAlertResponse;
import com.aiot.smarthome.realtime.RealtimeHub;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;

@Service
public class AlertService {
  private final HistoryService historyService;
  private final RealtimeHub realtimeHub;

  public AlertService(HistoryService historyService, RealtimeHub realtimeHub) {
    this.historyService = historyService;
    this.realtimeHub = realtimeHub;
  }

  public void handleFireAlert(byte[] imageBytes) {
    // Save to alert history and fire alert history
    historyService.saveAlert("FIRE", "Detected", "N/A", "Alert Triggered", "Active");
    historyService.saveFireAlert("ESP32", "", 1.0, "FIRE");

    FireAlertResponse alertResponse = new FireAlertResponse(
        null,                    // id
        "ESP32",                 // deviceId
        "",                      // imagePath
        1.0,                     // confidence
        LocalDateTime.now(),     // detectedAt
        "FIRE"                   // status
    );

    realtimeHub.broadcastFireAlert(alertResponse);
    realtimeHub.broadcastAlert(alertResponse);
  }
}
