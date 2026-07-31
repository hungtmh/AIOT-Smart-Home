package com.aiot.smarthome.controller;

import com.aiot.smarthome.dto.HistoryCountsResponse;
import com.aiot.smarthome.dto.HistoryPage;
import com.aiot.smarthome.dto.RecentActivityRow;
import com.aiot.smarthome.service.HistoryService;
import java.util.List;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/history")
public class HistoryController {
  private final HistoryService historyService;

  public HistoryController(HistoryService historyService) {
    this.historyService = historyService;
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

  @GetMapping("/{tab}")
  public HistoryPage getHistoryPage(
      @PathVariable String tab,
      @RequestParam(defaultValue = "0") int page,
      @RequestParam(defaultValue = "20") int size) {
    return historyService.getHistoryPage(tab, page, size);
  }
}
