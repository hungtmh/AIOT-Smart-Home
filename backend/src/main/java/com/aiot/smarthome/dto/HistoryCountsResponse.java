package com.aiot.smarthome.dto;

public record HistoryCountsResponse(
    long sensors,
    long controls,
    long voice,
    long alerts,
    long fire) {
}
