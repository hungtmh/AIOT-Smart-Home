import { API_BASE_URL } from "../lib/api";
import { useRealtime } from "../realtime/RealtimeContext";

export default function FireAlert() {
  const { fireAlert, clearFireAlert } = useRealtime();

  if (!fireAlert) return null;

  const imageName = (fireAlert.imagePath || "")
    .split(/[\\/]/)
    .filter(Boolean)
    .pop();
  const imageUrl = imageName
    ? `${API_BASE_URL}/uploads/${imageName}`
    : null;

  return (
    <div
      style={{
        position: "fixed",
        inset: 0,
        background: "rgba(0,0,0,0.45)",
        display: "flex",
        justifyContent: "center",
        alignItems: "center",
        zIndex: 9999,
      }}
    >
      <div
        style={{
          position: "relative",
          width: "520px",
          maxWidth: "90%",
          background: "#ffffff",
          borderRadius: "18px",
          padding: "28px",
          boxShadow: "0 20px 50px rgba(0,0,0,0.25)",
        }}
      >
        {/* Nút đóng */}
        <button
          onClick={clearFireAlert}
          style={{
            position: "absolute",
            top: "14px",
            right: "14px",
            width: "34px",
            height: "34px",
            border: "none",
            borderRadius: "50%",
            background: "#f3f4f6",
            cursor: "pointer",
            fontSize: "20px",
            fontWeight: "bold",
            color: "#666",
          }}
        >
          ×
        </button>

        <h2
          style={{
            margin: 0,
            marginBottom: "20px",
            color: "#dc2626",
            fontSize: "28px",
            fontWeight: "700",
            textAlign: "center",
          }}
        >
          Fire Alert
        </h2>

        {imageUrl ? (
          <img
            src={imageUrl}
            alt="Fire Detection"
            style={{
              width: "100%",
              borderRadius: "12px",
              border: "1px solid #e5e7eb",
              marginBottom: "20px",
            }}
          />
        ) : (
          <div
            style={{
              width: "100%",
              minHeight: "220px",
              borderRadius: "12px",
              border: "1px dashed #d1d5db",
              marginBottom: "20px",
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
              color: "#6b7280",
              background: "#f9fafb",
            }}
          >
            No image available
          </div>
        )}

        <div
          style={{
            background: "#f9fafb",
            borderRadius: "12px",
            padding: "16px",
            lineHeight: "2",
            color: "#374151",
          }}
        >
          <div>
            <strong>Camera:</strong> {fireAlert.deviceId}
          </div>

          <div>
            <strong>Confidence:</strong>{" "}
            {(fireAlert.confidence * 100).toFixed(1)}%
          </div>

          <div>
            <strong>Status:</strong> {fireAlert.status}
          </div>

          <div>
            <strong>Detected At:</strong>{" "}
            {fireAlert.detectedAt
              ? new Date(fireAlert.detectedAt).toLocaleString()
              : "Unknown"}
          </div>
        </div>

        {fireAlert.confidence >= 0.8 && (
          <div
            style={{
              marginTop: "16px",
              padding: "12px 16px",
              background: "#fee2e2",
              border: "1px solid #fca5a5",
              borderRadius: "10px",
              color: "#991b1b",
              fontWeight: "600",
              fontSize: "14px",
              display: "flex",
              alignItems: "center",
              gap: "8px",
            }}
          >
            <span>🚨</span>
            <span>
              High confidence fire alert! Water Pump Relay has been <strong>AUTOMATICALLY ACTIVATED</strong> for 60 seconds.
            </span>
          </div>
        )}

        <button
          onClick={clearFireAlert}
          style={{
            marginTop: "22px",
            width: "100%",
            padding: "12px",
            border: "none",
            borderRadius: "10px",
            background: "#2563eb",
            color: "#fff",
            fontSize: "15px",
            fontWeight: "600",
            cursor: "pointer",
          }}
        >
          Close
        </button>
      </div>
    </div>
  );
}
