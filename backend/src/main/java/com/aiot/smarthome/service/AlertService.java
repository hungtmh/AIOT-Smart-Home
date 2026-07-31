package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.FireAlertResponse;
import com.aiot.smarthome.realtime.RealtimeHub;
import org.springframework.stereotype.Service;

import java.util.Base64;

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

    // Broadcast the alert to WebSocket clients
    FireAlertResponse alertResponse = new FireAlertResponse("FIRE", base64Image, System.currentTimeMillis());
    realtimeHub.broadcastAlert(alertResponse);
  }
}
