package com.aiot.smarthome.device;

import jakarta.validation.constraints.NotNull;

public record LedCommandRequest(@NotNull Boolean state) {
}
