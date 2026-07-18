package com.aiot.smarthome.realtime;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.aiot.smarthome.service.DeviceService;
import com.aiot.smarthome.service.TelemetryService;
import java.io.IOException;
import java.util.Map;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.security.oauth2.jwt.Jwt;
import org.springframework.security.oauth2.jwt.JwtDecoder;
import org.springframework.security.oauth2.jwt.JwtException;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.CloseStatus;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;
import org.springframework.web.socket.handler.TextWebSocketHandler;

@Component
public class RealtimeWebSocketHandler extends TextWebSocketHandler {
  private static final Logger logger = LoggerFactory.getLogger(RealtimeWebSocketHandler.class);

  private final JwtDecoder jwtDecoder;
  private final ObjectMapper objectMapper;
  private final RealtimeHub realtimeHub;
  private final DeviceService deviceService;
  private final TelemetryService telemetryService;

  public RealtimeWebSocketHandler(
      JwtDecoder jwtDecoder,
      ObjectMapper objectMapper,
      RealtimeHub realtimeHub,
      DeviceService deviceService,
      TelemetryService telemetryService) {
    this.jwtDecoder = jwtDecoder;
    this.objectMapper = objectMapper;
    this.realtimeHub = realtimeHub;
    this.deviceService = deviceService;
    this.telemetryService = telemetryService;
  }

  @Override
  protected void handleTextMessage(WebSocketSession session, TextMessage message) throws IOException {
    JsonNode json;

    try {
      json = objectMapper.readTree(message.getPayload());
    } catch (Exception exception) {
      reject(session, "Invalid realtime message");
      return;
    }

    if (!"AUTH".equals(json.path("type").asText())) {
      reject(session, "Authenticate before subscribing");
      return;
    }

    String token = json.path("token").asText("");
    try {
      Jwt jwt = jwtDecoder.decode(token);
      WebSocketSession authenticatedSession = realtimeHub.register(session);
      send(authenticatedSession, Map.of(
          "type", "AUTHENTICATED",
          "userId", jwt.getSubject()));
      send(authenticatedSession, Map.of(
          "type", "DEVICE_SNAPSHOT",
          "data", deviceService.getDeviceStates()));
      send(authenticatedSession, Map.of(
          "type", "TELEMETRY",
          "data", telemetryService.getLatestTelemetry()));
      logger.debug("Realtime session {} authenticated for user {}", session.getId(), jwt.getSubject());
    } catch (JwtException exception) {
      logger.warn("Rejected realtime session {}: {}", session.getId(), exception.getMessage());
      reject(session, "JWT is missing or invalid");
    }
  }

  @Override
  public void afterConnectionClosed(WebSocketSession session, CloseStatus status) {
    realtimeHub.unregister(session.getId());
  }

  @Override
  public void handleTransportError(WebSocketSession session, Throwable exception) {
    realtimeHub.unregister(session.getId());
  }

  private void reject(WebSocketSession session, String reason) throws IOException {
    send(session, Map.of(
        "type", "AUTH_ERROR",
        "message", reason));
    session.close(CloseStatus.POLICY_VIOLATION.withReason(reason));
  }

  private void send(WebSocketSession session, Object payload) throws IOException {
    session.sendMessage(new TextMessage(objectMapper.writeValueAsString(payload)));
  }
}
