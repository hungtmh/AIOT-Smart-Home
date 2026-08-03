package com.aiot.smarthome.dto;

import java.time.OffsetDateTime;

public record VoiceCommandResponse(
    Long id,
    String recognizedText,
    String mappedDevice,
    String action,
    Double confidence,
    String result,
    OffsetDateTime createdAt) {
}
