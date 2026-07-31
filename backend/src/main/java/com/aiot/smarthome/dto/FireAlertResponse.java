package com.aiot.smarthome.dto;


import java.time.LocalDateTime;

public record FireAlertResponse(
    String type,
    String imageBase64,
    long timestamp
) {}
