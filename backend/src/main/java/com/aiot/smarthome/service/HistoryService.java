package com.aiot.smarthome.service;

import com.aiot.smarthome.dto.HistoryCountsResponse;
import com.aiot.smarthome.dto.HistoryPage;
import com.aiot.smarthome.dto.RecentActivityRow;
import com.aiot.smarthome.repository.HistoryRepository;
import java.util.List;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

@Service
public class HistoryService {
  private final HistoryRepository repository;

  public HistoryService(HistoryRepository repository) {
    this.repository = repository;
  }

  public HistoryPage getHistoryPage(String tab, int page, int size) {
    return switch (tab) {
      case "sensors" -> buildPage(
          "Sensor Data",
          List.of("Time", "Temperature (C)", "Humidity (%)", "Smoke Level"),
          repository.findSensors(page, size),
          page,
          size,
          repository.countSensors());

      case "controls" -> buildPage(
          "Control History",
          List.of("Time", "Device", "Action", "Source", "Result"),
          repository.findControls(page, size),
          page,
          size,
          repository.countControls());

      case "voice" -> buildPage(
          "Voice Command History",
          List.of("Time", "Recognized Text", "Mapped Device", "Action", "Confidence", "Result"),
          repository.findVoiceCommands(page, size),
          page,
          size,
          repository.countVoice());

      case "alerts" -> buildPage(
          "Alert History",
          List.of("Time", "Alert Type", "Sensor Value", "Threshold", "Action", "Status"),
          repository.findAlerts(page, size),
          page,
          size,
          repository.countAlerts());

      default -> throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "Unknown history tab: " + tab);
    };
  }

  public HistoryCountsResponse getCounts() {
    return new HistoryCountsResponse(
        repository.countSensors(),
        repository.countControls(),
        repository.countVoice(),
        repository.countAlerts());
  }

  public List<RecentActivityRow> getRecentActivity(int limit) {
    return repository.findRecentActivity(limit);
  }

  private HistoryPage buildPage(String title, List<String> headers, List<List<String>> rows,
      int page, int size, long totalRecords) {
    int totalPages = Math.max(1, (int) Math.ceil((double) totalRecords / size));
    return new HistoryPage(title, headers, rows, page, totalPages, totalRecords);
  }
}
