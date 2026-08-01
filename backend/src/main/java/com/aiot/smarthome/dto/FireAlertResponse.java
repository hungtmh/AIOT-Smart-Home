package com.aiot.smarthome.dto;


import java.time.LocalDateTime;

public record FireAlertResponse(
    Long id,
    String deviceId,
    String imagePath,
    Double confidence,
    LocalDateTime detectedAt,
    String status
) {}
