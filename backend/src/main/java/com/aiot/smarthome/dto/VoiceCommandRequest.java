package com.aiot.smarthome.dto;

public record VoiceCommandRequest(
    String recognizedText,
    String mappedDevice,
    String action,
    Double confidence) {
}
