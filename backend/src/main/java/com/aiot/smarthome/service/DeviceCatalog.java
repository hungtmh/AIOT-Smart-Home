package com.aiot.smarthome.service;

import com.aiot.smarthome.model.DeviceDefinition;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.Function;
import java.util.stream.Collectors;
import org.springframework.stereotype.Component;

@Component
public class DeviceCatalog {
  private final List<DeviceDefinition> devices = List.of(
      new DeviceDefinition("led", "LED Light", "relay"),
      new DeviceDefinition("servo", "Servo Motor", "servo"),
      new DeviceDefinition("buzzer", "Buzzer Alarm", "buzzer"),
      new DeviceDefinition("pump", "Mini Water Pump", "pump"));

  private final Map<String, DeviceDefinition> devicesById = devices.stream()
      .collect(Collectors.toUnmodifiableMap(DeviceDefinition::id, Function.identity()));

  public List<DeviceDefinition> all() {
    return devices;
  }

  public Optional<DeviceDefinition> find(String deviceId) {
    return Optional.ofNullable(devicesById.get(deviceId));
  }
}
