package com.aiot.smarthome.mqtt;

import com.aiot.smarthome.config.AiotProperties;
import com.aiot.smarthome.model.DeviceDefinition;
import com.aiot.smarthome.service.DeviceCatalog;
import com.aiot.smarthome.service.DeviceService;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.UUID;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.context.annotation.Lazy;
import org.springframework.stereotype.Component;

@Component
public class MqttCommandPublisher implements MqttCallback {
  private static final Logger logger = LoggerFactory.getLogger(MqttCommandPublisher.class);

  private final AiotProperties properties;
  private final DeviceCatalog deviceCatalog;
  private final DeviceService deviceService;
  private MqttClient client;

  public MqttCommandPublisher(
      AiotProperties properties,
      DeviceCatalog deviceCatalog,
      @Lazy DeviceService deviceService) {
    this.properties = properties;
    this.deviceCatalog = deviceCatalog;
    this.deviceService = deviceService;
  }

  @PostConstruct
  public void connect() {
    try {
      AiotProperties.Mqtt mqtt = properties.mqtt();
      String clientId = mqtt.clientId() + "-" + UUID.randomUUID().toString().substring(0, 8);
      client = new MqttClient(mqtt.brokerUri(), clientId, new MemoryPersistence());
      client.setCallback(this);
      client.connect(connectOptions(mqtt));
      for (DeviceDefinition device : deviceCatalog.all()) {
        client.subscribe(stateTopic(device.id()), 1);
      }
      logger.info("Connected to MQTT broker {} and subscribed to device state topics", mqtt.brokerUri());
    } catch (Exception exception) {
      logger.warn("MQTT is not connected yet: {}", exception.getMessage());
    }
  }

  public boolean publishDeviceState(String deviceId, boolean enabled) {
    if (!isConnected()) {
      connect();
    }

    if (!isConnected()) {
      return false;
    }

    try {
      byte[] payload = (enabled ? "ON" : "OFF").getBytes(StandardCharsets.UTF_8);
      MqttMessage message = new MqttMessage(payload);
      message.setQos(1);
      message.setRetained(true);
      client.publish(commandTopic(deviceId), message);
      return true;
    } catch (MqttException exception) {
      logger.warn("Failed to publish {} command: {}", deviceId, exception.getMessage());
      return false;
    }
  }

  @PreDestroy
  public void disconnect() throws MqttException {
    if (client != null && client.isConnected()) {
      client.disconnect();
    }
  }

  @Override
  public void connectionLost(Throwable cause) {
    logger.warn("MQTT connection lost", cause);
  }

  @Override
  public void messageArrived(String topic, MqttMessage message) {
    String payload = new String(message.getPayload(), StandardCharsets.UTF_8).trim();
    String deviceId = deviceIdFromStateTopic(topic);
    if (deviceId == null) {
      logger.warn("Ignoring message from unknown topic {}", topic);
      return;
    }

    Boolean reportedState = parseState(payload);
    if (reportedState == null) {
      logger.warn("Ignoring unknown state payload '{}' from topic {}", payload, topic);
      return;
    }

    try {
      deviceService.handleReportedState(deviceId, reportedState);
      logger.info("Received {} reported state {} from topic {}", deviceId, payload, topic);
    } catch (Exception exception) {
      logger.warn("Received {} state {}, but could not save it to database yet", deviceId, payload, exception);
    }
  }

  @Override
  public void deliveryComplete(IMqttDeliveryToken token) {
  }

  private boolean isConnected() {
    return client != null && client.isConnected();
  }

  private String commandTopic(String deviceId) {
    return properties.mqtt().commandTopicPattern().formatted(deviceId);
  }

  private String stateTopic(String deviceId) {
    return properties.mqtt().stateTopicPattern().formatted(deviceId);
  }

  private String deviceIdFromStateTopic(String topic) {
    return deviceCatalog.all().stream()
        .map(DeviceDefinition::id)
        .filter(deviceId -> stateTopic(deviceId).equals(topic))
        .findFirst()
        .orElse(null);
  }

  private Boolean parseState(String payload) {
    String normalized = payload.trim().toUpperCase(Locale.ROOT);
    if (normalized.equals("ON") || normalized.equals("1") || normalized.equals("TRUE") || normalized.equals("OPEN")) {
      return true;
    }

    if (normalized.equals("OFF") || normalized.equals("0") || normalized.equals("FALSE") || normalized.equals("CLOSE")
        || normalized.equals("CLOSED")) {
      return false;
    }

    return null;
  }

  private MqttConnectOptions connectOptions(AiotProperties.Mqtt mqtt) {
    MqttConnectOptions options = new MqttConnectOptions();
    options.setAutomaticReconnect(true);
    options.setCleanSession(true);
    options.setConnectionTimeout(10);

    if (mqtt.username() != null && !mqtt.username().isBlank()) {
      options.setUserName(mqtt.username());
    }

    if (mqtt.password() != null && !mqtt.password().isBlank()) {
      options.setPassword(mqtt.password().toCharArray());
    }

    return options;
  }
}
