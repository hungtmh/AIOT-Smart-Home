package com.aiot.smarthome.controller;

import com.aiot.smarthome.dto.HistoryCountsResponse;
import com.aiot.smarthome.dto.HistoryPage;
import com.aiot.smarthome.dto.RecentActivityRow;
import com.aiot.smarthome.dto.VoiceCommandRequest;
import com.aiot.smarthome.dto.VoiceCommandResponse;
import com.aiot.smarthome.realtime.RealtimeHub;
import com.aiot.smarthome.service.DeviceService;
import com.aiot.smarthome.service.HistoryService;
import java.time.OffsetDateTime;
import java.util.List;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/history")
public class HistoryController {
  private final HistoryService historyService;
  private final DeviceService deviceService;
  private final RealtimeHub realtimeHub;

  public HistoryController(HistoryService historyService, DeviceService deviceService, RealtimeHub realtimeHub) {
    this.historyService = historyService;
    this.deviceService = deviceService;
    this.realtimeHub = realtimeHub;
  }

  @GetMapping("/counts")
  public HistoryCountsResponse getCounts() {
    return historyService.getCounts();
  }

  @GetMapping("/recent-activity")
  public List<RecentActivityRow> getRecentActivity(
      @RequestParam(defaultValue = "10") int limit) {
    return historyService.getRecentActivity(limit);
  }

  @GetMapping("/voice/latest")
  public ResponseEntity<VoiceCommandResponse> getLatestVoiceCommand() {
    VoiceCommandResponse latest = historyService.getLatestVoiceCommand();
    if (latest == null) {
      return ResponseEntity.noContent().build();
    }
    return ResponseEntity.ok(latest);
  }

  @PostMapping("/voice")
  public ResponseEntity<VoiceCommandResponse> recordVoiceCommand(@RequestBody VoiceCommandRequest request) {
    String device = request.mappedDevice();
    String action = request.action();
    String result = "Accepted";

    if (device != null && !device.isBlank() && action != null && !action.isBlank()) {
      try {
        boolean state = "ON".equalsIgnoreCase(action) || "OPEN".equalsIgnoreCase(action) || "TRUE".equalsIgnoreCase(action);
        deviceService.commandDevice(device.toLowerCase(), state);
      } catch (Exception ex) {
        result = "Failed: " + ex.getMessage();
      }
    }

    historyService.saveVoiceCommand(
        request.recognizedText(),
        request.mappedDevice(),
        request.action(),
        request.confidence(),
        result);

    VoiceCommandResponse response = new VoiceCommandResponse(
        null,
        request.recognizedText(),
        request.mappedDevice(),
        request.action(),
        request.confidence(),
        result,
        OffsetDateTime.now());

    realtimeHub.broadcastVoiceCommand(response);
    return ResponseEntity.ok(response);
  }

  @GetMapping("/{tab}")
  public HistoryPage getHistoryPage(
      @PathVariable String tab,
      @RequestParam(defaultValue = "0") int page,
      @RequestParam(defaultValue = "20") int size) {
    return historyService.getHistoryPage(tab, page, size);
  }
}
