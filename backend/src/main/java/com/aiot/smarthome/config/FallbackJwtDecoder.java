package com.aiot.smarthome.config;

import java.util.List;
import org.springframework.security.oauth2.jwt.Jwt;
import org.springframework.security.oauth2.jwt.JwtDecoder;
import org.springframework.security.oauth2.jwt.JwtException;

public class FallbackJwtDecoder implements JwtDecoder {
  private final List<JwtDecoder> decoders;

  public FallbackJwtDecoder(List<JwtDecoder> decoders) {
    this.decoders = decoders;
  }

  @Override
  public Jwt decode(String token) throws JwtException {
    JwtException firstException = null;

    for (JwtDecoder decoder : decoders) {
      try {
        return decoder.decode(token);
      } catch (JwtException exception) {
        if (firstException == null) {
          firstException = exception;
        } else {
          firstException.addSuppressed(exception);
        }
      }
    }

    throw firstException != null ? firstException : new JwtException("No JWT decoder is configured");
  }
}
