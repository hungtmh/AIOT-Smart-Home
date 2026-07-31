package com.aiot.smarthome.dto;

public record FireAlertResponse(
    String type,
    String imageBase64,
    long timestamp
) {}
