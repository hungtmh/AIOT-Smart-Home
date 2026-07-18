package com.aiot.smarthome.config;

import com.aiot.smarthome.realtime.RealtimeWebSocketHandler;
import java.util.Arrays;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.WebSocketConfigurer;
import org.springframework.web.socket.config.annotation.WebSocketHandlerRegistry;

@Configuration
@EnableWebSocket
public class RealtimeWebSocketConfig implements WebSocketConfigurer {
  private final AiotProperties properties;
  private final RealtimeWebSocketHandler realtimeWebSocketHandler;

  public RealtimeWebSocketConfig(AiotProperties properties, RealtimeWebSocketHandler realtimeWebSocketHandler) {
    this.properties = properties;
    this.realtimeWebSocketHandler = realtimeWebSocketHandler;
  }

  @Override
  public void registerWebSocketHandlers(WebSocketHandlerRegistry registry) {
    String[] origins = Arrays.stream(properties.cors().allowedOrigins().split(","))
        .map(String::trim)
        .filter(origin -> !origin.isBlank())
        .toArray(String[]::new);

    registry.addHandler(realtimeWebSocketHandler, "/ws/realtime")
        .setAllowedOrigins(origins);
  }
}
