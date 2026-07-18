package com.aiot.smarthome.dto;

import jakarta.validation.constraints.NotNull;

public record DeviceCommandRequest(@NotNull Boolean state) {
}
