package com.aiot.smarthome.mqtt;

import com.aiot.smarthome.config.AiotProperties;
import com.aiot.smarthome.service.LedService;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import java.nio.charset.StandardCharsets;
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
  private final LedService ledService;
  private MqttClient client;

  public MqttCommandPublisher(AiotProperties properties, @Lazy LedService ledService) {
    this.properties = properties;
    this.ledService = ledService;
  }

  @PostConstruct
  public void connect() {
    try {
      AiotProperties.Mqtt mqtt = properties.mqtt();
      String clientId = mqtt.clientId() + "-" + UUID.randomUUID().toString().substring(0, 8);
      client = new MqttClient(mqtt.brokerUri(), clientId, new MemoryPersistence());
      client.setCallback(this);
      client.connect(connectOptions(mqtt));
      client.subscribe(mqtt.stateTopic(), 1);
      logger.info("Connected to MQTT broker {} and subscribed to {}", mqtt.brokerUri(), mqtt.stateTopic());
    } catch (Exception exception) {
      logger.warn("MQTT is not connected yet: {}", exception.getMessage());
    }
  }

  public boolean publishLedState(boolean enabled) {
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
      client.publish(properties.mqtt().commandTopic(), message);
      return true;
    } catch (MqttException exception) {
      logger.warn("Failed to publish LED command: {}", exception.getMessage());
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
    boolean ledOn = payload.equalsIgnoreCase("ON") || payload.equals("1") || payload.equalsIgnoreCase("true");
    try {
      ledService.handleReportedLedState(ledOn);
      logger.info("Received LED reported state {} from topic {}", payload, topic);
    } catch (Exception exception) {
      logger.warn("Received LED state {}, but could not save it to database yet", payload, exception);
    }
  }

  @Override
  public void deliveryComplete(IMqttDeliveryToken token) {
  }

  private boolean isConnected() {
    return client != null && client.isConnected();
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
