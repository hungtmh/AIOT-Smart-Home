package com.aiot.smarthome.dto;

import jakarta.validation.constraints.NotNull;

public record LedCommandRequest(@NotNull Boolean state) {
}
