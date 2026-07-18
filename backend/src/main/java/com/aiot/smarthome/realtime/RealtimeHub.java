package com.aiot.smarthome.realtime;

import com.aiot.smarthome.dto.DeviceStateResponse;
import com.aiot.smarthome.dto.TelemetryResponse;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;
import org.springframework.web.socket.handler.ConcurrentWebSocketSessionDecorator;

@Component
public class RealtimeHub {
  private static final Logger logger = LoggerFactory.getLogger(RealtimeHub.class);
  private static final int SEND_TIMEOUT_MILLIS = 5_000;
  private static final int BUFFER_SIZE_BYTES = 64 * 1024;

  private final ObjectMapper objectMapper;
  private final ConcurrentMap<String, WebSocketSession> sessions = new ConcurrentHashMap<>();

  public RealtimeHub(ObjectMapper objectMapper) {
    this.objectMapper = objectMapper;
  }

  public WebSocketSession register(WebSocketSession session) {
    WebSocketSession concurrentSession =
        new ConcurrentWebSocketSessionDecorator(session, SEND_TIMEOUT_MILLIS, BUFFER_SIZE_BYTES);
    sessions.put(session.getId(), concurrentSession);
    return concurrentSession;
  }

  public void unregister(String sessionId) {
    sessions.remove(sessionId);
  }

  public void broadcastDeviceState(DeviceStateResponse state) {
    broadcast("DEVICE_STATE", state);
  }

  public void broadcastTelemetry(TelemetryResponse telemetry) {
    broadcast("TELEMETRY", telemetry);
  }

  private void broadcast(String type, Object data) {
    final String payload;

    try {
      payload = objectMapper.writeValueAsString(Map.of("type", type, "data", data));
    } catch (JsonProcessingException exception) {
      logger.warn("Could not serialize realtime event {}", type, exception);
      return;
    }

    TextMessage message = new TextMessage(payload);
    sessions.forEach((sessionId, session) -> send(sessionId, session, message));
  }

  private void send(String sessionId, WebSocketSession session, TextMessage message) {
    if (!session.isOpen()) {
      sessions.remove(sessionId);
      return;
    }

    try {
      session.sendMessage(message);
    } catch (IOException | IllegalStateException exception) {
      sessions.remove(sessionId);
      logger.debug("Removed unavailable realtime session {}: {}", sessionId, exception.getMessage());
    }
  }
}
