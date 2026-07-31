package com.aiot.smarthome.dto;

import java.util.List;

public record HistoryPage(
    String title,
    List<String> headers,
    List<List<String>> rows,
    int page,
    int totalPages,
    long totalRecords) {
}
