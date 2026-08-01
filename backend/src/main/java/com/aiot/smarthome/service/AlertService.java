package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.FireAlertResponse;
import com.aiot.smarthome.realtime.RealtimeHub;
import org.springframework.stereotype.Service;

import java.util.Base64;
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
    // Save to alert history
    historyService.saveAlert("FIRE", "Detected", "N/A", "Alert Triggered", "Active");

    // Convert image to Base64
    String base64Image = Base64.getEncoder().encodeToString(imageBytes);

  FireAlertResponse alertResponse = new FireAlertResponse(
      null,                    // id
      "ESP32",                 // deviceId
      "",                      // imagePath (AlertService không có đường dẫn ảnh)
      1.0,                     // confidence
      LocalDateTime.now(),     // detectedAt
      "FIRE"                   // status
  );

realtimeHub.broadcastFireAlert(alertResponse);    realtimeHub.broadcastAlert(alertResponse);
  }
}
